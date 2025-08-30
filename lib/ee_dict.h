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

#ifndef EE_FIND_FIRST_BIT_INVALID
	#define EE_FIND_FIRST_BIT_INVALID    (32)
#endif // EE_FIND_FIRST_BIT_INVALID

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
	uint8_t bytes[EE_VAL_SIZE];
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

EE_INLINE uint64_t ee_hash64(DictKey key) 
{
	uint64_t hash = ((uint64_t*)key.bytes)[0];

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

EE_INLINE int ee_key_cmp(DictKey first, DictKey second)
{
	return memcmp(first.bytes, second.bytes, EE_KEY_SIZE) == 0;
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

EE_INLINE void ee_dict_insert(Dict* dict, DictKey key, DictValue val)
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

		_mm_prefetch(&dict->ctrls[next_group_index], _MM_HINT_T0);
		_mm_prefetch(&dict->slots[next_group_index], _MM_HINT_T0);

		__m128i group = _mm_loadu_si128((__m128i*)&dict->ctrls[group_index]);
		__m128i match = _mm_cmpeq_epi8(group, hash_sign128);
		
		int32_t match_mask = _mm_movemask_epi8(match);

		while (match_mask)
		{
			int32_t first = ee_first_bit_u32(match_mask);

			if (ee_key_cmp(dict->slots[group_index + first].key, key))
			{
				dict->slots[group_index + first].val = val;
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

			dict->slots[slot_index].key = key;
			dict->slots[slot_index].val = val;
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

EE_INLINE void ee_dict_set(Dict* dict, DictKey key, DictValue val)
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

	__m128i hash_sign128 = _mm_set1_epi8(hash_sign);
	__m128i empty128 = _mm_set1_epi8(EE_SLOT_EMPTY);

	size_t probe_step = 0;

	while (probe_step < dict->cap)
	{
		size_t group_index = base_index & EE_GROUP_MASK;

		size_t next_probe_step = probe_step + 1;
		size_t next_base_index = (base_index + EE_GROUP_SIZE * next_probe_step) & dict->mask;
		size_t next_group_index = next_base_index & EE_GROUP_MASK;

		_mm_prefetch(&dict->ctrls[next_group_index], _MM_HINT_T0);
		_mm_prefetch(&dict->slots[next_group_index], _MM_HINT_T0);

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

EE_INLINE DictValue* ee_dict_at(Dict* dict, DictKey key)
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

		_mm_prefetch(&dict->ctrls[next_group_index], _MM_HINT_T0);
		_mm_prefetch(&dict->slots[next_group_index], _MM_HINT_T0);

		__m128i group = _mm_loadu_si128((__m128i*)&dict->ctrls[group_index]);
		__m128i match = _mm_cmpeq_epi8(group, hash_sign128);

		int32_t match_mask = _mm_movemask_epi8(match);
		
		while (match_mask)
		{
			int32_t first = ee_first_bit_u32(match_mask);

			if (ee_key_cmp(dict->slots[group_index + first].key, key))
			{
				return &dict->slots[group_index + first].val;
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

EE_INLINE DictValue ee_dict_get(Dict* dict, DictKey key)
{
	EE_ASSERT(dict != NULL, "Trying to get from NULL Dict");

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
	EE_ASSERT(dict != NULL, "Trying to check NULL Dict");

	DictValue* val = ee_dict_at(dict, key);

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

EE_INLINE int ee_dict_iter_next(DictIter* it, DictKey* key, DictValue* val)
{
	EE_ASSERT(it != NULL && it->dict != NULL, "Trying to iterate over NULL Dict");
	EE_ASSERT(key != NULL, "Trying to dereference NULL DictKey");
	EE_ASSERT(val != NULL, "Trying to dereference NULL DictValue");

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

EE_INLINE DictKey ee_key_from_2s32(int32_t a, int32_t b)
{
	DictKey out = { 0 };

	memcpy(out.bytes, (uint8_t*)&a, sizeof(int32_t));
	memcpy(out.bytes + sizeof(int32_t), (uint8_t*)&b, sizeof(int32_t));

	return out;
}

EE_INLINE DictValue ee_val_from_2s32(int32_t a, int32_t b)
{
	DictValue out = { 0 };

	memcpy(out.bytes, (uint8_t*)&a, sizeof(int32_t));
	memcpy(out.bytes + sizeof(int32_t), (uint8_t*)&b, sizeof(int32_t));

	return out;
}

EE_INLINE DictKey ee_key_from_2u32(uint32_t a, uint32_t b)
{
	DictKey out = { 0 };

	memcpy(out.bytes, (uint8_t*)&a, sizeof(uint32_t));
	memcpy(out.bytes + sizeof(uint32_t), (uint8_t*)&b, sizeof(uint32_t));

	return out;
}

EE_INLINE DictValue ee_val_from_2u32(uint32_t a, uint32_t b)
{
	DictValue out = { 0 };

	memcpy(out.bytes, (uint8_t*)&a, sizeof(uint32_t));
	memcpy(out.bytes + sizeof(uint32_t), (uint8_t*)&b, sizeof(uint32_t));

	return out;
}

EE_INLINE DictKey ee_key_from_s64(int64_t a)
{
	DictKey out = { 0 };

	memcpy(out.bytes, (uint8_t*)&a, sizeof(int64_t));

	return out;
}

EE_INLINE DictValue ee_val_from_s64(int64_t a)
{
	DictValue out = { 0 };

	memcpy(out.bytes, (uint8_t*)&a, sizeof(int64_t));

	return out;
}

EE_INLINE DictKey ee_key_from_u64(uint64_t a)
{
	DictKey out = { 0 };

	memcpy(out.bytes, (uint8_t*)&a, sizeof(uint64_t));

	return out;
}

EE_INLINE DictValue ee_val_from_u64(uint64_t a)
{
	DictValue out = { 0 };

	memcpy(out.bytes, (uint8_t*)&a, sizeof(uint64_t));

	return out;
}

EE_INLINE DictKey ee_key_from_4s16(int16_t a, int16_t b, int16_t c, int16_t d)
{
	DictKey out = { 0 };

	memcpy(out.bytes, (uint8_t*)&a, sizeof(int16_t));
	memcpy(out.bytes + sizeof(int16_t), (uint8_t*)&b, sizeof(int16_t));
	memcpy(out.bytes + 2 * sizeof(int16_t), (uint8_t*)&c, sizeof(int16_t));
	memcpy(out.bytes + 3 * sizeof(int16_t), (uint8_t*)&d, sizeof(int16_t));

	return out;
}

EE_INLINE DictValue ee_val_from_4s16(int16_t a, int16_t b, int16_t c, int16_t d)
{
	DictValue out = { 0 };

	memcpy(out.bytes, (uint8_t*)&a, sizeof(int16_t));
	memcpy(out.bytes + sizeof(int16_t), (uint8_t*)&b, sizeof(int16_t));
	memcpy(out.bytes + 2 * sizeof(int16_t), (uint8_t*)&c, sizeof(int16_t));
	memcpy(out.bytes + 3 * sizeof(int16_t), (uint8_t*)&d, sizeof(int16_t));

	return out;
}

EE_INLINE DictKey ee_key_from_bytes(const uint8_t* data, size_t len)
{
	EE_ASSERT(len > EE_VAL_SIZE, "Trying to create a dict key from (%zu) bytes when maximum key size (%d)", len, EE_KEY_SIZE);
	
	DictKey out = { 0 };
	memcpy(out.bytes, data, len > EE_KEY_SIZE ? EE_KEY_SIZE : len);

	return out;
}

EE_INLINE DictValue ee_val_from_bytes(const uint8_t* data, size_t len)
{
	EE_ASSERT(len > EE_VAL_SIZE, "Trying to create a dict value from (%zu) bytes when maximum value size (%d)", len, EE_VAL_SIZE);

	DictValue out = { 0 };
	memcpy(out.bytes, data, len > EE_VAL_SIZE ? EE_VAL_SIZE : len);

	return out;
}

EE_INLINE DictKey ee_key_from_8u8(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3,
	uint8_t a4, uint8_t a5, uint8_t a6, uint8_t a7)
{
	DictKey out = { 0 };

	out.bytes[0] = a0;
	out.bytes[1] = a1;
	out.bytes[2] = a2;
	out.bytes[3] = a3;
	out.bytes[4] = a4;
	out.bytes[5] = a5;
	out.bytes[6] = a6;
	out.bytes[7] = a7;

	return out;
}

EE_INLINE DictValue ee_val_from_8u8(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3,
	uint8_t a4, uint8_t a5, uint8_t a6, uint8_t a7)
{
	DictValue out = { 0 };

	out.bytes[0] = a0;
	out.bytes[1] = a1;
	out.bytes[2] = a2;
	out.bytes[3] = a3;
	out.bytes[4] = a4;
	out.bytes[5] = a5;
	out.bytes[6] = a6;
	out.bytes[7] = a7;

	return out;
}

EE_INLINE void ee_key_set_2s32(DictKey* out, int32_t a, int32_t b)
{
	memcpy(out->bytes, (uint8_t*)&a, sizeof(int32_t));
	memcpy(out->bytes + sizeof(int32_t), (uint8_t*)&b, sizeof(int32_t));
}

EE_INLINE void ee_val_set_2s32(DictValue* out, int32_t a, int32_t b)
{
	memcpy(out->bytes, (uint8_t*)&a, sizeof(int32_t));
	memcpy(out->bytes + sizeof(int32_t), (uint8_t*)&b, sizeof(int32_t));
}

EE_INLINE void ee_key_set_2u32(DictKey* out, uint32_t a, uint32_t b)
{
	memcpy(out->bytes, (uint8_t*)&a, sizeof(uint32_t));
	memcpy(out->bytes + sizeof(uint32_t), (uint8_t*)&b, sizeof(uint32_t));
}

EE_INLINE void ee_val_set_2u32(DictValue* out, uint32_t a, uint32_t b)
{
	memcpy(out->bytes, (uint8_t*)&a, sizeof(uint32_t));
	memcpy(out->bytes + sizeof(uint32_t), (uint8_t*)&b, sizeof(uint32_t));
}

EE_INLINE void ee_key_set_s64(DictKey* out, int64_t a)
{
	memcpy(out->bytes, (uint8_t*)&a, sizeof(int64_t));
}

EE_INLINE void ee_val_set_s64(DictValue* out, int64_t a)
{
	memcpy(out->bytes, (uint8_t*)&a, sizeof(int64_t));
}

EE_INLINE void ee_key_set_u64(DictKey* out, uint64_t a)
{
	memcpy(out->bytes, (uint8_t*)&a, sizeof(uint64_t));
}

EE_INLINE void ee_val_set_u64(DictValue* out, uint64_t a)
{
	memcpy(out->bytes, (uint8_t*)&a, sizeof(uint64_t));
}

EE_INLINE void ee_key_set_4s16(DictKey* out, int16_t a, int16_t b, int16_t c, int16_t d)
{
	memcpy(out->bytes, (uint8_t*)&a, sizeof(int16_t));
	memcpy(out->bytes + sizeof(int16_t), (uint8_t*)&b, sizeof(int16_t));
	memcpy(out->bytes + 2 * sizeof(int16_t), (uint8_t*)&c, sizeof(int16_t));
	memcpy(out->bytes + 3 * sizeof(int16_t), (uint8_t*)&d, sizeof(int16_t));
}

EE_INLINE void ee_val_set_4s16(DictValue* out, int16_t a, int16_t b, int16_t c, int16_t d)
{
	memcpy(out->bytes, (uint8_t*)&a, sizeof(int16_t));
	memcpy(out->bytes + sizeof(int16_t), (uint8_t*)&b, sizeof(int16_t));
	memcpy(out->bytes + 2 * sizeof(int16_t), (uint8_t*)&c, sizeof(int16_t));
	memcpy(out->bytes + 3 * sizeof(int16_t), (uint8_t*)&d, sizeof(int16_t));
}

EE_INLINE void ee_key_set_bytes(DictKey* out, const uint8_t* data, size_t len)
{
	memcpy(out->bytes, data, len > EE_KEY_SIZE ? EE_KEY_SIZE : len);
}

EE_INLINE void ee_val_set_bytes(DictValue* out, const uint8_t* data, size_t len)
{
	memcpy(out->bytes, data, len > EE_VAL_SIZE ? EE_VAL_SIZE : len);
}

EE_INLINE void ee_key_set_8u8(DictKey* out, uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3,
	uint8_t a4, uint8_t a5, uint8_t a6, uint8_t a7)
{
	out->bytes[0] = a0;
	out->bytes[1] = a1;
	out->bytes[2] = a2;
	out->bytes[3] = a3;
	out->bytes[4] = a4;
	out->bytes[5] = a5;
	out->bytes[6] = a6;
	out->bytes[7] = a7;
}

EE_INLINE void ee_val_set_8u8(DictValue* out, uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3,
	uint8_t a4, uint8_t a5, uint8_t a6, uint8_t a7)
{
	out->bytes[0] = a0;
	out->bytes[1] = a1;
	out->bytes[2] = a2;
	out->bytes[3] = a3;
	out->bytes[4] = a4;
	out->bytes[5] = a5;
	out->bytes[6] = a6;
	out->bytes[7] = a7;
}

#endif // EE_DICT_H
