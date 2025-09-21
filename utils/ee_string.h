#ifndef EE_STRING_H
#define EE_STRING_H

#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#pragma warning (disable : 4996)
#endif

#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "stdio.h"

#ifndef EE_NO_ASSERT
#ifndef EE_ASSERT

#define EE_ASSERT(cond, fmt, ...) do {                                    \
	if (!(cond)) {                                                        \
		fprintf(stderr, "[%s][%d][%s] ", __FILE__, __LINE__, __func__);   \
		fprintf(stderr, fmt "\n", ##__VA_ARGS__);                         \
		exit(1);                                                          \
	}                                                                     \
} while (0)

#endif // EE_ASSERT
#else  // EE_NO_ASSERT

#define EE_ASSERT(cond, fmt, ...)    ((void)0)

#endif // EE_NO_ASSERT

#ifndef EE_INLINE
#define EE_INLINE    static inline
#endif // EE_INLINE

#define EE_SS_LEN           (16)
#define EE_LS_PREFIX_LEN    (4)

#ifndef EE_TRUE
#define EE_TRUE             (1)
#endif // EE_TRUE

#ifndef EE_FALSE
#define EE_FALSE            (0)
#endif // EE_FALSE

#ifndef EE_FIND_FIRST_BIT_INVALID
#define EE_FIND_FIRST_BIT_INVALID    (32)
#endif // EE_FIND_FIRST_BIT_INVALID

#if !defined(EE_NO_AVX2)

#include "immintrin.h"

#define EE_SIMD_BYTES         (32)
#define EE_SIMD_BITS          (EE_SIMD_BYTES * 8)
#define EE_SIMD_U64X_PARTS    (4) 

#define ee_loadu_si           _mm256_loadu_si256
#define ee_load_si            _mm256_load_si256
#define ee_set1_epi8          _mm256_set1_epi8
#define ee_set1_epi64x        _mm256_set1_epi64x
#define ee_cmpeq_epi8         _mm256_cmpeq_epi8
#define ee_movemask_epi8      _mm256_movemask_epi8
#define ee_and_si             _mm256_and_si256
#define ee_or_si              _mm256_or_si256
#define ee_xor_si             _mm256_xor_si256
#define ee_slli_epi32         _mm256_slli_epi32
#define ee_srli_epi32         _mm256_srli_epi32
#define ee_slli_epi64         _mm256_slli_epi64
#define ee_srli_epi64         _mm256_srli_epi64
#define ee_add_epi64          _mm256_add_epi64

typedef __m256i ee_simd_i;

typedef struct ee_u64x
{
	union
	{
		struct
		{
			u64 part_0;
			u64 part_1;
			u64 part_2;
			u64 part_3;
		};

		u64 parts[EE_SIMD_U64X_PARTS];
	};
} ee_u64x;

#elif !defined(EE_NO_SSE2)

#include "immintrin.h"

#define EE_SIMD_BYTES         (16)
#define EE_SIMD_BITS          (EE_SIMD_BYTES * 8)
#define EE_SIMD_U64X_PARTS    (2) 

#define ee_loadu_si           _mm_loadu_si128
#define ee_load_si            _mm_load_si128
#define ee_set1_epi8          _mm_set1_epi8
#define ee_set1_epi64x        _mm_set1_epi64x
#define ee_cmpeq_epi8         _mm_cmpeq_epi8
#define ee_movemask_epi8      _mm_movemask_epi8
#define ee_and_si             _mm_and_si128
#define ee_or_si              _mm_or_si128
#define ee_xor_si             _mm_xor_si128
#define ee_slli_epi32         _mm_slli_epi32
#define ee_srli_epi32         _mm_srli_epi32
#define ee_slli_epi64         _mm_slli_epi64
#define ee_srli_epi64         _mm_srli_epi64
#define ee_add_epi64          _mm_add_epi64

typedef __m128i ee_simd_i;

