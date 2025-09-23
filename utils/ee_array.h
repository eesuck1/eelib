#pragma once

#ifndef EE_ARRAY_H
#define EE_ARRAY_H

#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "immintrin.h"

#ifndef EE_NO_ASSERT
#ifndef EE_ASSERT
#include "stdio.h"

#define EE_ASSERT(cond, fmt, ...) do {                                    \
	if (!(cond)) {                                                        \
		fprintf(stderr, "[%s][%d][%s] ", __FILE__, __LINE__, __func__);   \
		fprintf(stderr, fmt "\n", ##__VA_ARGS__);                         \
		exit(1);                                                          \
	}                                                                     \
} while (0)
#endif // EE_ASSERT
#else
#define EE_ASSERT(cond, fmt, ...)    ((void)0)
#endif // EE_NO_ASSERT

#ifndef EE_INLINE
#define EE_INLINE    static inline
#endif // EE_INLINE

#ifndef EE_TRUE
#define EE_TRUE     (1)
#endif // EE_TRUE

#ifndef EE_FALSE
#define EE_FALSE    (0)
#endif // EE_FALSE

#ifndef EE_FIND_FIRST_BIT_INVALID
#define EE_FIND_FIRST_BIT_INVALID    (32)
#endif // EE_FIND_FIRST_BIT_INVALID

#ifndef EE_ALLOCA
#ifdef _MSC_VER

#ifdef EE_USE_MALLOCA
#define EE_ALLOCA(size)    (_malloca(size))
#define EE_FREEA(ptr)      (_freea(ptr))
#else
#define EE_ALLOCA(size)    (alloca(size))
#define EE_FREEA(ptr)      ((void)(ptr))
#endif // EE_USE_MALLOCA

#else

#define EE_ALLOCA(size)    (alloca(size))
#define EE_FREEA(ptr)      ((void)(ptr))

#endif // _MSC_VER
#endif // EE_ALLOCA

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

#define EE_ARRAY_DT(x)                   ((u8*)(&(x)))
#define EE_ARRAY_INVALID                 (0xffffffffffffffffull)
#define EE_ARRAY_SORT_TH                 (16)
#define EE_ARRAY_AT(v_ptr, i, dtype)     ((dtype*)ee_array_at(v_ptr, i))
#define EE_ARRAY_GET(v_ptr, i, dtype)    (*(dtype*)ee_array_at(v_ptr, i))

#ifndef EE_ALLOCATOR
#define EE_ALLOCATOR

typedef struct Allocator
{
	void* (*alloc_fn)(struct Allocator* self, size_t size);
	void* (*realloc_fn)(struct Allocator* self, void* buffer, size_t old_size, size_t new_size);
	void  (*free_fn)(struct Allocator* self, void* buffer);
	void* context;
} Allocator;

EE_INLINE void* ee_default_alloc(Allocator* allocator, size_t size)
{
	(void)allocator;

	return malloc(size);
}

EE_INLINE void* ee_default_realloc(Allocator* allocator, void* buffer, size_t old_size, size_t new_size)
{
	(void)allocator;
	(void)old_size;

	return realloc(buffer, new_size);
}

EE_INLINE void ee_default_free(Allocator* allocator, void* buffer)
{
	(void)allocator;

	free(buffer);
}

#endif // EE_ALLOCATOR


typedef struct Array
{
	size_t top;
	size_t cap;
	size_t elem_size;
	u8* buffer;
	Allocator allocator;
} Array;

#ifndef EE_BIN_CMP
#define EE_BIN_CMP
typedef int (*BinCmp)(const void* a, const void* b);
#endif // EE_BIN_CMP

typedef enum ArraySortType
{
	EE_SORT_DEFAULT = 0,
	EE_SORT_INSERT  = 1,
	EE_SORT_QUICK   = 2,
	EE_SORT_HEAP    = 3,
	EE_SORT_INTRO   = 4,
} ArraySortType;

