#ifndef EE_STRING_H
#define EE_STRING_H

#pragma warning(disable : 4996)

#include "stdio.h"
#include "ee_core.h"

#define EE_SS_LEN                   (16)
#define EE_LS_PREFIX_LEN            (4)

#define EE_STR_LEV_BLOCK_SIZE       (64)
#define EE_STR_CHARS_MASK_LEN       (0xFF)
#define EE_STR_INVALID              (0xffffffffffffffffull)

#define EE_STR_FILE_READ            ("r")
#define EE_STR_FILE_READ_BYTES      ("rb")
#define EE_STR_FILE_WRITE           ("w")
#define EE_STR_FILE_APPEND          ("a")
#define EE_STR_FILE_WRITE_BYTES     ("wb")
#define EE_STR_FILE_APPEND_BYTES    ("ab")

#define EE_UINT64_SHIFT             (6)
#define EE_UINT64_MASK              ((1ull << EE_UINT64_SHIFT) - 1)
#define EE_UINT64_INV_MASK          (~EE_UINT64_MASK)

typedef struct Str
{
	size_t top;
	size_t cap;
	char* buffer;
	Allocator allocator;
} Str;

typedef struct Str_View
{
	size_t len;
	const char* buffer;
} Str_View;

EE_EXTERN_C_START

EE_INLINE Str ee_str_new(size_t size, const Allocator* allocator)
{
	Str out = { 0 };

	if (allocator == NULL)
	{
		out.allocator.alloc_fn = ee_default_alloc;
		out.allocator.realloc_fn = ee_default_realloc;
		out.allocator.free_fn = ee_default_free;
		out.allocator.context = NULL;
	}
	else
	{
		memcpy(&out.allocator, allocator, sizeof(Allocator));
	}

	out.cap = size;
	out.top = 0;
	out.buffer = (char*)out.allocator.alloc_fn(&out.allocator, size);

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Str.buffer", size);

	return out;
}

EE_INLINE Str ee_str_from_cstr(const char* c_str, const Allocator* allocator)
{
	Str out = { 0 };

	if (allocator == NULL)
	{
		out.allocator.alloc_fn = ee_default_alloc;
		out.allocator.realloc_fn = ee_default_realloc;
		out.allocator.free_fn = ee_default_free;
		out.allocator.context = NULL;
	}
	else
	{
		memcpy(&out.allocator, allocator, sizeof(Allocator));
	}

	size_t size = strlen((const char*)c_str);

	out.cap = size;
	out.top = size;
	out.buffer = (char*)out.allocator.alloc_fn(&out.allocator, out.cap);

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Str.buffer", out.cap);

	memcpy(out.buffer, c_str, size);

	return out;
}

EE_INLINE Str ee_str_from_file(const char* file_path, const char* mode, const Allocator* allocator)
{
	EE_ASSERT(file_path != NULL, "Trying to open NULL file path");

	if (mode == NULL)
	{
		mode = EE_STR_FILE_READ_BYTES;
	}

	EE_ASSERT(strcmp((const char*)mode, EE_STR_FILE_READ) == 0 || strcmp((const char*)mode, EE_STR_FILE_READ_BYTES) == 0, "Invalid mode for reading from file (%s)", mode);

	FILE* file = fopen((const char*)file_path, (const char*)mode);

	EE_ASSERT(file != NULL, "Unable to open file (%s)", file_path);

	fseek(file, 0, SEEK_END);
	long file_size_l = ftell(file);
	EE_ASSERT(file_size_l >= 0, "Unable to get file size");
	fseek(file, 0, SEEK_SET);

	size_t file_size = (size_t)file_size_l;

	Str out = { 0 };

	if (allocator == NULL)
	{
		out.allocator.alloc_fn = ee_default_alloc;
		out.allocator.realloc_fn = ee_default_realloc;
		out.allocator.free_fn = ee_default_free;
		out.allocator.context = NULL;
	}
	else
	{
		memcpy(&out.allocator, allocator, sizeof(Allocator));
	}

	out.cap = file_size;
	out.top = file_size;
	out.buffer = (char*)out.allocator.alloc_fn(&out.allocator, out.cap);

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Str.buffer", out.cap);

	size_t bytes_read = fread(out.buffer, 1, file_size, file);
	EE_ASSERT(bytes_read == file_size, "Unable to read (%zu) bytes from file", file_size);

	fclose(file);

	return out;
}

