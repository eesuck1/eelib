#ifndef EE_STRING_H
#define EE_STRING_H

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

Str ee_str_new(size_t size, Allocator* allocator)
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

void ee_str_free(Str* str)
{
	EE_ASSERT(str != NULL, "Trying to free NULL string");
	EE_ASSERT(str->buffer != NULL, "Trying to free NULL string buffer");

	str->allocator.free_fn(&str->allocator, str->buffer);

	memset(str, 0, sizeof(Str));
}

void ee_str_grow(Str* str)
{
	EE_ASSERT(str != NULL, "Trying to grow NULL string");
	EE_ASSERT(str->buffer != NULL, "Trying to grow NULL string buffer");

	size_t new_cap = str->cap + (str->cap >> 1);
	uint8_t* new_buffer = str->allocator.realloc_fn(&str->allocator, str->buffer, str->cap, new_cap);

	EE_ASSERT(new_buffer != NULL, "Unable to reallocate (%zu) bytes for Str.buffer");

	str->cap = new_cap;
	str->buffer = new_buffer;
}

int32_t ee_str_full(Str* str)
{
	EE_ASSERT(str != NULL, "Trying to check NULL string");

	return str->top >= str->cap;
}

int32_t ee_str_empty(Str* str)
{
	EE_ASSERT(str != NULL, "Trying to check NULL string");

	return str->top == 0;
}

void ee_str_push(Str* str, char symbol)
{
	EE_ASSERT(str != NULL, "Trying to push into NULL string");

	if (ee_str_full(str))
	{
		ee_str_grow(str);
	}

	str->buffer[str->top++] = symbol;
}

void ee_str_pop(Str* str, char* out_val)
{
	EE_ASSERT(str != NULL, "Trying to pop from NULL string");
	EE_ASSERT(!ee_str_empty(str), "Trying to pop from empty string");

	str->top--;

	if (out_val != NULL)
	{
		*out_val = str->buffer[str->top];
	}
}

int32_t ee_str_cmp(Str* a, Str* b)
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

#endif // EE_STRING_H
