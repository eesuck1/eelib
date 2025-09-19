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

#define EE_SIMD_BYTES       (32)
#define EE_SIMD_BITS        (EE_SIMD_BYTES * 8)

#define ee_loadu_si         _mm256_loadu_si256
#define ee_set1_epi8        _mm256_set1_epi8
#define ee_cmpeq_epi8       _mm256_cmpeq_epi8
#define ee_movemask_epi8    _mm256_movemask_epi8

typedef __m256i ee_simd_i;

#elif !defined(EE_NO_SSE2)

#include "immintrin.h"

#define EE_SIMD_BYTES       (16)
#define EE_SIMD_BITS        (EE_SIMD_BYTES * 8)

#define ee_loadu_si         _mm_loadu_si128
#define ee_set1_epi8        _mm_set1_epi8
#define ee_cmpeq_epi8       _mm_cmpeq_epi8
#define ee_movemask_epi8    _mm_movemask_epi8

typedef __m128i ee_simd_i;

#else
#error "SIMD does not supported on your machine!"
#endif


#define EE_STR_LEV_BLOCK_SIZE    (64)
#define EE_STR_CHARS_MASK_LEN    (0xFF)
#define EE_STR_INVALID           (0xffffffffffffffffull)

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
	uint8_t* buffer;
	Allocator allocator;
} Str;

EE_INLINE int32_t ee_str_first_bit_u32(uint32_t x)
{
#if defined(__BMI__)
	return _tzcnt_u32(x);
#elif defined(__GNUC__) || defined(__clang__)
	return x ? __builtin_ctz(x) : EE_FIND_FIRST_BIT_INVALID;
#elif defined(_MSC_VER)
	unsigned long i;

	if (_BitScanForward(&i, x))
	{
		return (int32_t)i;
	}
	else
	{
		return EE_FIND_FIRST_BIT_INVALID;
	}
#else
	for (int32_t i = 0; i < 32; ++i)
	{
		if (x & (1u << i))
		{
			return i;
		}
	}

	return EE_FIND_FIRST_BIT_INVALID;
#endif
}

EE_INLINE int32_t ee_str_popcnt_u32(uint32_t x)
{
#if defined(__GNUC__) || defined(__clang__)
	return __builtin_popcount(x);
#elif defined(_MSC_VER)
	return __popcnt(x);
#else
	int32_t count = 0;

	while (x) 
	{
		x &= x - 1;
		count++;
	}

	return count;
#endif
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

	size_t size = strlen(c_str);

	out.cap = size;
	out.top = size;
	out.buffer = out.allocator.alloc_fn(&out.allocator, out.cap);

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Str.buffer", out.cap);

	memcpy(out.buffer, c_str, size);

	return out;
}

EE_INLINE Str ee_str_from_file(const char* file_path, const Allocator* allocator)
{
	EE_ASSERT(file_path != NULL, "Trying to open NULL file path");

	FILE* file = fopen(file_path, "r");

	EE_ASSERT(file != NULL, "Unable to open file (%s)", file_path);

	fseek(file, 0, SEEK_END);
	int32_t file_size = ftell(file);
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
	
	EE_ASSERT(bytes_read == file_size, "Unable to read (%zu) bytes from file", file_size);

	fclose(file);

	return out;
}

EE_INLINE const char* ee_str_to_cstr(Str* str)
{
	EE_ASSERT(str != NULL, "Trying to get c string from NULL string");

	size_t c_str_len = str->top + 1;
	char* c_str = str->allocator.alloc_fn(&str->allocator, c_str_len);

	EE_ASSERT(c_str != NULL, "Unable to allocate (%zu) bytes for C string buffer", c_str_len);

	memcpy(c_str, str->buffer, str->top);
	c_str[c_str_len - 1] = '\0';

	return (const char*)c_str;
}

