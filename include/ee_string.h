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
#define EE_TRUE             (1)
#define EE_FALSE            (0)

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

#endif // EE_STRING_H