EE_INLINE const char* ee_str_to_cstr(Str* str)
{
	EE_ASSERT(str != NULL, "Trying to get c string from NULL string");

	size_t c_str_len = str->top + 1;
	char* c_str = (char*)str->allocator.alloc_fn(&str->allocator, c_str_len);

	EE_ASSERT(c_str != NULL, "Unable to allocate (%zu) bytes for C string buffer", c_str_len);

	memcpy(c_str, str->buffer, str->top);
	c_str[c_str_len - 1] = '\0';

	return (const char*)c_str;
}

EE_INLINE void ee_str_to_file(Str* str, const char* file_path, const char* mode)
{
	EE_ASSERT(str != NULL, "Trying to write a NULL string to file");
	EE_ASSERT(file_path != NULL, "Trying to write to a NULL file path");

	if (mode == NULL)
	{
		mode = EE_STR_FILE_APPEND_BYTES;
	}

	EE_ASSERT(strcmp((const char*)mode, EE_STR_FILE_WRITE) == 0 || strcmp((const char*)mode, EE_STR_FILE_WRITE_BYTES) == 0 || strcmp((const char*)mode, EE_STR_FILE_APPEND) == 0 || strcmp((const char*)mode, EE_STR_FILE_APPEND_BYTES) == 0, "Invalid mode for writing to file (%s)", mode);

	FILE* file = fopen((const char*)file_path, (const char*)mode);

	EE_ASSERT(file != NULL, "Unable to open file (%s)", file_path);

	size_t bytes_wrote = fwrite(str->buffer, 1, str->top, file);

	EE_ASSERT(bytes_wrote == str->top, "Unable to write (%zu) bytes to file", str->top);

	fclose(file);
}


EE_INLINE void ee_str_free(Str* str)
{
	EE_ASSERT(str != NULL, "Trying to free NULL string");
	EE_ASSERT(str->buffer != NULL, "Trying to free NULL string buffer");

	str->allocator.free_fn(&str->allocator, str->buffer);

	memset(str, 0, sizeof(Str));
}

EE_INLINE void ee_str_grow(Str* str)
{
	EE_ASSERT(str != NULL, "Trying to grow NULL string");
	EE_ASSERT(str->buffer != NULL, "Trying to grow NULL string buffer");

	size_t new_cap = str->cap + (str->cap >> 1);
	char* new_buffer = (char*)str->allocator.realloc_fn(&str->allocator, str->buffer, str->cap, new_cap);

	EE_ASSERT(new_buffer != NULL, "Unable to reallocate (%zu) bytes for Str.buffer", new_cap);

	str->cap = new_cap;
	str->buffer = new_buffer;
}

EE_INLINE i32 ee_str_full(const Str* str)
{
	EE_ASSERT(str != NULL, "Trying to check NULL string");

	return str->top >= str->cap;
}

EE_INLINE i32 ee_str_empty(const Str* str)
{
	EE_ASSERT(str != NULL, "Trying to check NULL string");

	return str->top == 0;
}

EE_INLINE void ee_str_push(Str* str, char symbol)
{
	EE_ASSERT(str != NULL, "Trying to push into NULL string");

	if (ee_str_full(str))
	{
		ee_str_grow(str);
	}

	str->buffer[str->top++] = symbol;
}

EE_INLINE void ee_str_pop(Str* str, char* out_val)
{
	EE_ASSERT(str != NULL, "Trying to pop from NULL string");
	EE_ASSERT(!ee_str_empty(str), "Trying to pop from empty string");

	str->top--;

	if (out_val != NULL)
	{
		*out_val = str->buffer[str->top];
	}
}

EE_INLINE i32 ee_str_cmp(const Str* a, const Str* b)
{
	if (a->top < b->top)
	{
		return -1;
	}
	else if (a->top > b->top)
	{
		return 1;
	}
	
	return memcmp(a->buffer, b->buffer, a->top);
}

