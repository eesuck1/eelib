#pragma once

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

#define EE_START_STR_SIZE    (16)

typedef uint8_t str_dt;

typedef struct Str
{
	size_t len;
	size_t cap;
	str_dt* buffer;
} Str;

typedef struct LongStr
{
	int len;
	str_dt prefix[EE_LS_PREFIX_LEN];
	str_dt* buffer;
} LongStr;

typedef struct ShortStr
{
	int len;
	str_dt buffer[EE_SS_LEN];
} ShortStr;

typedef struct StrView
{
	size_t len;
	const str_dt* buffer;
} StrView;

EE_INLINE uint64_t ee_str_next_pow_2(uint64_t x)
{
	if (x == 0)
	{
		return 1;
	}

	x--;

	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x |= x >> 32;

	x++;

	return x;
}

EE_INLINE ShortStr ee_short_str_new(const str_dt* buffer, int32_t len)
{
	EE_ASSERT(len <= EE_SS_LEN, "Given buffer length (%d) should not be grater than (%d)", len, EE_SS_LEN);

	ShortStr out = { 0 };

	memcpy(out.buffer, buffer, len);
	out.len = len;
	
	return out;
}

EE_INLINE int ee_short_str_cmp(ShortStr first, ShortStr second)
{
	EE_ASSERT(EE_SS_LEN == 16, "String comparator expects 16-byte short strings, (%d) given", EE_SS_LEN);

	const uint64_t* f_data = (uint64_t*)first.buffer;
	const uint64_t* s_data = (uint64_t*)second.buffer;

	return (first.len == second.len) && !((f_data[0] ^ s_data[0]) && (f_data[1] ^ s_data[1]));
}

EE_INLINE Str ee_str_new()
{
	Str out = { 0 };

	out.len = 0;
	out.cap = EE_START_STR_SIZE;
	out.buffer = (str_dt*)malloc(sizeof(str_dt) * EE_START_STR_SIZE);

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%d) bytes for Str.buffer", EE_START_STR_SIZE);

	return out;
}

EE_INLINE Str ee_str_from(const char* cstr)
{
	Str out = { 0 };

	size_t len = strlen(cstr);

	out.len = len;
	out.cap = ee_str_next_pow_2(len + 1);
	out.buffer = (str_dt*)malloc(sizeof(str_dt) * out.cap);

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Str.buffer", sizeof(str_dt) * out.cap);

	if (out.buffer != NULL)
	{
		memcpy(out.buffer, cstr, len + 1);
	}

	return out;
}

EE_INLINE void ee_str_free(Str* str)
{
	EE_ASSERT(str != NULL, "Unable to free NULL Str pointer");
	EE_ASSERT(str->buffer != NULL, "Unable to free NULL Str.buffer pointer");

	if (str == NULL || str->buffer == NULL)
	{
		return;
	}

	free(str->buffer);
}

EE_INLINE Str ee_str_copy(const Str* src)
{
	EE_ASSERT(src != NULL, "NULL Str pointer");
	EE_ASSERT(src->buffer != NULL, "NULL Str.buffer pointer");

	if (src == NULL || src->buffer == NULL)
	{
		return;
	}

	Str out = { 0 };

	out.len = src->len;
	out.cap = src->cap;
	out.buffer = (str_dt*)malloc(sizeof(str_dt) * src->len);
	
	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Str.buffer", sizeof(str_dt) * src->len);

	if (out.buffer == NULL)
	{
		return out;
	}

	memcpy(out.buffer, src->buffer, out.len);

	return out;
}

EE_INLINE void ee_str_assign(Str* dest, const Str* src)
{
	EE_ASSERT(dest != NULL, "NULL dest Str pointer");
	EE_ASSERT(dest->buffer != NULL, "NULL dest Str.buffer pointer");
	EE_ASSERT(src != NULL, "NULL src Str pointer");
	EE_ASSERT(src->buffer != NULL, "NULL src Str.buffer pointer");

	if (src == NULL || src->buffer == NULL || dest == NULL || dest->buffer == NULL)
	{
		return;
	}

	if (src->len >= dest->cap)
	{
		dest->cap = src->cap;
		str_dt* new_buffer = (str_dt*)realloc(dest->buffer, sizeof(str_dt) * dest->cap);

		EE_ASSERT(new_buffer != NULL, "Unable to reallocate (%zu) bytes for Str.buffer", sizeof(str_dt) * dest->cap);

		if (new_buffer == NULL)
		{
			return;
		}

		dest->buffer = new_buffer;
	}

	memcpy(dest->buffer, src->buffer, src->len + 1);
}

EE_INLINE char* ee_str_cstr(const Str* str)
{
	EE_ASSERT(str != NULL, "NULL Str pointer");
	EE_ASSERT(str->buffer != NULL, "NULL Str.buffer pointer");

	if (str == NULL || str->buffer == NULL)
	{
		return NULL;
	}

	char* out = (char*)malloc(sizeof(str_dt) * (str->len + 1));

	EE_ASSERT(out != NULL, "Unable to allocate (%zu) bytes for out buffer", sizeof(str_dt) * (str->len + 1));

	if (out == NULL)
	{
		return NULL;
	}

	memcpy(out, str->buffer, str->len);
	out[str->len] = '\0';

	return out;
}

#endif // EE_STRING_H
