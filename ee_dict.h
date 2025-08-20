#pragma once

#ifndef EE_DICT_H

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

#define EE_KEY_SIZE        (16)
#define EE_VALUE_SIZE      (24)
#define EE_DEFAULT_SEED    (263611987)

typedef struct Key
{
	uint8_t bytes[EE_KEY_SIZE];
} Key;

typedef struct Value
{
	uint8_t bytes[EE_VALUE_SIZE];
} Value;

typedef struct Dict
{
	Key* keys;
    Value* vals;
    int* ocup;

	size_t count;
	size_t cap;
} Dict;

EE_INLINE Dict ee_dict_new(size_t cap)
{
    Dict out = { 0 };

    out.keys = (Key*)malloc(sizeof(Key) * cap);
    out.vals = (Value*)malloc(sizeof(Value) * cap);
    out.ocup = (Value*)malloc(sizeof(int) * cap);

    out.count = 0;
    out.cap = cap;

    return out;
}

EE_INLINE uint64_t ee_hash(Key key, uint64_t seed)
{
    EE_ASSERT(EE_KEY_SIZE == 16, "Hash function expect 16-byte key, (%d) given", EE_KEY_SIZE);

    uint64_t k1 = ((uint64_t*)key.bytes)[0];
    uint64_t k2 = ((uint64_t*)key.bytes)[1];

    k1 *= 0xff51afd7ed558ccdULL;
    k1 = (k1 << 31) | (k1 >> 33);
    k1 *= 0xc4ceb9fe1a85ec53ULL;

    k2 *= 0xff51afd7ed558ccdULL;
    k2 = (k2 << 33) | (k2 >> 31);
    k2 *= 0xc4ceb9fe1a85ec53ULL;

    uint64_t h = seed ^ k1 ^ k2;

    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;

    return h;
}

EE_INLINE int ee_key_eq(Key first, Key second)
{
    return memcmp(first.bytes, second.bytes, sizeof(Key)) == 0;
}

EE_INLINE void ee_dict_insert(Dict* dict, Key key, Value val)
{
    uint64_t hash = ee_hash(key, EE_DEFAULT_SEED);
    uint64_t index = hash % dict->cap; // TODO(eesuck): replace slow % with fast multiplicative hashing


}


EE_INLINE Key ee_key_cstr(const char* str)
{
    Key out = { 0 };

    // TODO(eesuck): think about assertation when i >= key size
    for (int i = 0; i < str[i] != '\0' && i < EE_KEY_SIZE; ++i)
    {
        out.bytes[i] = (uint8_t)str[i];
    }

    return out;
}

EE_INLINE Key ee_key_str_view(const char* str, int len)
{
    EE_ASSERT(len <= EE_KEY_SIZE, "Given buffer length (%d) should not be grater than (%d)", len, EE_KEY_SIZE);

    Key out = { 0 };

    memcpy(out.bytes, str, len);

    return out;
}

#endif // EE_DICT_H