EE_INLINE size_t ee_str_find_b(const Str* str, const Str* target, size_t low, size_t high)
{
	EE_ASSERT(str != NULL, "Trying to search in NULL string");
	EE_ASSERT(target != NULL, "Trying to search a NULL target");
	EE_ASSERT(low <= str->top && high <= str->top && low < high, "Invalid bounds (%zu, %zu) for string with length (%zu)", low, high, str->top);

	size_t target_len = target->top;

	EE_ASSERT(target_len <= high - low, "Target too long for bounds");

	ee_simd_i mask = ee_set1_epi8(target->buffer[0]);

	size_t i = low;
	size_t upper = (high & ~(EE_SIMD_BYTES - 1));
	size_t start_block = (low + EE_SIMD_BYTES - 1) & ~(EE_SIMD_BYTES - 1);

	for (; i < start_block && i + target_len <= high; ++i)
	{
		if (memcmp(&str->buffer[i], target->buffer, target_len) == 0)
		{
			return i;
		}
	}

	for (; i < upper; i += EE_SIMD_BYTES)
	{
		ee_simd_i group = ee_loadu_si((const ee_simd_i*)&str->buffer[i]);
		ee_simd_i match = ee_cmpeq_epi8(group, mask);
		i32 match_mask = ee_movemask_epi8(match);

		while (match_mask)
		{
			i32 first = ee_first_bit_u32(match_mask);

			if ((i + first + target_len <= high) &&
				memcmp(&str->buffer[i + first], target->buffer, target_len) == 0)
			{
				return i + first;
			}

			match_mask &= match_mask - 1;
		}
	}

	for (; i < high && i + target_len <= high; ++i)
	{
		if (memcmp(&str->buffer[i], target->buffer, target_len) == 0)
		{
			return i;
		}
	}

	return EE_STR_INVALID;
}

EE_INLINE size_t ee_str_find(const Str* str, const Str* target)
{
	return ee_str_find_b(str, target, 0, str->top);
}

EE_INLINE size_t ee_str_count_b(const Str* str, const Str* target, size_t low, size_t high)
{
	EE_ASSERT(str != NULL, "Trying to count in NULL string");
	EE_ASSERT(target != NULL, "Trying to count NULL target");
	EE_ASSERT(low <= str->top && high <= str->top && low < high, "Invalid bounds (%zu, %zu) for string with length (%zu)", low, high, str->top);

	size_t target_len = target->top;

	EE_ASSERT(target_len <= high - low, "Target too long for bounds");

	ee_simd_i mask = ee_set1_epi8(target->buffer[0]);

	size_t out = 0;
	size_t i = low;
	size_t upper = (high & ~(EE_SIMD_BYTES - 1));
	size_t start_block = (low + EE_SIMD_BYTES - 1) & ~(EE_SIMD_BYTES - 1);

	for (; i < start_block && i + target_len <= high; ++i)
	{
		if (memcmp(&str->buffer[i], target->buffer, target_len) == 0)
		{
			out++;
			i += target_len - 1;
		}
	}

	for (; i < upper; i += EE_SIMD_BYTES)
	{
		ee_simd_i group = ee_loadu_si((const ee_simd_i*)&str->buffer[i]);
		ee_simd_i match = ee_cmpeq_epi8(group, mask);
		
		i32 match_mask = ee_movemask_epi8(match);

		while (match_mask)
		{
			i32 first = ee_first_bit_u32(match_mask);

			if ((i + first + target_len <= high) && memcmp(&str->buffer[i + first], target->buffer, target_len) == 0)
			{
				out++;

				u64 remove = (1ull << (first + target_len)) - 1ull;
				match_mask &= ~(i32)remove;
			}
			else
			{
				match_mask &= match_mask - 1;
			}
		}
	}

	for (; i < high && i + target_len <= high; ++i)
	{
		if (memcmp(&str->buffer[i], target->buffer, target_len) == 0)
		{
			out++;
			i += target_len - 1;
		}
	}

	return out;
}

