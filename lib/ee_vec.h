#pragma once

#ifndef EE_VEC_H
#define EE_VEC_H

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

#define EE_VEC_DT(x)      ((uint8_t*)(&(x)))
#define EE_VEC_INVALID    (0xffffffffffffffffull)
#define EE_VEC_SORT_TH    (16)

typedef struct Vec
{
	size_t top;
	size_t cap;
	size_t elem_size;
	uint8_t* buffer;
} Vec;

typedef int (*VecCmp)(const void* a, const void* b);

EE_INLINE int ee_is_pow2(uint64_t x)
{
	return (x != 0) && ((x & (x - 1)) == 0);
}

#if defined(_MSC_VER)
	#include "intrin.h"
#endif

EE_INLINE int32_t ee_vec_first_bit_u32(uint32_t x)
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

EE_INLINE Vec ee_vec_new(size_t size, size_t elem_size)
{
	EE_ASSERT(size > 0, "Invalid vector size (%zu)", size);
	EE_ASSERT(elem_size > 0, "Invalid vector elem_size (%zu)", elem_size);

	Vec out = { 0 };

	out.top = 0;
	out.cap = elem_size * size;
	out.elem_size = elem_size;
	out.buffer = (uint8_t*)malloc(elem_size * size);

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Vec.buffer", elem_size * size);
	
	return out;
}

EE_INLINE int ee_vec_full(Vec* vec)
{
	EE_ASSERT(vec != NULL, "Trying to check NULL Vec");

	return vec->top >= vec->cap;
}

EE_INLINE int ee_vec_empty(Vec* vec)
{
	return vec->top < vec->elem_size;
}

EE_INLINE size_t ee_vec_len(Vec* vec)
{
	return vec->top / vec->elem_size;
}

EE_INLINE size_t ee_vec_size(Vec* vec)
{
	return vec->top;
}

EE_INLINE void ee_vec_free(Vec* vec)
{
	EE_ASSERT(vec != NULL, "Trying to free NULL Vec");
	EE_ASSERT(vec->buffer != NULL, "Trying to free NULL Vec.buffer");

	free(vec->buffer);

	memset(vec, 0, sizeof(Vec));
}

EE_INLINE void ee_vec_clear(Vec* vec)
{
	EE_ASSERT(vec != NULL, "Trying to clear NULL Vec");
	EE_ASSERT(vec->buffer != NULL, "Trying to clear NULL Vec.buffer");

	vec->top = 0;
}

EE_INLINE void ee_vec_reserve(Vec* vec, size_t size)
{
	EE_ASSERT(vec != NULL, "Trying to grow NULL Vec");
	EE_ASSERT(vec->buffer != NULL, "Trying to reallocate NULL Vec.buffer");
	EE_ASSERT(size * vec->elem_size > vec->cap, "Reserve expects Vec to grow, given size (%zu) current capacity (%zu)", size, vec->cap / vec->elem_size);

	vec->cap = size * vec->elem_size;

	uint8_t* new_buffer = (uint8_t*)realloc(vec->buffer, vec->cap);

	EE_ASSERT(new_buffer != NULL, "Unable to reallocate (%zu) bytes for Vec.buffer", vec->cap);

	vec->buffer = new_buffer;
}

EE_INLINE void ee_vec_grow(Vec* vec)
{
	EE_ASSERT(vec != NULL, "Trying to grow NULL Vec");
	EE_ASSERT(vec->buffer != NULL, "Trying to reallocate NULL Vec.buffer");

	vec->cap = vec->cap + (vec->cap >> 1);

	uint8_t* new_buffer = (uint8_t*)realloc(vec->buffer, vec->cap * vec->elem_size);

	EE_ASSERT(new_buffer != NULL, "Unable to reallocate (%zu) bytes for Vec.buffer", vec->cap * vec->elem_size);

	vec->buffer = new_buffer;
}

EE_INLINE void ee_vec_push(Vec* vec, uint8_t* val)
{
	EE_ASSERT(vec != NULL, "Trying to push into NULL Vec");
	EE_ASSERT(val != NULL, "Trying to push NULL value");

	if (ee_vec_full(vec))
	{
		ee_vec_grow(vec);
	}

	memcpy(&vec->buffer[vec->top], val, vec->elem_size);

	vec->top += vec->elem_size;
}

EE_INLINE uint8_t* ee_vec_top(Vec* vec)
{
	EE_ASSERT(vec != NULL, "Trying to get NULL Vec top");
	EE_ASSERT(vec->top >= vec->elem_size, "Trying to get top element of empty Vec");

	return &vec->buffer[vec->top - vec->elem_size];
}

EE_INLINE uint8_t* ee_vec_at(Vec* vec, size_t i)
{
	EE_ASSERT(vec != NULL, "Trying to get NULL Vec element");
	EE_ASSERT(i * vec->elem_size < vec->top, "Index (%zu) is out of bounds for vector with top (%zu)", i, vec->top / vec->elem_size);

	return &vec->buffer[i * vec->elem_size];
}

EE_INLINE void ee_vec_pop(Vec* vec)
{
	EE_ASSERT(vec != NULL, "Trying to pop NULL Vec");
	EE_ASSERT(vec->top >= vec->elem_size, "Trying to pop empty Vec");

	vec->top -= vec->elem_size;
}

EE_INLINE void ee_vec_set(Vec* vec, size_t i, uint8_t* val)
{
	EE_ASSERT(vec != NULL, "Trying to set into NULL Vec");
	EE_ASSERT(vec->top >= i * vec->elem_size, "Invalid setting index (%zu) Vec.top at position (%zu)", i, vec->top / vec->elem_size);

	memcpy(&vec->buffer[i * vec->elem_size], val, vec->elem_size);
}