EE_INLINE int ee_is_pow2(u64 x)
{
	return (x != 0) && ((x & (x - 1)) == 0);
}

#if defined(_MSC_VER)
	#include "intrin.h"
#endif

EE_INLINE s32 ee_array_first_bit_u32(u32 x)
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

EE_INLINE int ee_array_log2_u32(u32 x)
{
#if defined(__GNUC__) || defined(__clang__)
	return x ? 31 - __builtin_clz(x) : -1;
#elif defined(_MSC_VER)
	unsigned long i;

	if (_BitScanReverse(&i, x))
	{
		return i;
	}
	else
	{
		return -1;
	}
#else
	int out = -1;

	while (x) 
	{
		out++;
		x >>= 1;
	}

	return out;
#endif
}

EE_INLINE Array ee_array_new(size_t size, size_t elem_size, Allocator* allocator)
{
	EE_ASSERT(size > 0, "Invalid arraytor size (%zu)", size);
	EE_ASSERT(elem_size > 0, "Invalid arraytor elem_size (%zu)", elem_size);

	Array out = { 0 };

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

	out.top = 0;
	out.cap = elem_size * size;
	out.elem_size = elem_size;
	out.buffer = (u8*)out.allocator.alloc_fn(&out.allocator, out.cap);

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Array.buffer", out.cap);
	
	return out;
}

EE_INLINE int ee_array_full(Array* array)
{
	EE_ASSERT(array != NULL, "Trying to check NULL Array");

	return array->top >= array->cap;
}

EE_INLINE int ee_array_empty(Array* array)
{
	return array->top < array->elem_size;
}

EE_INLINE size_t ee_array_len(Array* array)
{
	return array->top / array->elem_size;
}

EE_INLINE size_t ee_array_size(Array* array)
{
	return array->top;
}

EE_INLINE void ee_array_free(Array* array)
{
	EE_ASSERT(array != NULL, "Trying to free NULL Array");
	EE_ASSERT(array->buffer != NULL, "Trying to free NULL Array.buffer");

	array->allocator.free_fn(&array->allocator, array->buffer);

	memset(array, 0, sizeof(Array));
}

EE_INLINE void ee_array_clear(Array* array)
{
	EE_ASSERT(array != NULL, "Trying to clear NULL Array");
	EE_ASSERT(array->buffer != NULL, "Trying to clear NULL Array.buffer");

	array->top = 0;
}

EE_INLINE void ee_array_reserve(Array* array, size_t size)
{
	// TODO(eesuck): improve to handle shrinking

	EE_ASSERT(array != NULL, "Trying to reserve NULL Array");
	EE_ASSERT(array->buffer != NULL, "Trying to reallocate NULL Array.buffer");
	EE_ASSERT(size * array->elem_size > array->cap, "Reserve expects Array to grow, given size (%zu) current capacity (%zu)", size, array->cap / array->elem_size);

	size_t new_cap = size * array->elem_size;
	u8* new_buffer = (u8*)array->allocator.realloc_fn(&array->allocator, array->buffer, array->cap, new_cap);
	
	EE_ASSERT(new_buffer != NULL, "Unable to reallocate (%zu) bytes for Array.buffer", new_cap);
	
	array->cap = size * array->elem_size;
	array->buffer = new_buffer;
}

EE_INLINE void ee_array_grow(Array* array)
{
	EE_ASSERT(array != NULL, "Trying to grow NULL Array");
	EE_ASSERT(array->buffer != NULL, "Trying to reallocate NULL Array.buffer");

	size_t new_cap = array->cap + (array->cap >> 1);
	u8* new_buffer = (u8*)array->allocator.realloc_fn(&array->allocator, array->buffer, array->cap, new_cap);
	
	EE_ASSERT(new_buffer != NULL, "Unable to reallocate (%zu) bytes for Array.buffer", new_cap);
	
	array->cap = new_cap;
	array->buffer = new_buffer;
}