EE_INLINE size_t ee_str_count(const Str* str, const Str* target)
{
	return ee_str_count_b(str, target, 0, str->top);
}

EE_INLINE size_t ee_str_replace_b(Str* str, const Str* old_str, const Str* new_str, size_t max_count, size_t low, size_t high)
{
	EE_ASSERT(str != NULL, "Trying to replace in NULL string");
	EE_ASSERT(old_str != NULL, "Trying to replace NULL old substring");
	EE_ASSERT(new_str != NULL, "Trying to replace with NULL new substring");

	i64 old_len = old_str->top;
	i64 new_len = new_str->top;
	i64 str_len = str->top;

	i64 replace_count = ee_str_count_b(str, old_str, low, high);
	i64 new_str_len = str_len + replace_count * (new_len - old_len);

	EE_ASSERT(new_str_len > 0, "Invalid resulting length of the buffer");

	char* new_buffer = (char*)str->allocator.alloc_fn(&str->allocator, new_str_len);

	EE_ASSERT(new_buffer != NULL, "Unable to allocate (%zd) bytes for temporary buffer", new_str_len);

	ee_simd_i mask = ee_set1_epi8(old_str->buffer[0]);

	size_t count = 0;
	size_t i = 0;
	size_t upper = str_len & ~(EE_SIMD_BYTES - 1);

	// TODO

	return count;
}

EE_INLINE void ee_str_set(Str* str, size_t i, char symbol)
{
	EE_ASSERT(str != NULL, "Trying to set into NULL string");
	EE_ASSERT(i < str->top, "Invalid set index (%zu) for string with length (%zu)", i, str->top);

	str->buffer[i] = symbol;
}

EE_INLINE char ee_str_get(const Str* str, size_t i)
{
	EE_ASSERT(str != NULL, "Trying to get from NULL string");
	EE_ASSERT(i < str->top, "Invalid get index (%zu) for string with length (%zu)", i, str->top);

	return str->buffer[i];
}

EE_INLINE char* ee_str_at(const Str* str, size_t i)
{
	EE_ASSERT(str != NULL, "Trying to get from NULL string");
	EE_ASSERT(i < str->top, "Invalid get index (%zu) for string with length (%zu)", i, str->top);

	return &str->buffer[i];
}

EE_INLINE size_t ee_str_len(const Str* str)
{
	EE_ASSERT(str != NULL, "Trying to get length of the NULL string");

	return str->top;
}

EE_INLINE i32 ee_str_lev_m64(const Str* a, const Str* b)
{
	EE_ASSERT(a != NULL, "Trying to compute distance from NULL 'a' string");
	EE_ASSERT(b != NULL, "Trying to compute distance from NULL 'b' string");
	EE_ASSERT(a->top < EE_STR_LEV_BLOCK_SIZE && b->top < EE_STR_LEV_BLOCK_SIZE,
		"Max string length for this function is (%d), a: (%zu), b: (%zu)", EE_STR_LEV_BLOCK_SIZE, a->top, b->top);
	EE_ASSERT(a->top >= b->top, "'a' string should be longer that 'b'");

	if (a->top == 0)
		return (i32)b->top;

	if (b->top == 0)
		return (i32)a->top;

	u64 char_equal[EE_STR_CHARS_MASK_LEN] = { 0 };

	for (size_t i = 0; i < a->top; ++i)
	{
		char symbol = a->buffer[i];

		char_equal[symbol] |= 1ull << i;
	}

	u64 pos_vec = ~0ull;
	u64 neg_vec = 0;
	u64 last = 1ull << (a->top - 1);

	i32 score = (i32)a->top;

	for (size_t j = 0; j < b->top; ++j)
	{
		char symbol = b->buffer[j];
		u64 equal = char_equal[symbol];
		u64 xv = equal | neg_vec;

		equal = equal | (((equal & pos_vec) + pos_vec) ^ pos_vec);
		neg_vec = neg_vec | ~(equal | pos_vec);
		pos_vec = pos_vec & equal;

		if (neg_vec & last)
			score++;

		if (pos_vec & last)
			score--;

		neg_vec = (neg_vec << 1) | 1ull;
		pos_vec = (pos_vec << 1) | ~(xv | neg_vec);
		neg_vec = neg_vec & xv;
	}

	return score;
}

