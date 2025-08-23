#pragma once

#ifndef EE_DICT_H
#define EE_DICT_H

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

#ifndef EE_TRUE
#define EE_TRUE            (1)
#endif // EE_TRUE

#ifndef EE_FALSE
#define EE_FALSE           (0)
#endif // EE_FALSE

typedef struct DictKey
{
	uint8_t bytes[EE_KEY_SIZE];
} DictKey;

typedef struct DictValue
{
	uint8_t bytes[EE_VALUE_SIZE];
} DictValue;

typedef struct Dict
{
	DictKey* keys;
    DictValue* vals;
    int* ocup;

	size_t count;
	size_t cap;
    size_t mask;
    size_t threshold;
} Dict;

EE_INLINE int ee_is_pow_2(uint64_t x)
{
    return x && !(x & (x - 1));
}

EE_INLINE uint64_t ee_next_pow_2(uint64_t x)
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

EE_INLINE uint64_t ee_scale_0p75(uint64_t x)
{
    return (x * 3) >> 2;
}

EE_INLINE void ee_alloc_kvo(DictKey** keys, DictValue** vals, int** ocup, size_t cap)
{
    uint8_t* memory = (uint8_t*)calloc(cap, (sizeof(DictKey) + sizeof(DictValue) + sizeof(int)));

    EE_ASSERT(memory != NULL, "Unable to allocate %zu bytes for buffer", (sizeof(DictKey) + sizeof(DictValue) + sizeof(int)) * cap);

    if (memory == NULL)
    {
        *keys = NULL;
        *vals = NULL;
        *ocup = NULL;

        return;
    }

    *keys = (DictKey*)memory;
    *vals = (DictValue*)(memory + sizeof(DictKey) * cap);
    *ocup = (int*)(memory + (sizeof(DictKey) + sizeof(DictValue)) * cap);
}

EE_INLINE void ee_free_kvo(Dict* dict)
{
    EE_ASSERT(dict->keys != NULL, "Base of buffer is NULL");

    if (dict->keys == NULL)
    {
        return;
    }

    free(dict->keys);
}

EE_INLINE Dict ee_dict_new(size_t cap)
{
    Dict out = { 0 };

    out.count = 0;

    if (!ee_is_pow_2(cap))
    {
        cap = ee_next_pow_2(cap);
    }

    out.cap = cap;
    out.mask = cap - 1;
    out.threshold = ee_scale_0p75(cap);

    ee_alloc_kvo(&out.keys, &out.vals, &out.ocup, cap);

    return out;
}

EE_INLINE uint64_t ee_hash(DictKey key, uint64_t seed)
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

EE_INLINE int ee_key_eq(DictKey first, DictKey second)
{
    return memcmp(first.bytes, second.bytes, EE_KEY_SIZE) == 0;
}

EE_INLINE int ee_val_eq(DictValue first, DictValue second)
{
    return memcmp(first.bytes, second.bytes, EE_VALUE_SIZE) == 0;
}

EE_INLINE void ee_dict_insert(Dict* dict, DictKey key, DictValue val)
{
    EE_ASSERT(dict != NULL, "Dict pointer is NULL");

    if (dict == NULL)
    {
        return;
    }

    uint64_t h = ee_hash(key, EE_DEFAULT_SEED);
    uint64_t h_i = h & dict->mask;

    for (size_t i = 0; i < dict->cap; ++i)
    {
        if (!dict->ocup[h_i])
        {
            dict->keys[h_i] = key;
            dict->vals[h_i] = val;
            dict->ocup[h_i] = EE_TRUE;

            dict->count++;
            
            break;
        }

        if (ee_key_eq(key, dict->keys[h_i]))
        {
            dict->vals[h_i] = val;
            
            break;
        }

        h_i = (h_i + 1) & dict->mask;
    }
}

EE_INLINE void ee_dict_expand(Dict* dict)
{
    Dict new_dict = ee_dict_new(dict->cap * 2);

    for (size_t i = 0; i < dict->cap; ++i)
    {
        if (!dict->ocup[i])
        {
            continue;
        }

        ee_dict_insert(&new_dict, dict->keys[i], dict->vals[i]);
    }

    ee_free_kvo(dict);

    *dict = new_dict;
}

EE_INLINE void ee_dict_add(Dict* dict, DictKey key, DictValue val)
{
    ee_dict_insert(dict, key, val);

    if (dict->count > dict->threshold)
    {
        ee_dict_expand(dict);
    }
}

EE_INLINE DictValue ee_dict_get(Dict* dict, DictKey key)
{
    EE_ASSERT(dict != NULL, "Dict pointer is NULL");

    if (dict == NULL)
    {
        return;
    }

    uint64_t h = ee_hash(key, EE_DEFAULT_SEED);
    uint64_t h_i = h & dict->mask;

    for (size_t i = 0; i < dict->cap; ++i)
    {
        EE_ASSERT(dict->ocup[h_i], "Invalid key");

        if (ee_key_eq(key, dict->keys[h_i]))
        {
            return dict->vals[h_i];
        }

        h_i = (h_i + 1) & dict->mask;
    }

    EE_ASSERT(0, "Invalid key");
    DictValue null = { 0 };
    
    return null;
}