EE_INLINE void ee_array_push(Array* array, u8* val)
{
	EE_ASSERT(array != NULL, "Trying to push into NULL Array");
	EE_ASSERT(val != NULL, "Trying to push NULL value");

	if (ee_array_full(array))
	{
		ee_array_grow(array);
	}

	memcpy(&array->buffer[array->top], val, array->elem_size);

	array->top += array->elem_size;
}

EE_INLINE void ee_array_push_zero(Array* array)
{
	EE_ASSERT(array != NULL, "Trying to push into NULL Array");

	if (ee_array_full(array))
	{
		ee_array_grow(array);
	}

	memset(&array->buffer[array->top], 0, array->elem_size);

	array->top += array->elem_size;
}

EE_INLINE void ee_array_push_nothing(Array* array)
{
	EE_ASSERT(array != NULL, "Trying to push into NULL Array");

	if (ee_array_full(array))
	{
		ee_array_grow(array);
	}

	array->top += array->elem_size;
}

EE_INLINE u8* ee_array_top(Array* array)
{
	EE_ASSERT(array != NULL, "Trying to get NULL Array top");
	EE_ASSERT(array->top >= array->elem_size, "Trying to get top element of empty Array");

	return &array->buffer[array->top - array->elem_size];
}

EE_INLINE u8* ee_array_at(Array* array, size_t i)
{
	EE_ASSERT(array != NULL, "Trying to get NULL Array element");
	EE_ASSERT(i * array->elem_size < array->top, "Index (%zu) is out of bounds for arraytor with top (%zu)", i, array->top / array->elem_size);

	return &array->buffer[i * array->elem_size];
}

EE_INLINE void ee_array_pop(Array* array, u8* out_val)
{
	EE_ASSERT(array != NULL, "Trying to pop NULL Array");
	EE_ASSERT(array->top >= array->elem_size, "Trying to pop empty Array");

	array->top -= array->elem_size;

	if (out_val != NULL)
	{
		memcpy(out_val, &array->buffer[array->top], array->elem_size);
	}
}

EE_INLINE void ee_array_set(Array* array, size_t i, u8* val)
{
	EE_ASSERT(array != NULL, "Trying to set into NULL Array");
	EE_ASSERT(array->top >= i * array->elem_size, "Invalid setting index (%zu) Array.top at position (%zu)", i, array->top / array->elem_size);

	memcpy(&array->buffer[i * array->elem_size], val, array->elem_size);
}

EE_INLINE size_t ee_array_find(Array* array, u8* target)
{
	EE_ASSERT(array != NULL, "Trying to find in NULL Array");
	EE_ASSERT(target != NULL, "Trying to find a NULL value");

	for (size_t i = 0; i < array->top; i += array->elem_size)
	{
		if (memcmp(target, &array->buffer[i], array->elem_size) == 0)
		{
			return i / array->elem_size;
		}
	}

	return EE_ARRAY_INVALID;
}

EE_INLINE void ee_array_insert(Array* array, size_t i, u8* val)
{
	EE_ASSERT(array != NULL, "Trying to insert into NULL Array");
	EE_ASSERT(i <= array->top / array->elem_size, "Index out of bounds");

	if (ee_array_full(array))
	{
		ee_array_grow(array);
	}

	size_t offset = i * array->elem_size;

	memmove(&array->buffer[offset + array->elem_size], &array->buffer[offset], array->top - offset);
	memcpy(&array->buffer[i * array->elem_size], val, array->elem_size);

	array->top += array->elem_size;
}

EE_INLINE void ee_array_erase(Array* array, size_t i)
{
	EE_ASSERT(array != NULL, "Trying to erase from NULL Array");
	EE_ASSERT(i < array->top / array->elem_size, "Index out of bounds");

	size_t offset = i * array->elem_size;

	memmove(&array->buffer[offset], &array->buffer[offset + array->elem_size], array->top - offset - array->elem_size);

	array->top -= array->elem_size;
}