EE_INLINE void ee_str_to_file(Str* str, const char* file_path, const char* mode)
{
	EE_ASSERT(str != NULL, "Trying to write a NULL string to file");
	EE_ASSERT(file_path != NULL, "Trying to write to a NULL file path");

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
	uint8_t* new_buffer = str->allocator.realloc_fn(&str->allocator, str->buffer, str->cap, new_cap);

	EE_ASSERT(new_buffer != NULL, "Unable to reallocate (%zu) bytes for Str.buffer");

	str->cap = new_cap;
	str->buffer = new_buffer;
}

EE_INLINE int32_t ee_str_full(const Str* str)
{
	EE_ASSERT(str != NULL, "Trying to check NULL string");

	return str->top >= str->cap;
}

EE_INLINE int32_t ee_str_empty(const Str* str)
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

EE_INLINE int32_t ee_str_cmp(const Str* a, const Str* b)
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

EE_INLINE int64_t ee_str_finds_b(const Str* str, const char* target, size_t low, size_t high)
{
	EE_ASSERT(str != NULL, "Trying to search in NULL string");
	EE_ASSERT(target != NULL, "Trying to search a NULL target");
	EE_ASSERT(high <= str->top, "Invalid upper bound");
	EE_ASSERT(low <= str->top, "Invalid lower bound");
	EE_ASSERT(low < high, "Trying to set lower bound bigger than upper bound (%zu, %zu)", low, high);

	size_t target_len = strnlen(target, str->top);

	EE_ASSERT(target_len < high - low, "Too long target char array for bounds (%zu, %zu)", low, high);

	ee_simd_i mask = ee_set1_epi8(target[0]);

	size_t i = low;

	for (; i < high; i += EE_SIMD_BYTES)
	{
		ee_simd_i group = ee_loadu_si((const ee_simd_i*)&str->buffer[i]);
		ee_simd_i match = ee_cmpeq_epi8(group, mask);

		int32_t match_mask = ee_movemask_epi8(match);

		while (match_mask)
		{
			int32_t first = ee_str_first_bit_u32(match_mask);

			if ((i + first + target_len <= high) && (memcmp(&str->buffer[i + first], target, target_len) == 0))
			{
				return i + first;
			}

			match_mask = match_mask & (~(1 << first));
		}
	}

	return EE_STR_INVALID;
}

EE_INLINE size_t ee_str_find_b(const Str* str, const Str* target, size_t low, size_t high)
{
	EE_ASSERT(str != NULL, "Trying to search in NULL string");
	EE_ASSERT(target != NULL, "Trying to search a NULL target");
	EE_ASSERT(high <= str->top, "Invalid upper bound");
	EE_ASSERT(low <= str->top, "Invalid lower bound");
	EE_ASSERT(low < high, "Trying to set lower bound bigger than upper bound (%zu, %zu)", low, high);

	size_t target_len = target->top;

	EE_ASSERT(target_len < high - low, "Too long target char array for bounds (%zu, %zu)", low, high);

	ee_simd_i mask = ee_set1_epi8(target->buffer[0]);

	size_t i = low;

	for (; i < high; i += EE_SIMD_BYTES)
	{
		ee_simd_i group = ee_loadu_si((const ee_simd_i*) & str->buffer[i]);
		ee_simd_i match = ee_cmpeq_epi8(group, mask);

		int32_t match_mask = ee_movemask_epi8(match);

		while (match_mask)
		{
			int32_t first = ee_str_first_bit_u32(match_mask);

			if ((i + first + target_len <= high) && (memcmp(&str->buffer[i + first], target->buffer, target_len) == 0))
			{
				return i + first;
			}

			match_mask = match_mask & (~(1 << first));
		}
	}

	return EE_STR_INVALID;
}