EE_INLINE DictKey ee_key_cstr(const char* str)
{
    DictKey out = { 0 };

    for (int i = 0; i < str[i] != '\0' && i < EE_KEY_SIZE; ++i)
    {
        out.bytes[i] = (uint8_t)str[i];
    }

    return out;
}

EE_INLINE DictKey ee_key_str_view(const char* str, int len)
{
    EE_ASSERT(len <= EE_KEY_SIZE, "Given buffer length (%d) should not be grater than (%d)", len, EE_KEY_SIZE);

    DictKey out = { 0 };

    if (len <= EE_KEY_SIZE)
    {
        memcpy(out.bytes, str, len);
    }
    else
    {
        memcpy(out.bytes, str, EE_KEY_SIZE);
    }

    return out;
}

EE_INLINE DictKey ee_key_from_4s32(int32_t x0, int32_t x1, int32_t x2, int32_t x3)
{
    DictKey out = { 0 };

    memcpy(out.bytes, &x0, sizeof(int32_t));
    memcpy(out.bytes + sizeof(int32_t), &x1, sizeof(int32_t));
    memcpy(out.bytes + 2 * sizeof(int32_t), &x2, sizeof(int32_t));
    memcpy(out.bytes + 3 * sizeof(int32_t), &x3, sizeof(int32_t));

    return out;
}

EE_INLINE void ee_key_to_4s32(DictKey key, int32_t* x0, int32_t* x1, int32_t* x2, int32_t* x3)
{
    memcpy(x0, key.bytes, sizeof(int32_t));
    memcpy(x1, key.bytes + sizeof(int32_t), sizeof(int32_t));
    memcpy(x2, key.bytes + 2 * sizeof(int32_t), sizeof(int32_t));
    memcpy(x3, key.bytes + 3 * sizeof(int32_t), sizeof(int32_t));
}

EE_INLINE DictKey ee_key_from_4u32(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3)
{
    DictKey out = { 0 };

    memcpy(out.bytes, &x0, sizeof(uint32_t));
    memcpy(out.bytes + sizeof(uint32_t), &x1, sizeof(uint32_t));
    memcpy(out.bytes + 2 * sizeof(uint32_t), &x2, sizeof(uint32_t));
    memcpy(out.bytes + 3 * sizeof(uint32_t), &x3, sizeof(uint32_t));

    return out;
}

EE_INLINE void ee_key_to_4u32(DictKey key, uint32_t* x0, uint32_t* x1, uint32_t* x2, uint32_t* x3)
{
    memcpy(x0, key.bytes, sizeof(uint32_t));
    memcpy(x1, key.bytes + sizeof(uint32_t), sizeof(uint32_t));
    memcpy(x2, key.bytes + 2 * sizeof(uint32_t), sizeof(uint32_t));
    memcpy(x3, key.bytes + 3 * sizeof(uint32_t), sizeof(uint32_t));
}

EE_INLINE DictKey ee_key_from_2s64(int64_t x0, int64_t x1)
{
    DictKey out = { 0 };

    memcpy(out.bytes, &x0, sizeof(int64_t));
    memcpy(out.bytes + sizeof(int64_t), &x1, sizeof(int64_t));

    return out;
}

EE_INLINE void ee_key_to_2s64(DictKey key, int64_t* x0, int64_t* x1)
{
    memcpy(x0, key.bytes, sizeof(int64_t));
    memcpy(x1, key.bytes + sizeof(int64_t), sizeof(int64_t));
}

EE_INLINE DictKey ee_key_from_2u64(uint64_t x0, uint64_t x1)
{
    DictKey out = { 0 };

    memcpy(out.bytes, &x0, sizeof(uint64_t));
    memcpy(out.bytes + sizeof(uint64_t), &x1, sizeof(uint64_t));

    return out;
}

EE_INLINE void ee_key_to_2u64(DictKey key, uint64_t* x0, uint64_t* x1)
{
    memcpy(x0, key.bytes, sizeof(uint64_t));
    memcpy(x1, key.bytes + sizeof(uint64_t), sizeof(uint64_t));
}

EE_INLINE DictValue ee_value_from_3s64(int64_t x0, int64_t x1, int64_t x2)
{
    DictValue out = { 0 };

    memcpy(out.bytes, &x0, sizeof(int64_t));
    memcpy(out.bytes + sizeof(int64_t), &x1, sizeof(int64_t));
    memcpy(out.bytes + 2 * sizeof(int64_t), &x2, sizeof(int64_t));

    return out;
}

EE_INLINE void ee_value_to_3s64(DictValue value, int64_t* x0, int64_t* x1, int64_t* x2)
{
    memcpy(x0, value.bytes, sizeof(int64_t));
    memcpy(x1, value.bytes + sizeof(int64_t), sizeof(int64_t));
    memcpy(x2, value.bytes + 2 * sizeof(int64_t), sizeof(int64_t));
}