EE_INLINE void ee_array_swap(Array* array, size_t i, size_t j)
{
	EE_ASSERT(array != NULL, "Trying to swap an elements in NULL Array");
	EE_ASSERT(ee_array_size(array) >= 2, "Trying to swap Array with number of elements (%zu), minimum is 2", ee_array_size(array));

	u8* temp = (u8*)EE_ALLOCA(array->elem_size);

	memcpy(temp, ee_array_at(array, i), array->elem_size);
	memcpy(ee_array_at(array, i), ee_array_at(array, j), array->elem_size);
	memcpy(ee_array_at(array, j), temp, array->elem_size);

	EE_FREEA(temp);
}

EE_INLINE void ee_array_insertsort(Array* array, BinCmp cmp, s64 low, s64 high)
{
	EE_ASSERT(array != NULL, "Trying to sort a NULL Array");
	EE_ASSERT(cmp != NULL, "Trying to sort with a NULL BinCmp");

	if (ee_array_empty(array))
	{
		return;
	}

	u8* temp = (u8*)EE_ALLOCA(array->elem_size);
	EE_ASSERT(temp != NULL, "Unable to allocate (%zu) for sorting temporary buffer", array->elem_size);

	for (s64 i = low; i <= high; i += array->elem_size)
	{
		memcpy(temp, &array->buffer[i], array->elem_size);

		s64 j = i;

		while (j > low && cmp(&array->buffer[j - array->elem_size], temp) > 0)
		{
			memcpy(&array->buffer[j], &array->buffer[j - array->elem_size], array->elem_size);
			j -= array->elem_size;
		}

		memcpy(&array->buffer[j], temp, array->elem_size);
	}

	EE_FREEA(temp);
}

EE_INLINE void ee_array_quicksort(Array* array, BinCmp cmp, s64 low, s64 high)
{
	EE_ASSERT(array != NULL, "Trying to sort a NULL Array");
	EE_ASSERT(cmp != NULL, "Trying to sort with a NULL BinCmp");

	if (ee_array_empty(array))
		return;

	if (low < 0 || high < 0 || low >= high)
		return;

	u8* temp = (u8*)EE_ALLOCA(array->elem_size);
	u8* pivot = (u8*)EE_ALLOCA(array->elem_size);
	
	EE_ASSERT(temp != NULL, "Unable to allocate (%zu) for sorting temporary buffer", array->elem_size);
	EE_ASSERT(pivot != NULL, "Unable to allocate (%zu) for pivot", array->elem_size);
	
	size_t low_index = low / array->elem_size;
	size_t high_index = high / array->elem_size;
	size_t mid_index = (low_index + high_index) >> 1;
	
	memcpy(pivot, &array->buffer[mid_index * array->elem_size], array->elem_size);

	s64 i = low - array->elem_size;
	s64 j = high + array->elem_size;

	while (EE_TRUE)
	{
		do { i += array->elem_size; } 
		while (cmp(&array->buffer[i], pivot) < 0);

		do { j -= array->elem_size; } 
		while (cmp(&array->buffer[j], pivot) > 0);

		if (i >= j)
		{ break; }

		memcpy(temp, &array->buffer[i], array->elem_size);
		memcpy(&array->buffer[i], &array->buffer[j], array->elem_size);
		memcpy(&array->buffer[j], temp, array->elem_size);
	}

	EE_FREEA(temp);
	EE_FREEA(pivot);

	ee_array_quicksort(array, cmp, low, j);
	ee_array_quicksort(array, cmp, j + array->elem_size, high);
}