EE_INLINE size_t ee_vec_find(Vec* vec, uint8_t* target)
{
	EE_ASSERT(vec != NULL, "Trying to find in NULL Vec");
	EE_ASSERT(target != NULL, "Trying to find a NULL value");

	for (size_t i = 0; i < vec->top; i += vec->elem_size)
	{
		if (memcmp(target, &vec->buffer[i], vec->elem_size) == 0)
		{
			return i / vec->elem_size;
		}
	}

	return EE_VEC_INVALID;
}

EE_INLINE void ee_vec_insert(Vec* vec, size_t i, uint8_t* val)
{
	EE_ASSERT(vec != NULL, "Trying to insert into NULL Vec");
	EE_ASSERT(i <= vec->top / vec->elem_size, "Index out of bounds");

	if (ee_vec_full(vec))
	{
		ee_vec_grow(vec);
	}

	size_t offset = i * vec->elem_size;

	memmove(&vec->buffer[offset + vec->elem_size], &vec->buffer[offset], vec->top - offset);
	memcpy(&vec->buffer[i * vec->elem_size], val, vec->elem_size);

	vec->top += vec->elem_size;
}

EE_INLINE void ee_vec_erase(Vec* vec, size_t i)
{
	EE_ASSERT(vec != NULL, "Trying to erase from NULL Vec");
	EE_ASSERT(i < vec->top / vec->elem_size, "Index out of bounds");

	size_t offset = i * vec->elem_size;

	memmove(&vec->buffer[offset], &vec->buffer[offset + vec->elem_size], vec->top - offset - vec->elem_size);

	vec->top -= vec->elem_size;
}

EE_INLINE void ee_vec_swap(Vec* vec, size_t i, size_t j, uint8_t* temp)
{
	EE_ASSERT(vec != NULL, "Trying to swap an elements in NULL Vec");
	EE_ASSERT(ee_vec_size(vec) >= 2, "Trying to swap Vec with number of elements (%zu), minimum is 2", ee_vec_size(vec));

	size_t alloc = EE_FALSE;

	if (temp == NULL)
	{
		if (ee_vec_full(vec))
		{
			temp = (uint8_t*)malloc(vec->elem_size * sizeof(uint8_t));
			alloc = EE_TRUE;
		}
		else
		{
			temp = &vec->buffer[vec->top];
		}
	}

	memcpy(temp, ee_vec_at(vec, i), vec->elem_size);
	memcpy(ee_vec_at(vec, i), ee_vec_at(vec, j), vec->elem_size);
	memcpy(ee_vec_at(vec, j), temp, vec->elem_size);

	if (alloc)
	{
		free(temp);
	}
}

EE_INLINE void ee_vec_insertsort(Vec* vec, VecCmp cmp, size_t low, size_t high, uint8_t* temp)
{
	EE_ASSERT(vec != NULL, "Trying to sort a NULL Vec");
	EE_ASSERT(cmp != NULL, "Trying to sort with a NULL VecCmp");

	if (ee_vec_empty(vec))
	{
		return;
	}

	size_t alloc = EE_FALSE;

	if (temp == NULL)
	{
		if (ee_vec_full(vec))
		{
			temp = (uint8_t*)malloc(vec->elem_size * sizeof(uint8_t));
			alloc = EE_TRUE;
		}
		else
		{
			temp = &vec->buffer[vec->top];
		}
	}

	EE_ASSERT(temp != NULL, "Unable to allocate (%zu) for sorting temporary buffer", vec->elem_size * sizeof(uint8_t));

	for (size_t i = low; i < high; ++i)
	{
		memcpy(temp, ee_vec_at(vec, i), vec->elem_size);

		size_t j = i;

		while (j > 0 && cmp(ee_vec_at(vec, j - 1), temp) > 0)
		{
			ee_vec_set(vec, j, ee_vec_at(vec, j - 1));
			j--;
		}

		ee_vec_set(vec, j, temp);
	}

	if (alloc)
	{
		free(temp);
	}
}

EE_INLINE void ee_vec_quicksort(Vec* vec, VecCmp cmp, int64_t low, int64_t high, uint8_t* temp)
{
	EE_ASSERT(vec != NULL, "Trying to sort a NULL Vec");
	EE_ASSERT(cmp != NULL, "Trying to sort with a NULL VecCmp");

	if (ee_vec_empty(vec))
		return;

	if (low < 0 || high < 0 || low >= high)
		return;

	size_t alloc = EE_FALSE;

	if (temp == NULL)
	{
		if (ee_vec_full(vec))
		{
			temp = (uint8_t*)malloc(vec->elem_size * sizeof(uint8_t));
			alloc = EE_TRUE;
		}
		else
		{
			temp = &vec->buffer[vec->top];
		}
	}

	EE_ASSERT(temp != NULL, "Unable to allocate (%zu) for sorting temporary buffer", vec->elem_size * sizeof(uint8_t));

	uint8_t* pivot = ee_vec_at(vec, low);

	int64_t i = low - 1;
	int64_t j = high + 1;

	while (EE_TRUE)
	{
		do { i++; } 
		while (cmp(ee_vec_at(vec, i), pivot) < 0);

		do { j--; }
		while (cmp(ee_vec_at(vec, j), pivot) > 0);

		if (i >= j) { break; }

		ee_vec_swap(vec, i, j, temp);
	}

	ee_vec_quicksort(vec, cmp, low, j, temp);
	ee_vec_quicksort(vec, cmp, j + 1, high, temp);

	if (alloc)
	{
		free(temp);
	}
}

EE_INLINE void ee_vec_introsort(Vec* vec, VecCmp cmp)
{
}

#endif // EE_VEC_H