EE_INLINE DictValue ee_value_from_3u64(uint64_t x0, uint64_t x1, uint64_t x2)
{
    DictValue out = { 0 };

    memcpy(out.bytes, &x0, sizeof(uint64_t));
    memcpy(out.bytes + sizeof(uint64_t), &x1, sizeof(uint64_t));
    memcpy(out.bytes + 2 * sizeof(uint64_t), &x2, sizeof(uint64_t));

    return out;
}

EE_INLINE void ee_value_to_3u64(DictValue value, uint64_t* x0, uint64_t* x1, uint64_t* x2)
{
    memcpy(x0, value.bytes, sizeof(uint64_t));
    memcpy(x1, value.bytes + sizeof(uint64_t), sizeof(uint64_t));
    memcpy(x2, value.bytes + 2 * sizeof(uint64_t), sizeof(uint64_t));
}

EE_INLINE DictValue ee_value_from_6s32(int32_t x0, int32_t x1, int32_t x2, int32_t x3, int32_t x4, int32_t x5)
{
    DictValue out = { 0 };

    memcpy(out.bytes, &x0, sizeof(int32_t));
    memcpy(out.bytes + sizeof(int32_t), &x1, sizeof(int32_t));
    memcpy(out.bytes + 2 * sizeof(int32_t), &x2, sizeof(int32_t));
    memcpy(out.bytes + 3 * sizeof(int32_t), &x3, sizeof(int32_t));
    memcpy(out.bytes + 4 * sizeof(int32_t), &x4, sizeof(int32_t));
    memcpy(out.bytes + 5 * sizeof(int32_t), &x5, sizeof(int32_t));

    return out;
}

EE_INLINE void ee_value_to_6s32(DictValue value, int32_t* x0, int32_t* x1, int32_t* x2, int32_t* x3, int32_t* x4, int32_t* x5)
{
    memcpy(x0, value.bytes, sizeof(int32_t));
    memcpy(x1, value.bytes + sizeof(int32_t), sizeof(int32_t));
    memcpy(x2, value.bytes + 2 * sizeof(int32_t), sizeof(int32_t));
    memcpy(x3, value.bytes + 3 * sizeof(int32_t), sizeof(int32_t));
    memcpy(x4, value.bytes + 4 * sizeof(int32_t), sizeof(int32_t));
    memcpy(x5, value.bytes + 5 * sizeof(int32_t), sizeof(int32_t));
}

EE_INLINE DictValue ee_value_from_6u32(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t x4, uint32_t x5)
{
    DictValue out = { 0 };

    memcpy(out.bytes, &x0, sizeof(uint32_t));
    memcpy(out.bytes + sizeof(uint32_t), &x1, sizeof(uint32_t));
    memcpy(out.bytes + 2 * sizeof(uint32_t), &x2, sizeof(uint32_t));
    memcpy(out.bytes + 3 * sizeof(uint32_t), &x3, sizeof(uint32_t));
    memcpy(out.bytes + 4 * sizeof(uint32_t), &x4, sizeof(uint32_t));
    memcpy(out.bytes + 5 * sizeof(uint32_t), &x5, sizeof(uint32_t));

    return out;
}

EE_INLINE void ee_value_to_6u32(DictValue value, uint32_t* x0, uint32_t* x1, uint32_t* x2, uint32_t* x3, uint32_t* x4, uint32_t* x5)
{
    memcpy(x0, value.bytes, sizeof(uint32_t));
    memcpy(x1, value.bytes + sizeof(uint32_t), sizeof(uint32_t));
    memcpy(x2, value.bytes + 2 * sizeof(uint32_t), sizeof(uint32_t));
    memcpy(x3, value.bytes + 3 * sizeof(uint32_t), sizeof(uint32_t));
    memcpy(x4, value.bytes + 4 * sizeof(uint32_t), sizeof(uint32_t));
    memcpy(x5, value.bytes + 5 * sizeof(uint32_t), sizeof(uint32_t));
}

EE_INLINE DictValue ee_value_from_data(const void* data, size_t size)
{
    EE_ASSERT(size <= EE_VALUE_SIZE, "Data size (%zu) exceeds DictValue size (%d)", size, EE_VALUE_SIZE);

    DictValue out = { 0 };
    memcpy(out.bytes, data, size);
    return out;
}

EE_INLINE void ee_value_to_data(DictValue value, void* data, size_t size)
{
    EE_ASSERT(size <= EE_VALUE_SIZE, "Data size (%zu) exceeds DictValue size (%d)", size, EE_VALUE_SIZE);


    if (size <= EE_VALUE_SIZE)
    {
        memcpy(data, value.bytes, size);
    }
    else
    {
        memcpy(data, value.bytes, EE_VALUE_SIZE);
    }
}


#endif // EE_DICT_H