EE_INLINE void ee_array_heapsort(Array* array, BinCmp cmp, s64 low, s64 high)
{
	EE_ASSERT(array != NULL, "Trying to sort a NULL Array");
	EE_ASSERT(cmp != NULL, "Trying to sort with a NULL BinCmp");

	if (ee_array_empty(array))
		return;

	if (low < 0 || high < 0 || low >= high)
		return;

	u8* temp = (u8*)EE_ALLOCA(array->elem_size);
	EE_ASSERT(temp != NULL, "Unable to allocate (%zu) for sorting temporary buffer", array->elem_size);

	s64 count = (high - low) / array->elem_size + 1;
	s64 start = (count >> 1) * array->elem_size;
	s64 end = count * array->elem_size;

	while ((size_t)end > array->elem_size)
	{
		if (start > 0)
		{
			start -= array->elem_size;
		}
		else
		{
			end -= array->elem_size;

			memcpy(temp, &array->buffer[low], array->elem_size);
			memcpy(&array->buffer[low], &array->buffer[low + end], array->elem_size);
			memcpy(&array->buffer[low + end], temp, array->elem_size);
		}

		s64 root = start;

		while ((2 * root + array->elem_size) < (size_t)end)
		{
			s64 child = 2 * root + array->elem_size;

			if ((child + array->elem_size < (size_t)end) && cmp(&array->buffer[low + child], &array->buffer[low + child + array->elem_size]) < 0)
			{
				child += array->elem_size;
			}

			if (cmp(&array->buffer[low + root], &array->buffer[low + child]) < 0)
			{
				memcpy(temp, &array->buffer[low + root], array->elem_size);
				memcpy(&array->buffer[low + root], &array->buffer[low + child], array->elem_size);
				memcpy(&array->buffer[low + child], temp, array->elem_size);
				
				root = child;
			}
			else
			{
				break;
			}
		}
	}

	EE_FREEA(temp);
}

EE_INLINE void ee_array_introsort(Array* array, BinCmp cmp, s64 low, s64 high, s32 max_depth)
{
	EE_ASSERT(array != NULL, "Trying to sort a NULL Array");
	EE_ASSERT(cmp != NULL, "Trying to sort with a NULL BinCmp");

	if (ee_array_empty(array))
		return;

	if (low < 0 || high < 0 || low >= high)
		return;

	s64 len = (high - low) / array->elem_size + 1;

	if (len <= EE_ARRAY_SORT_TH)
	{
		ee_array_insertsort(array, cmp, low, high);
	}
	else if (max_depth <= 0)
	{
		ee_array_heapsort(array, cmp, low, high);
	}
	else
	{
		u8* temp = (u8*)EE_ALLOCA(array->elem_size);
		u8* pivot = (u8*)EE_ALLOCA(array->elem_size);
		
		EE_ASSERT(temp != NULL, "Unable to allocate (%zu) on stack", array->elem_size);
		EE_ASSERT(pivot != NULL, "Unable to allocate (%zu) on stack", array->elem_size);

		size_t low_index = low / array->elem_size;
		size_t high_index = high / array->elem_size;
		size_t mid_index = (low_index + high_index) >> 1;

		memcpy(pivot, &array->buffer[mid_index * array->elem_size], array->elem_size);

		s64 i = low - array->elem_size;
		s64 j = high + array->elem_size;

		while (EE_TRUE)
		{
			do { i += array->elem_size; } 
			while (cmp(&array->buffer[i], pivot) < 0);

			do { j -= array->elem_size; } 
			while (cmp(&array->buffer[j], pivot) > 0);

			if (i >= j)
			{ break; }

			memcpy(temp, &array->buffer[i], array->elem_size);
			memcpy(&array->buffer[i], &array->buffer[j], array->elem_size);
			memcpy(&array->buffer[j], temp, array->elem_size);
		}

		EE_FREEA(temp);
		EE_FREEA(pivot);

		ee_array_introsort(array, cmp, low, j, max_depth - 1);
		ee_array_introsort(array, cmp, j + array->elem_size, high, max_depth - 1);
	}
}