typedef struct ee_u64x
{
	union
	{
		struct
		{
			u64 part_0;
			u64 part_1;
		};

		u64 parts[EE_SIMD_U64X_PARTS];
	};
} ee_u64x;

#else
#error "SIMD does not supported on your machine!"
#endif // EE_SIMD

#ifndef EE_TYPES
#define EE_TYPES

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

typedef float       f32;
typedef double      f64;
typedef long double f80;

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
_Static_assert(sizeof(float) == 4, "f32: sizeof(float) != 4");
_Static_assert(sizeof(double) == 8, "f64: sizeof(double) != 8");
#endif

#endif // EE_TYPES

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

#ifndef EE_ALLOCATOR
#define EE_ALLOCATOR

typedef struct Allocator
{
	void* (*alloc_fn)(struct Allocator* self, size_t size);
	void* (*realloc_fn)(struct Allocator* self, void* buffer, size_t old_size, size_t new_size);
	void  (*free_fn)(struct Allocator* self, void* buffer);
	void* context;
} Allocator;

EE_INLINE void* ee_default_alloc(Allocator * allocator, size_t size)
{
	(void)allocator;

	return malloc(size);
}

EE_INLINE void* ee_default_realloc(Allocator * allocator, void* buffer, size_t old_size, size_t new_size)
{
	(void)allocator;
	(void)old_size;

	return realloc(buffer, new_size);
}

EE_INLINE void ee_default_free(Allocator * allocator, void* buffer)
{
	(void)allocator;

	free(buffer);
}

#endif // EE_ALLOCATOR

typedef struct Str
{
	size_t top;
	size_t cap;
	u8* buffer;
	Allocator allocator;
} Str;

EE_INLINE s32 ee_str_first_bit_u32(u32 x)
{
#if defined(__BMI__)
	return _tzcnt_u32(x);
#elif defined(__GNUC__) || defined(__clang__)
	return x ? __builtin_ctz(x) : EE_FIND_FIRST_BIT_INVALID;
#elif defined(_MSC_VER)
	unsigned long i;

	if (_BitScanForward(&i, x))
	{
		return (s32)i;
	}
	else
	{
		return EE_FIND_FIRST_BIT_INVALID;
	}
#else
	for (s32 i = 0; i < 32; ++i)
	{
		if (x & (1u << i))
		{
			return i;
		}
	}

	return EE_FIND_FIRST_BIT_INVALID;
#endif
}

EE_INLINE s32 ee_str_popcnt_u32(u32 x)
{
#if defined(__GNUC__) || defined(__clang__)
	return __builtin_popcount(x);
#elif defined(_MSC_VER)
	return __popcnt(x);
#else
	s32 count = 0;

	while (x) 
	{
		x &= x - 1;
		count++;
	}

	return count;
#endif
}

EE_INLINE u64 ee_min_u64(u64 a, u64 b)
{
	return a < b ? a : b;
}

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
	out.buffer = out.allocator.alloc_fn(&out.allocator, size);

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Str.buffer", size);

	return out;
}

EE_INLINE Str ee_str_from_cstr(const u8* c_str, const Allocator* allocator)
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

	size_t size = strlen(c_str);

	out.cap = size;
	out.top = size;
	out.buffer = out.allocator.alloc_fn(&out.allocator, out.cap);

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Str.buffer", out.cap);

	memcpy(out.buffer, c_str, size);

	return out;
}

EE_INLINE Str ee_str_from_file(const u8* file_path, const u8* mode, const Allocator* allocator)
{
	EE_ASSERT(file_path != NULL, "Trying to open NULL file path");
	EE_ASSERT(mode == EE_STR_FILE_READ || mode == EE_STR_FILE_READ_BYTES, "Invalid mode for reading from file (%s)", mode);

	if (mode == NULL)
	{
		mode = EE_STR_FILE_READ_BYTES;
	}

	FILE* file = fopen(file_path, mode);

	EE_ASSERT(file != NULL, "Unable to open file (%s)", file_path);

	fseek(file, 0, SEEK_END);
	s32 file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	EE_ASSERT(file_size > 0, "Unable to get file size");

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
	out.buffer = out.allocator.alloc_fn(&out.allocator, out.cap);

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Str.buffer", out.cap);

	size_t bytes_read = fread(out.buffer, 1, file_size, file);
	
	EE_ASSERT(bytes_read == file_size, "Unable to read (%d) bytes from file", file_size);

	fclose(file);

	return out;
}