EE_INLINE int64_t ee_str_findc_b(const Str* str, char target, size_t low, size_t high)
{
	EE_ASSERT(str != NULL, "Trying to search in NULL string");
	EE_ASSERT(high <= str->top, "Invalid upper bound");
	EE_ASSERT(low <= str->top, "Invalid lower bound");
	EE_ASSERT(low < high, "Trying to set lower bound bigger than upper bound (%zu, %zu)", low, high);

	ee_simd_i mask = ee_set1_epi8(target);

	size_t i = low;

	for (; i < high; i += EE_SIMD_BYTES)
	{
		ee_simd_i group = ee_loadu_si((const ee_simd_i*)&str->buffer[i]);
		ee_simd_i match = ee_cmpeq_epi8(group, mask);

		int32_t match_mask = ee_movemask_epi8(match);

		if (match_mask)
		{
			int32_t first = ee_str_first_bit_u32(match_mask);

			if (i + first < high)
			{
				return i + first;
			}

			return EE_STR_INVALID;
		}
	}

	return EE_STR_INVALID;
}

EE_INLINE size_t ee_str_finds(const Str* str, const char* target)
{
	return ee_str_finds_b(str, target, 0, str->top);
}

EE_INLINE size_t ee_str_findc(const Str* str, char target)
{
	return ee_str_findc_b(str, target, 0, str->top);
}

EE_INLINE size_t ee_str_find(const Str* str, const Str* target)
{
	return ee_str_find_b(str, target, 0, str->top);
}

EE_INLINE size_t ee_str_countc_b(const Str* str, char target, size_t low, size_t high)
{
	EE_ASSERT(str != NULL, "Trying to search in NULL string");
	EE_ASSERT(high <= str->top, "Invalid upper bound");
	EE_ASSERT(low <= str->top, "Invalid lower bound");
	EE_ASSERT(low < high, "Trying to set lower bound bigger than upper bound (%zu, %zu)", low, high);

	ee_simd_i mask = ee_set1_epi8(target);

	size_t out = 0;
	size_t i = low;

	// NOTE: last bytes can be done after loop and if condition would be unnecessary
	for (; i < high; i += EE_SIMD_BYTES)
	{
		ee_simd_i group = ee_loadu_si((const ee_simd_i*)&str->buffer[i]);
		ee_simd_i match = ee_cmpeq_epi8(group, mask);

		int32_t match_mask = ee_movemask_epi8(match);

		if (match_mask)
		{
			out += ee_str_popcnt_u32(match_mask);
		}
	}

	return out;
}

EE_INLINE int32_t ee_str_lev_m64(const Str* a, const Str* b)
{
	EE_ASSERT(a != NULL, "Trying to compute distance from NULL 'a' string");
	EE_ASSERT(b != NULL, "Trying to compute distance from NULL 'b' string");
	EE_ASSERT(a->top < EE_STR_LEV_BLOCK_SIZE && b->top < EE_STR_LEV_BLOCK_SIZE, 
		"Max string length for this function is (%d), a: (%zu), b: (%zu)", EE_STR_LEV_BLOCK_SIZE, a->top, b->top);

	if (a->top == 0)
		return b->top;

	if (b->top == 0)
		return a->top;

	const Str* shorter = a;
	const Str* longer = b;

	if (a->top > b->top)
	{
		shorter = b;
		longer = a;
	}

	uint64_t char_equal[EE_STR_CHARS_MASK_LEN] = { 0 };

	for (size_t i = 0; i < longer->top; ++i)
	{
		uint8_t symbol = longer->buffer[i];

		char_equal[symbol] |= 1ull << i;
	}

	uint64_t pos_vec = ~0ull;
	uint64_t neg_vec = 0;
	uint64_t last = 1ull << (longer->top - 1);

	int32_t score = longer->top;

	for (size_t j = 0; j < shorter->top; ++j)
	{
		uint8_t symbol = shorter->buffer[j];
		uint64_t equal = char_equal[symbol];
		uint64_t xv = equal | neg_vec;

		equal = equal | (((equal & pos_vec) + pos_vec) ^ pos_vec);
		neg_vec = neg_vec | (~(equal | pos_vec));
		pos_vec = pos_vec & equal;

		if (neg_vec & last)
			score++;

		if (pos_vec & last)
			score--;

		neg_vec = (neg_vec << 1) | 1ull;
		pos_vec = (pos_vec << 1) | ~(xv | neg_vec);
		neg_vec = pos_vec & xv;
	}

	return score;
}

#endif // EE_STRING_H