EE_INLINE void ee_array_sort(Array* array, BinCmp cmp, ArraySortType type)
{
	switch (type)
	{
	case EE_SORT_INSERT:
		{
		ee_array_insertsort(array, cmp, 0, ee_array_size(array) - array->elem_size);
		} break;
	case EE_SORT_QUICK:
		{
		ee_array_quicksort(array, cmp, 0, ee_array_size(array) - array->elem_size);
		} break;
	case EE_SORT_HEAP:
		{
		ee_array_heapsort(array, cmp, 0, ee_array_size(array) - array->elem_size);
		} break;
	case EE_SORT_DEFAULT:
	case EE_SORT_INTRO:
		{
		u32 len = (u32)ee_array_len(array); // TODO(eesuck): log2 for numbers greater than max u32
		s32 max_depth = ee_array_log2_u32(len) * 2;

		ee_array_introsort(array, cmp, 0, (len - 1) * array->elem_size, max_depth);
		} break;
	default:
	{
		EE_ASSERT(0, "Unknown arraytor sort type (%d)", type);
		return;
	}
	}
}

EE_INLINE void ee_array_fill(Array* array, u8* val, size_t a, size_t b)
{
	EE_ASSERT(array != NULL, "Trying to fill a NULL Array");
	EE_ASSERT(val != NULL, "Trying to fill a Array with a NULL value");
	EE_ASSERT((a < b) && (a * array->elem_size < array->cap) && (b * array->elem_size <= array->cap), 
		"Incorrect fill bounds (%zu):(%zu) for arraytor with len (%zu)", a, b, ee_array_len(array));

	size_t low  = a * array->elem_size;
	size_t high = b * array->elem_size;

	for (size_t i = low; i < high; i += array->elem_size)
	{
		memcpy(&array->buffer[i], val, array->elem_size);
	}

	if (high > array->top)
	{
		array->top = high;
	}
}

EE_INLINE Array ee_array_copy(Array* array, Allocator* allocator)
{
	EE_ASSERT(array != NULL, "Trying to copy into NULL Array");
	EE_ASSERT(array->buffer != NULL, "Trying to copy into NULL Array.buffer");

	Array out = *array;

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

	out.buffer = (u8*)out.allocator.alloc_fn(&out.allocator, out.cap);

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Array.buffer copy", out.cap);

	memcpy(out.buffer, array->buffer, out.cap);

	return out;
}

EE_INLINE void ee_array_reverse(Array* array)
{
	EE_ASSERT(array != NULL, "Trying to reverse NULL Array");

	u8* temp = (u8*)EE_ALLOCA(array->elem_size);
	EE_ASSERT(temp != NULL, "Unable to allocate (%zu) on stack", array->elem_size);

	size_t len = ee_array_len(array);

	for (size_t i = 0; i < (len >> 1); ++i)
	{
		size_t j = len - i - 1;

		memcpy(temp, ee_array_at(array, i), array->elem_size);
		memcpy(ee_array_at(array, i), ee_array_at(array, j), array->elem_size);
		memcpy(ee_array_at(array, j), temp, array->elem_size);
	}

	EE_FREEA(temp);
}

EE_INLINE void ee_array_swap_n_pop(Array* array, size_t i, u8* out_val)
{
	EE_ASSERT(array != NULL, "Trying to pop from NULL array");
	EE_ASSERT(!ee_array_empty(array), "Trying to pop from empty array");
	EE_ASSERT(i < ee_array_len(array), "Invalid swap and pop index (%zu) for array with len (%zu)", i, ee_array_len(array));

	size_t len = ee_array_len(array);

	if (len == 1)
	{
		ee_array_pop(array, out_val);
		return;
	}

	if (out_val != NULL)
	{
		memcpy(out_val, array->buffer[i * array->elem_size], array->elem_size);
	}

	array->top -= array->elem_size;

	u8* temp = (u8*)EE_ALLOCA(array->elem_size);
	EE_ASSERT(temp != NULL, "Unable to allocate (%zu) on stack", array->elem_size);

	memcpy(temp, &array->buffer[i * array->elem_size], array->elem_size);
	memcpy(&array->buffer[i * array->elem_size], &array->buffer[array->top], array->elem_size);
	memcpy(&array->buffer[array->top], temp, array->elem_size);
	
	EE_FREEA(temp);
}

#endif // EE_ARRAY_H