EE_INLINE i32 ee_str_lev_mx(const Str* a, const Str* b) 
{
	EE_ASSERT(a != NULL, "Trying to compute distance from NULL 'a' string");
	EE_ASSERT(b != NULL, "Trying to compute distance from NULL 'b' string");
	EE_ASSERT(a->top >= b->top, "'a' string should be longer that 'b'");


	if (a->top == 0)
		return (i32)b->top;

	if (b->top == 0)
		return (i32)a->top;

	size_t n = a->top;
	size_t m = b->top;

	size_t hsize = 1 + ((n - 1) >> EE_UINT64_SHIFT);
	size_t vsize = 1 + ((m - 1) >> EE_UINT64_SHIFT);

	u64* buffer = (u64*)malloc(2 * hsize * sizeof(u64));

	EE_ASSERT(buffer != NULL, "Unable to allocate (%zu) bytes for internal buffer", 2 * hsize * sizeof(u64));

	u64* phc = buffer;
	u64* mhc = &buffer[hsize];
	
	for (size_t i = 0; i < hsize; ++i) 
	{
		phc[i] = ~0ull;
		mhc[i] = 0;
	}

	u64 char_equal[EE_STR_CHARS_MASK_LEN] = { 0 };
	i32 score = (i32)m;

	size_t block_index = 0;

	for (; block_index < vsize - 1; ++block_index) 
	{
		u64 neg_vec = 0;
		u64 pos_vec = ~0ull;

		size_t row_start = block_index << EE_UINT64_SHIFT;
		size_t row_end = ee_min_u64(64, m - row_start) + row_start;

		for (size_t k = row_start; k < row_end; ++k) 
		{
			char symbol = b->buffer[k];
			char_equal[symbol] |= 1ull << (k & EE_UINT64_MASK);
		}

		for (size_t i = 0; i < n; ++i) 
		{
			char symbol = a->buffer[i];
			u64 equal = char_equal[symbol];

			u64 pv_bit = (phc[i >> EE_UINT64_SHIFT] >> (i & EE_UINT64_MASK)) & 1ull;
			u64 mv_bit = (mhc[i >> EE_UINT64_SHIFT] >> (i & EE_UINT64_MASK)) & 1ull;

			u64 xv = equal | neg_vec;
			u64 xh = ((((equal | mv_bit) & pos_vec) + pos_vec) ^ pos_vec) | equal | mv_bit;

			u64 ph = neg_vec | ~(xh | pos_vec);
			u64 mh = pos_vec & xh;

			if ((ph >> 63) ^ pv_bit) 
			{
				phc[i >> EE_UINT64_SHIFT] ^= 1ull << (i & EE_UINT64_MASK);
			}

			if ((mh >> 63) ^ mv_bit) 
			{
				mhc[i >> EE_UINT64_SHIFT] ^= 1ull << (i & EE_UINT64_MASK);
			}

			ph = (ph << 1) | pv_bit;
			mh = (mh << 1) | mv_bit;

			pos_vec = mh | ~(xv | ph);
			neg_vec = ph & xv;
		}

		for (size_t k = row_start; k < row_end; ++k) 
		{
			char_equal[b->buffer[k]] = 0;
		}
	}

	u64 neg_vec = 0;
	u64 pos_vec = ~0ull;

	size_t row_start = block_index << EE_UINT64_SHIFT;
	size_t row_end = ee_min_u64(64, m - row_start) + row_start;

	for (size_t k = row_start; k < row_end; ++k) 
	{
		char symbol = b->buffer[k];
		char_equal[symbol] |= 1ull << (k & EE_UINT64_MASK);
	}

	for (size_t i = 0; i < n; ++i) 
	{
		char symbol = a->buffer[i];
		u64 equal = char_equal[symbol];

		u64 pv_bit = (phc[i >> EE_UINT64_SHIFT] >> (i & EE_UINT64_MASK)) & 1ull;
		u64 mv_bit = (mhc[i >> EE_UINT64_SHIFT] >> (i & EE_UINT64_MASK)) & 1ull;

		u64 xv = equal | neg_vec;
		u64 xh = ((((equal | mv_bit) & pos_vec) + pos_vec) ^ pos_vec) | equal | mv_bit;

		u64 ph = neg_vec | ~(xh | pos_vec);
		u64 mh = pos_vec & xh;

		score += (ph >> ((m - 1) & 63)) & 1ull;
		score -= (mh >> ((m - 1) & 63)) & 1ull;

		if ((ph >> 63) ^ pv_bit) 
		{
			phc[i >> EE_UINT64_SHIFT] ^= 1ull << (i & EE_UINT64_MASK);
		}

		if ((mh >> 63) ^ mv_bit) 
		{
			mhc[i >> EE_UINT64_SHIFT] ^= 1ull << (i & EE_UINT64_MASK);
		}

		ph = (ph << 1) | pv_bit;
		mh = (mh << 1) | mv_bit;

		pos_vec = mh | ~(xv | ph);
		neg_vec = ph & xv;
	}

	free(buffer);

	return score;
}

