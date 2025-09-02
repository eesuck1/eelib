#pragma once

#ifndef EE_DICT_H
#define EE_DICT_H

#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "immintrin.h"

#if defined(_MSC_VER)
	#include "intrin.h"
#endif

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

#define EE_GROUP_SIZE                (16)
#define EE_DICT_START_SIZE           (32)
#define EE_KEY_SIZE                  (8)
#define EE_VAL_SIZE                  (8)

#define EE_SLOT_EMPTY                (0x80)
#define EE_SLOT_DELETED              (0xFE)
#define EE_GROUP_MASK                (~(EE_GROUP_SIZE - 1))

#define EE_DICT_DT(x)                ((uint8_t*)(&(x)))

#ifndef EE_FIND_FIRST_BIT_INVALID
	#define EE_FIND_FIRST_BIT_INVALID    (32)
#endif // EE_FIND_FIRST_BIT_INVALID

#ifndef EE_TRUE
	#define EE_TRUE            (1)
#endif // EE_TRUE

#ifndef EE_FALSE
	#define EE_FALSE           (0)
#endif // EE_FALSE

typedef struct Slot
{
	uint8_t key[EE_KEY_SIZE];
	uint8_t val[EE_VAL_SIZE];
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

typedef struct DictIter
{
	Dict* dict;
	size_t index;
} DictIter;

EE_INLINE int32_t ee_first_bit_u32(uint32_t x)
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

EE_INLINE uint64_t ee_dict_th(uint64_t x)
{
	return (x * 896) >> 10;
}

EE_INLINE uint64_t ee_hash64(uint8_t* key) 
{
	uint64_t hash;

	memcpy(&hash, key, sizeof(uint64_t));

	hash ^= hash >> 30;
	hash *= 0xbf58476d1ce4e5b9ULL;
	hash ^= hash >> 27;
	hash *= 0x94d049bb133111ebULL;
	hash ^= hash >> 31;

	return hash;
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

EE_INLINE int ee_key_cmp(uint8_t* first, uint8_t* second)
{
	return memcmp(first, second, EE_KEY_SIZE) == 0;
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

EE_INLINE void ee_dict_free(Dict* dict)
{
	EE_ASSERT(dict != NULL, "Trying to free NULL Dict");
	EE_ASSERT(dict->ctrls != NULL, "Trying to free NULL Dict.ctrls");
	EE_ASSERT(dict->slots != NULL, "Trying to free NULL Dict.slots");

	free(dict->ctrls);
	free(dict->slots);

	memset(dict, 0, sizeof(Dict));
}

EE_INLINE void ee_dict_insert(Dict* dict, uint8_t* key, uint8_t* val)
{
	EE_ASSERT(dict != NULL, "Trying to insert to NULL Dict");

	uint64_t hash = ee_hash64(key);
	uint64_t base_index = (hash >> 7) & dict->mask;
	uint8_t  hash_sign = hash & 0x7F;

	__m128i hash_sign128 = _mm_set1_epi8(hash_sign);
	__m128i empty128 = _mm_set1_epi8(EE_SLOT_EMPTY);
	__m128i deleted128 = _mm_set1_epi8(EE_SLOT_DELETED);

	size_t probe_step = 0;

	while (probe_step < dict->cap)
	{
		size_t group_index = base_index & EE_GROUP_MASK;

		size_t next_probe_step = probe_step + 1;
		size_t next_base_index = (base_index + EE_GROUP_SIZE * next_probe_step) & dict->mask;
		size_t next_group_index = next_base_index & EE_GROUP_MASK;

		_mm_prefetch((const char*)&dict->ctrls[next_group_index], _MM_HINT_T0);
		_mm_prefetch((const char*)&dict->slots[next_group_index], _MM_HINT_T0);

		__m128i group = _mm_loadu_si128((__m128i*)&dict->ctrls[group_index]);
		__m128i match = _mm_cmpeq_epi8(group, hash_sign128);
		
		int32_t match_mask = _mm_movemask_epi8(match);

		while (match_mask)
		{
			int32_t first = ee_first_bit_u32(match_mask);

			if (ee_key_cmp(dict->slots[group_index + first].key, key))
			{
				memcpy(&dict->slots[group_index + first].val, val, EE_VAL_SIZE);
				return;
			}

			match_mask = match_mask & (~(1 << first));
		}

		__m128i empty = _mm_cmpeq_epi8(group, empty128);
		__m128i deleted = _mm_cmpeq_epi8(group, deleted128);

		int32_t free_mask = _mm_movemask_epi8(_mm_or_si128(empty, deleted));

		if (free_mask)
		{
			int32_t slot_index = group_index + ee_first_bit_u32(free_mask);

			memcpy(&dict->slots[slot_index].key, key, EE_KEY_SIZE);
			memcpy(&dict->slots[slot_index].val, val, EE_VAL_SIZE);

			dict->ctrls[slot_index] = hash_sign;
			dict->count++;

			return;
		}

		probe_step = next_probe_step;
		base_index = next_base_index;
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

EE_INLINE void ee_dict_set(Dict* dict, uint8_t* key, uint8_t* val)
{
	ee_dict_insert(dict, key, val);

	if (dict->count > dict->th)
	{
		ee_dict_grow(dict);
	}
}

EE_INLINE void ee_dict_remove(Dict* dict, uint8_t* key)
{
	EE_ASSERT(dict != NULL, "Trying to insert to NULL Dict");

	uint64_t hash = ee_hash64(key);
	uint64_t base_index = (hash >> 7) & dict->mask;
	uint8_t  hash_sign = hash & 0x7F;

	__m128i hash_sign128 = _mm_set1_epi8(hash_sign);
	__m128i empty128 = _mm_set1_epi8(EE_SLOT_EMPTY);

	size_t probe_step = 0;

	while (probe_step < dict->cap)
	{
		size_t group_index = base_index & EE_GROUP_MASK;

		size_t next_probe_step = probe_step + 1;
		size_t next_base_index = (base_index + EE_GROUP_SIZE * next_probe_step) & dict->mask;
		size_t next_group_index = next_base_index & EE_GROUP_MASK;

		_mm_prefetch((const char*)&dict->ctrls[next_group_index], _MM_HINT_T0);
		_mm_prefetch((const char*)&dict->slots[next_group_index], _MM_HINT_T0);

		__m128i group = _mm_loadu_si128((__m128i*)&dict->ctrls[group_index]);
		__m128i match = _mm_cmpeq_epi8(group, hash_sign128);

		int32_t match_mask = _mm_movemask_epi8(match);

		while (match_mask)
		{
			int32_t first = ee_first_bit_u32(match_mask);

			if (ee_key_cmp(dict->slots[group_index + first].key, key))
			{
				dict->ctrls[group_index + first] = EE_SLOT_DELETED;
				dict->count--;

				return;
			}

			match_mask = match_mask & (~(1 << first));
		}

		__m128i empty = _mm_cmpeq_epi8(group, empty128);

		int32_t empty_mask = _mm_movemask_epi8(empty);

		if (empty_mask)
		{
			return;
		}

		probe_step = next_probe_step;
		base_index = next_base_index;
	}
}

EE_INLINE uint8_t* ee_dict_at(Dict* dict, uint8_t* key)
{
	EE_ASSERT(dict != NULL, "Trying to insert to NULL Dict");

	uint64_t hash = ee_hash64(key);
	uint64_t base_index = (hash >> 7) & dict->mask;
	uint8_t  hash_sign = hash & 0x7F;

	__m128i hash_sign128 = _mm_set1_epi8(hash_sign);
	__m128i empty128 = _mm_set1_epi8(EE_SLOT_EMPTY);

	size_t probe_step = 0;

	while (probe_step < dict->cap)
	{
		size_t group_index = base_index & EE_GROUP_MASK;

		size_t next_probe_step = probe_step + 1;
		size_t next_base_index = (base_index + EE_GROUP_SIZE * next_probe_step) & dict->mask;
		size_t next_group_index = next_base_index & EE_GROUP_MASK;

		_mm_prefetch((const char*)&dict->ctrls[next_group_index], _MM_HINT_T0);
		_mm_prefetch((const char*)&dict->slots[next_group_index], _MM_HINT_T0);

		__m128i group = _mm_loadu_si128((__m128i*)&dict->ctrls[group_index]);
		__m128i match = _mm_cmpeq_epi8(group, hash_sign128);

		int32_t match_mask = _mm_movemask_epi8(match);
		
		while (match_mask)
		{
			int32_t first = ee_first_bit_u32(match_mask);

			if (ee_key_cmp(dict->slots[group_index + first].key, key))
			{
				return dict->slots[group_index + first].val;
			}

			match_mask = match_mask & (~(1 << first));
		}

		__m128i empty = _mm_cmpeq_epi8(group, empty128);

		int32_t empty_mask = _mm_movemask_epi8(empty);

		if (empty_mask)
		{
			return NULL;
		}

		probe_step = next_probe_step;
		base_index = next_base_index;
	}

	return NULL;
}

EE_INLINE int ee_dict_contains(Dict* dict, uint8_t* key)
{
	EE_ASSERT(dict != NULL, "Trying to check NULL Dict");

	uint8_t* val = ee_dict_at(dict, key);

	return val != NULL;
}

EE_INLINE DictIter ee_dict_iter_new(Dict* dict)
{
	EE_ASSERT(dict != NULL, "Trying to create iterator over NULL Dict");

	DictIter out = { 0 };

	out.dict = dict;
	out.index = 0;

	return out;
}

EE_INLINE void ee_dict_iter_reset(DictIter* it)
{
	EE_ASSERT(it != NULL, "Trying to reset NULL DictIter");

	it->index = 0;
}

EE_INLINE int ee_dict_iter_next(DictIter* it, uint8_t** key, uint8_t** val)
{
	EE_ASSERT(it != NULL && it->dict != NULL, "Trying to iterate over NULL Dict");
	EE_ASSERT(key != NULL, "Trying to dereference NULL uint8_t*");
	EE_ASSERT(val != NULL, "Trying to dereference NULL uint8_t*");

	while (it->index < it->dict->cap)
	{
		if (it->dict->ctrls[it->index] == EE_SLOT_EMPTY || it->dict->ctrls[it->index] == EE_SLOT_DELETED)
		{
			it->index++;
			continue;
		}

		*key = it->dict->slots[it->index].key;
		*val = it->dict->slots[it->index].val;
		it->index++;

		return EE_TRUE;
	}

	return EE_FALSE;
}

#endif // EE_DICT_H