EE_INLINE const u8* ee_str_to_cstr(Str* str)
{
	EE_ASSERT(str != NULL, "Trying to get c string from NULL string");

	size_t c_str_len = str->top + 1;
	u8* c_str = str->allocator.alloc_fn(&str->allocator, c_str_len);

	EE_ASSERT(c_str != NULL, "Unable to allocate (%zu) bytes for C string buffer", c_str_len);

	memcpy(c_str, str->buffer, str->top);
	c_str[c_str_len - 1] = '\0';

	return (const u8*)c_str;
}

EE_INLINE void ee_str_to_file(Str* str, const u8* file_path, const u8* mode)
{
	EE_ASSERT(str != NULL, "Trying to write a NULL string to file");
	EE_ASSERT(file_path != NULL, "Trying to write to a NULL file path");
	EE_ASSERT(mode == EE_STR_FILE_WRITE || mode == EE_STR_FILE_WRITE_BYTES ||
		mode == EE_STR_FILE_APPEND || mode == EE_STR_FILE_APPEND_BYTES, "Invalid mode for writing to file (%s)", mode);

	if (mode == NULL)
	{
		mode = EE_STR_FILE_APPEND_BYTES;
	}

	FILE* file = NULL;

	file = fopen(file_path, mode);

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
	u8* new_buffer = str->allocator.realloc_fn(&str->allocator, str->buffer, str->cap, new_cap);

	EE_ASSERT(new_buffer != NULL, "Unable to reallocate (%zu) bytes for Str.buffer", new_cap);

	str->cap = new_cap;
	str->buffer = new_buffer;
}

EE_INLINE s32 ee_str_full(const Str* str)
{
	EE_ASSERT(str != NULL, "Trying to check NULL string");

	return str->top >= str->cap;
}

EE_INLINE s32 ee_str_empty(const Str* str)
{
	EE_ASSERT(str != NULL, "Trying to check NULL string");

	return str->top == 0;
}

EE_INLINE void ee_str_push(Str* str, u8 symbol)
{
	EE_ASSERT(str != NULL, "Trying to push into NULL string");

	if (ee_str_full(str))
	{
		ee_str_grow(str);
	}

	str->buffer[str->top++] = symbol;
}

EE_INLINE void ee_str_pop(Str* str, u8* out_val)
{
	EE_ASSERT(str != NULL, "Trying to pop from NULL string");
	EE_ASSERT(!ee_str_empty(str), "Trying to pop from empty string");

	str->top--;

	if (out_val != NULL)
	{
		*out_val = str->buffer[str->top];
	}
}