EE_INLINE i32 ee_str_lev(const Str* a, const Str* b)
{
	const Str* shorter = a;
	const Str* longer = b;

	if (a->top > b->top)
	{
		shorter = b;
		longer = a;
	}

	if (longer->top <= 64)
	{
		return ee_str_lev_m64(longer, shorter);
	}

	return ee_str_lev_mx(longer, shorter);
}

EE_INLINE void ee_str_print(const Str* str)
{
	fwrite(str->buffer, 1, str->top, stdout);
}

EE_INLINE Str ee_str_copy(const Str* str, Allocator* allocator)
{
	EE_ASSERT(str != NULL, "Trying to copy into NULL Array");
	EE_ASSERT(str->buffer != NULL, "Trying to copy into NULL Array.buffer");

	Str out = *str;

	if (allocator == NULL)
	{
		out.allocator.alloc_fn = ee_default_alloc;
		out.allocator.realloc_fn = ee_default_realloc;
		out.allocator.free_fn = ee_default_free;
		out.allocator.context = NULL;
	}
	else
	{
		memcpy(&out.allocator, allocator, sizeof(Allocator));
	}

	EE_ASSERT(out.allocator.alloc_fn != NULL, "Trying to set NULL alloc callback");
	EE_ASSERT(out.allocator.realloc_fn != NULL, "Trying to set NULL realloc callback");
	EE_ASSERT(out.allocator.free_fn != NULL, "Trying to set NULL free callback");

	out.buffer = (char*)out.allocator.alloc_fn(&out.allocator, out.cap);

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Str.buffer copy", out.cap);

	memcpy(out.buffer, str->buffer, out.cap);

	return out;
}

EE_INLINE void ee_str_grow_to(Str* str, size_t new_size)
{
	EE_ASSERT(str != NULL, "Trying to grow NULL string");
	EE_ASSERT(str->buffer != NULL, "Trying to grow NULL string buffer");
	EE_ASSERT(str->cap <= new_size, "Trying to shrink string");

	if (new_size == str->cap)
	{
		return;
	}

	size_t new_cap = new_size;
	char* new_buffer = (char*)str->allocator.realloc_fn(&str->allocator, str->buffer, str->cap, new_cap);

	EE_ASSERT(new_buffer != NULL, "Unable to reallocate (%zu) bytes for Str.buffer", new_cap);

	str->cap = new_cap;
	str->buffer = new_buffer;
}

EE_INLINE void ee_str_push_bytes(Str* str, const char* bytes, size_t len)
{
	EE_ASSERT(str != NULL, "Trying to push into NULL string");
	EE_ASSERT(bytes != NULL, "Trying to push NULL bytes");

	size_t need = str->top + len;

	if (need > str->cap)
	{
		size_t new_cap = str->cap ? str->cap : 1;
		
		while (new_cap < need) 
		{ 
			new_cap = new_cap + (new_cap >> 1); 
		}

		ee_str_grow_to(str, new_cap);
	}

	memcpy(&str->buffer[str->top], bytes, len);
	str->top += len;
}

