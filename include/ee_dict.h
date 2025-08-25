#pragma once

#ifndef EE_DICT_H
#define EE_DICT_H

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
#else  // EE_NO_ASSERT

#define EE_ASSERT(cond, fmt, ...)    ((void)0)

#endif // EE_NO_ASSERT

#ifndef EE_INLINE
#define EE_INLINE    static inline
#endif // EE_INLINE

#define EE_VALUE_SIZE           (24)
#define EE_GROUP_SIZE           (16)
#define EE_DICT_START_SIZE      (32)

#define EE_SLOT_EMPTY           (0x80)
#define EE_SLOT_DELETED         (0xFE)
#define EE_GROUP_MASK           (~(EE_GROUP_SIZE - 1))

#ifndef EE_TRUE
#define EE_TRUE            (1)
#endif // EE_TRUE

#ifndef EE_FALSE
#define EE_FALSE           (0)
#endif // EE_FALSE

typedef uint64_t DictKey;

typedef struct DictValue
{
	uint8_t bytes[EE_VALUE_SIZE];
} DictValue;

typedef struct Slot
{
	DictKey key;
	DictValue val;
} Slot;

typedef struct Dict
{
	Slot* slots;
	uint8_t* ctrls;

	size_t count;
	size_t cap;
	size_t mask;
	size_t th;
} Dict;

EE_INLINE uint64_t ee_dict_th(uint64_t x)
{
	return (x * 896) >> 10;
}

EE_INLINE uint64_t ee_hash64(uint64_t key) 
{
	key ^= key >> 33;
	key *= 0xff51afd7ed558ccdULL;
	key ^= key >> 33;
	key *= 0xc4ceb9fe1a85ec53ULL;
	key ^= key >> 33;

	return key;
}

EE_INLINE uint64_t ee_next_pow_2(DictKey x)
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

EE_INLINE int32_t ee_first_bit_u32(uint32_t x)
{
	for (int32_t i = 0; i < 32; ++i)
	{
		if (x & (1 << i))
		{
			return i;
		}
	}

	return -1;
}

EE_INLINE Dict ee_dict_new(size_t size)
{
	Dict out = { 0 };

	EE_ASSERT(size >= EE_DICT_START_SIZE, "size value (%zu) for Dict should be greater or equal to (%d)", size, EE_DICT_START_SIZE);

	if (size < EE_DICT_START_SIZE)
	{
		return out;
	}

	size_t cap = ee_next_pow_2(size);

	out.slots = (Slot*)malloc(sizeof(Slot) * cap);
	out.ctrls = (uint8_t*)malloc(sizeof(uint8_t) * cap);

	out.count = 0;
	out.cap = cap;
	out.mask = out.cap - 1;
	out.th = ee_dict_th(out.cap);

	EE_ASSERT(out.slots != NULL, "Unable to allocate (%zu) bytes for Dict.slots", sizeof(Slot) * cap);
	EE_ASSERT(out.ctrls != NULL, "Unable to allocate (%zu) bytes for Dict.ctrls", sizeof(uint8_t) * cap);

	if (out.ctrls != NULL)
	{
		memset(out.ctrls, EE_SLOT_EMPTY, sizeof(uint8_t) * cap);
	}

	return out;
}

EE_INLINE void ee_dict_insert(Dict* dict, DictKey key, DictValue val)
{
	EE_ASSERT(dict != NULL, "Trying to insert to NULL Dict");
	EE_ASSERT(val.bytes != NULL, "Trying to insert NULL value to Dict");

	uint64_t hash = ee_hash64(key);
	uint64_t base_index = (hash >> 7) & dict->mask;
	uint8_t  hash_sign = hash & 0x7F;

	size_t probe_step = 0;

	while (probe_step < dict->cap)
	{
		size_t group_index = base_index & EE_GROUP_MASK;

		__m128i group = _mm_loadu_si128((__m128i*) & dict->ctrls[group_index]);
		__m128i match = _mm_cmpeq_epi8(group, _mm_set1_epi8(hash_sign));
		
		int32_t match_mask = _mm_movemask_epi8(match);

		if (match_mask)
		{
			for (int i = 0; i < EE_GROUP_SIZE; ++i)
			{
				if (!(match_mask & (1 << i))) 
				{
					continue;
				}

				if (dict->slots[group_index + i].key == key)
				{
					memcpy(dict->slots[group_index + i].val.bytes, val.bytes, EE_VALUE_SIZE);

					return;
				}
			}
		}

		__m128i empty = _mm_cmpeq_epi8(group, _mm_set1_epi8(EE_SLOT_EMPTY));
		__m128i deleted = _mm_cmpeq_epi8(group, _mm_set1_epi8(EE_SLOT_DELETED));

		int32_t free_mask = _mm_movemask_epi8(_mm_or_si128(empty, deleted));

		if (free_mask)
		{
			int32_t slot_index = group_index + ee_first_bit_u32(free_mask);

			dict->slots[slot_index].key = key;
			dict->ctrls[slot_index] = hash_sign;
			dict->count++;
			
			memcpy(dict->slots[slot_index].val.bytes, val.bytes, EE_VALUE_SIZE);

			return;
		}

		probe_step++;
		base_index = (base_index + probe_step * EE_GROUP_SIZE) & dict->mask;
	}
}

