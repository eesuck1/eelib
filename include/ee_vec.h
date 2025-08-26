#pragma once

#ifndef EE_VEC_H
#define EE_VEC_H

#include "stdlib.h"
#include "string.h"
#include "stdint.h"

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

#define EE_VEC_DT(x)    ((uint8_t*)(&(x)))

typedef struct Vec
{
	size_t top;
	size_t cap;
	size_t elem_size;
	uint8_t* buffer;
} Vec;

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

EE_INLINE size_t ee_vec_size(Vec* vec)
{
	return vec->top / vec->elem_size;
}

EE_INLINE size_t ee_vec_bsize(Vec* vec)
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
	EE_ASSERT(i * vec->elem_size < vec->top, "Trying to get top element of empty Vec");

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

#endif // EE_VEC_H