EE_INLINE void ee_str_fill_free(Str* str, char val)
{
	EE_ASSERT(str != NULL, "Trying to fill NULL string");

	if (ee_str_full(str))
	{
		return;
	}

	memset(&str->buffer[str->top], val, str->cap - str->top);
	str->top = str->cap;
}

EE_INLINE void ee_str_clear_free(Str* str, char val)
{
	EE_ASSERT(str != NULL, "Trying to clear NULL string");

	if (ee_str_full(str))
	{
		return;
	}

	memset(&str->buffer[str->top], val, str->cap - str->top);
}

EE_INLINE void ee_str_clear(Str* str, char val)
{
	EE_ASSERT(str != NULL, "Trying to clear NULL string");

	memset(str->buffer, val, str->cap);
}

EE_INLINE void ee_str_clear_zero(Str* str)
{
	EE_ASSERT(str != NULL, "Trying to clear NULL string");

	memset(str->buffer, 0, str->cap);
}
EE_INLINE void ee_str_insert_bytes(Str* str, size_t i, const char* bytes, size_t len)
{
	EE_ASSERT(str != NULL, "Trying to insert bytes into NULL string");
	EE_ASSERT(bytes != NULL, "Trying to insert NULL bytes");
	EE_ASSERT(i <= str->top, "Invalid position (%zu) for string with top (%zu)", i, str->top);

	if (i == str->top)
	{
		ee_str_push_bytes(str, bytes, len);
		return;
	}

	size_t need = str->top + len;

	if (need > str->cap)
	{
		size_t new_cap = str->cap ? str->cap : 1;
		
		while (new_cap < need) 
		{ 
			new_cap = new_cap + (new_cap >> 1); 
		}

		ee_str_grow_to(str, new_cap);
	}

	memmove(&str->buffer[i + len], &str->buffer[i], str->top - i);
	memcpy(&str->buffer[i], bytes, len);

	str->top += len;
}

EE_INLINE void ee_str_set_bytes(Str* str, size_t i, const char* bytes, size_t len)
{
	EE_ASSERT(str != NULL, "Trying to insert bytes into NULL string");
	EE_ASSERT(bytes != NULL, "Trying to insert NULL bytes");
	EE_ASSERT(i <= str->top, "Invalid position (%zu) for string with top (%zu)", i, str->top);

	size_t new_cap = str->cap;

	while (i + len > new_cap)
	{
		new_cap = new_cap + (new_cap >> 1);
	}

	ee_str_grow_to(str, new_cap);

	memcpy(&str->buffer[i], bytes, len);
	
	if (i + len > str->top)
	{
		str->top = i + len;
	}
}

EE_INLINE void ee_str_reset(Str* str)
{
	EE_ASSERT(str != NULL, "Trying to reset NULL string");

	str->top = 0;
}


EE_INLINE Str_View ee_str_view_new(const char* buffer, size_t len)
{
	EE_ASSERT(buffer != NULL, "Trying to create string view from NULL buffer");

	Str_View out = { 0 };

	out.buffer = buffer;
	out.len = len;

	return out;
}

EE_INLINE Str_View ee_str_view_from_str(const Str* str, size_t pos, size_t len)
{
	EE_ASSERT(str != NULL, "Trying to get view from NULL string");
	EE_ASSERT(pos + len <= ee_str_len(str), "Invalid position or length (%zu, %zu) from string view for stirng with len (%zu)", pos, len, ee_str_len(str));

	Str_View out = { 0 };

	out.buffer = (const char*)ee_str_at(str, pos);
	out.len = len;

	return out;
}

EE_INLINE void ee_str_view_print(Str_View str_view)
{
	EE_ASSERT(str_view.buffer != NULL, "Trying to dereference NULL buffer");
	EE_ASSERT(str_view.len != 0, "Trying to print invalid string view, length (%zu)", str_view.len);

	fwrite(str_view.buffer, 1, str_view.len, stdout);
}

EE_EXTERN_C_END

#endif // EE_STRING_H