EE_INLINE void ee_dict_grow(Dict* dict)
{
	EE_ASSERT(dict != NULL, "Trying to insert to NULL Dict");

	Dict out = ee_dict_new(dict->cap * 2);

	for (size_t i = 0; i < dict->cap; ++i)
	{
		if (dict->ctrls[i] != EE_SLOT_EMPTY && dict->ctrls[i] != EE_SLOT_DELETED)
		{
			ee_dict_insert(&out, dict->slots[i].key, dict->slots[i].val);
		}
	}

	free(dict->slots);
	free(dict->ctrls);

	*dict = out;
}

EE_INLINE void ee_dict_add(Dict* dict, DictKey key, DictValue val)
{
	ee_dict_insert(dict, key, val);

	if (dict->count > dict->th)
	{
		ee_dict_grow(dict);
	}
}

EE_INLINE void ee_dict_remove(Dict* dict, DictKey key)
{
	EE_ASSERT(dict != NULL, "Trying to insert to NULL Dict");

	uint64_t hash = ee_hash64(key);
	uint64_t base_index = (hash >> 7) & dict->mask;
	uint8_t  hash_sign = hash & 0x7F;

	size_t probe_step = 0;

	while (probe_step < dict->cap)
	{
		size_t group_index = base_index & EE_GROUP_MASK;

		__m128i group = _mm_loadu_si128((__m128i*) & dict->ctrls[group_index]);
		__m128i match = _mm_cmpeq_epi8(group, _mm_set1_epi8(hash_sign));

		int32_t match_mask = _mm_movemask_epi8(match);

		if (match_mask)
		{
			for (int i = 0; i < EE_GROUP_SIZE; ++i)
			{
				if (!(match_mask & (1 << i)))
				{
					continue;
				}

				if (dict->slots[group_index + i].key == key)
				{
					dict->ctrls[group_index + i] = EE_SLOT_DELETED;
					dict->count--;
					
					return;
				}
			}
		}

		__m128i empty = _mm_cmpeq_epi8(group, _mm_set1_epi8(EE_SLOT_EMPTY));

		int32_t empty_mask = _mm_movemask_epi8(empty);

		if (empty_mask)
		{
			return;
		}

		probe_step++;
		base_index = (base_index + probe_step * EE_GROUP_SIZE) & dict->mask;
	}
}

EE_INLINE DictValue* ee_dict_at(Dict* dict, DictKey key)
{
	EE_ASSERT(dict != NULL, "Trying to insert to NULL Dict");

	uint64_t hash = ee_hash64(key);
	uint64_t base_index = (hash >> 7) & dict->mask;
	uint8_t  hash_sign = hash & 0x7F;

	size_t probe_step = 0;

	while (probe_step < dict->cap)
	{
		size_t group_index = base_index & EE_GROUP_MASK;

		__m128i group = _mm_loadu_si128((__m128i*) & dict->ctrls[group_index]);
		__m128i match = _mm_cmpeq_epi8(group, _mm_set1_epi8(hash_sign));

		int32_t match_mask = _mm_movemask_epi8(match);

		if (match_mask)
		{
			for (int i = 0; i < EE_GROUP_SIZE; ++i)
			{
				if (!(match_mask & (1 << i)))
				{
					continue;
				}

				if (dict->slots[group_index + i].key == key)
				{
					return &dict->slots[group_index + i].val;
				}
			}
		}

		__m128i empty = _mm_cmpeq_epi8(group, _mm_set1_epi8(EE_SLOT_EMPTY));

		int32_t empty_mask = _mm_movemask_epi8(empty);

		if (empty_mask)
		{
			return NULL;
		}

		probe_step++;
		base_index = (base_index + probe_step * EE_GROUP_SIZE) & dict->mask;
	}
}

EE_INLINE DictValue ee_dict_get(Dict* dict, DictKey key)
{
	DictValue* val = ee_dict_at(dict, key);

	if (val == NULL)
	{
		DictValue out = { 0 };

		return out;
	}

	return *val;
}

EE_INLINE int ee_dict_contains(Dict* dict, DictKey key)
{
	DictValue* val = ee_dict_at(dict, key);

	return val != NULL;
}

#endif // EE_DICT_H