EE_INLINE s32 ee_str_cmp(const Str* a, const Str* b)
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
		s32 match_mask = ee_movemask_epi8(match);

		while (match_mask)
		{
			s32 first = ee_str_first_bit_u32(match_mask);

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
		
		s32 match_mask = ee_movemask_epi8(match);

		while (match_mask)
		{
			s32 first = ee_str_first_bit_u32(match_mask);

			if ((i + first + target_len <= high) && memcmp(&str->buffer[i + first], target->buffer, target_len) == 0)
			{
				out++;

				u64 remove = (1ull << (first + target_len)) - 1ull;
				match_mask &= ~(s32)remove;
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

	s64 old_len = old_str->top;
	s64 new_len = new_str->top;
	s64 str_len = str->top;

	s64 replace_count = ee_str_count_b(str, old_str, low, high);
	s64 new_str_len = str_len + replace_count * (new_len - old_len);

	EE_ASSERT(new_str_len > 0, "Invalid resulting length of the buffer");

	u8* new_buffer = str->allocator.alloc_fn(&str->allocator, new_str_len);

	EE_ASSERT(new_buffer != NULL, "Unable to allocate (%zd) bytes for temporary buffer", new_str_len);

	ee_simd_i mask = ee_set1_epi8(old_str->buffer[0]);

	size_t count = 0;
	size_t i = 0;
	size_t upper = str_len & ~(EE_SIMD_BYTES - 1);

	// TODO

	return count;
}

EE_INLINE void ee_str_set(Str* str, size_t i, u8 symbol)
{
	EE_ASSERT(str != NULL, "Trying to set into NULL string");
	EE_ASSERT(i < str->top, "Invalid set index (%zu) for string with length (%zu)", i, str->top);

	str->buffer[i] = symbol;
}

EE_INLINE u8 ee_str_get(Str* str, size_t i)
{
	EE_ASSERT(str != NULL, "Trying to get from NULL string");
	EE_ASSERT(i < str->top, "Invalid get index (%zu) for string with length (%zu)", i, str->top);

	return str->buffer[i];
}

EE_INLINE s32 ee_str_lev_m64(const Str* a, const Str* b)
{
	EE_ASSERT(a != NULL, "Trying to compute distance from NULL 'a' string");
	EE_ASSERT(b != NULL, "Trying to compute distance from NULL 'b' string");
	EE_ASSERT(a->top < EE_STR_LEV_BLOCK_SIZE && b->top < EE_STR_LEV_BLOCK_SIZE,
		"Max string length for this function is (%d), a: (%zu), b: (%zu)", EE_STR_LEV_BLOCK_SIZE, a->top, b->top);
	EE_ASSERT(a->top >= b->top, "'a' string should be longer that 'b'");

	if (a->top == 0)
		return (s32)b->top;

	if (b->top == 0)
		return (s32)a->top;

	u64 char_equal[EE_STR_CHARS_MASK_LEN] = { 0 };

	for (size_t i = 0; i < a->top; ++i)
	{
		u8 symbol = a->buffer[i];

		char_equal[symbol] |= 1ull << i;
	}

	u64 pos_vec = ~0ull;
	u64 neg_vec = 0;
	u64 last = 1ull << (a->top - 1);

	s32 score = (s32)a->top;

	for (size_t j = 0; j < b->top; ++j)
	{
		u8 symbol = b->buffer[j];
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

EE_INLINE s32 ee_str_lev_mx(const Str* a, const Str* b) 
{
	EE_ASSERT(a != NULL, "Trying to compute distance from NULL 'a' string");
	EE_ASSERT(b != NULL, "Trying to compute distance from NULL 'b' string");
	EE_ASSERT(a->top >= b->top, "'a' string should be longer that 'b'");


	if (a->top == 0)
		return (s32)b->top;

	if (b->top == 0)
		return (s32)a->top;

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
	s32 score = (s32)m;

	size_t block_index = 0;

	for (; block_index < vsize - 1; ++block_index) 
	{
		u64 neg_vec = 0;
		u64 pos_vec = ~0ull;

		size_t row_start = block_index << EE_UINT64_SHIFT;
		size_t row_end = ee_min_u64(64, m - row_start) + row_start;

		for (size_t k = row_start; k < row_end; ++k) 
		{
			u8 symbol = b->buffer[k];
			char_equal[symbol] |= 1ull << (k & EE_UINT64_MASK);
		}

		for (size_t i = 0; i < n; ++i) 
		{
			u8 symbol = a->buffer[i];
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
		u8 symbol = b->buffer[k];
		char_equal[symbol] |= 1ull << (k & EE_UINT64_MASK);
	}

	for (size_t i = 0; i < n; ++i) 
	{
		u8 symbol = a->buffer[i];
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

EE_INLINE s32 ee_str_lev(const Str* a, const Str* b)
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

#endif // EE_STRING_H
