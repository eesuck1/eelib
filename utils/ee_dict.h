#pragma once

#ifndef EE_DICT_H
#define EE_DICT_H

#include "ee_core.h"

#if defined(_MSC_VER)
#include "intrin.h"
#endif

static const u64 EE_ZERO_U64 = 0;
static const u64 EE_ONE_U64  = 1;
static const u64 EE_MAX_U64  = 0xffffffffffffffff;

static const f64 EE_ZERO_F64 = 0.0;
static const f64 EE_ONE_F64  = 1.0;

#ifndef EE_DICT_START_SIZE
#define EE_DICT_START_SIZE           (32)
#endif

#define EE_GROUP_SIZE                (EED_SIMD_BYTES)
#define EE_KEY_SIZE                  (8)
#define EE_VAL_SIZE                  (8)

#define EE_SLOT_EMPTY                (0x80)
#define EE_SLOT_DELETED              (0xFE)
#define EE_GROUP_MASK                (~(EE_GROUP_SIZE - 1))

#define EE_DICT_DT(x)                ((u8*)(&(x)))
#define EE_CONST_ZERO                (EE_DICT_DT(EE_ZERO_U64))
#define EE_CONST_ONE                 (EE_DICT_DT(EE_ONE_U64))
#define EE_CONST_MAX_U64             (EE_DICT_DT(EE_MAX_U64))
#define EE_CONST_ZERO_F64            (EE_DICT_DT(EE_ZERO_F64))
#define EE_CONST_ONE_F64             (EE_DICT_DT(EE_ONE_F64))

typedef struct Slot
{
	u8 key[EE_KEY_SIZE];
	u8 val[EE_VAL_SIZE];
} Slot;

typedef struct Dict
{
	Slot* slots;
	u8* ctrls;

	size_t count;
	size_t cap;
	size_t mask;
	size_t th;

	Allocator allocator;
} Dict;

typedef struct DictIter
{
	Dict* dict;
	size_t index;
} DictIter;

EE_INLINE u64 ee_dict_th(u64 x)
{
	return (x * 896) >> 10;
}

EE_INLINE u64 ee_hash64(u8* key) 
{
	u64 hash;

	memcpy(&hash, key, sizeof(u64));

	hash ^= hash >> 30;
	hash *= 0xbf58476d1ce4e5b9ULL;
	hash ^= hash >> 27;
	hash *= 0x94d049bb133111ebULL;
	hash ^= hash >> 31;

	return hash;
}

EE_INLINE int ee_key_cmp(u8* first, u8* second)
{
	return memcmp(first, second, EE_KEY_SIZE) == 0;
}

EE_INLINE Dict ee_dict_new(size_t size, Allocator* allocator)
{
	Dict out = { 0 };

	if (size < EE_DICT_START_SIZE)
	{
		size = EE_DICT_START_SIZE;
	}

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

	EE_ASSERT(out.allocator.alloc_fn != NULL, "Trying to set NULL alloc callback");
	EE_ASSERT(out.allocator.realloc_fn != NULL, "Trying to set NULL realloc callback");
	EE_ASSERT(out.allocator.free_fn != NULL, "Trying to set NULL free callback");

	size_t cap = ee_next_pow_2(size);

	out.slots = (Slot*)out.allocator.alloc_fn(&out.allocator, sizeof(Slot) * cap);
	out.ctrls = (u8*)out.allocator.alloc_fn(&out.allocator, cap);

	out.count = 0;
	out.cap = cap;
	out.mask = out.cap - 1;
	out.th = ee_dict_th(out.cap);

	EE_ASSERT(out.slots != NULL, "Unable to allocate (%zu) bytes for Dict.slots", sizeof(Slot) * cap);
	EE_ASSERT(out.ctrls != NULL, "Unable to allocate (%zu) bytes for Dict.ctrls", sizeof(u8) * cap);

	if (out.ctrls != NULL)
	{
		memset(out.ctrls, EE_SLOT_EMPTY, sizeof(u8) * cap);
	}

	return out;
}

EE_INLINE void ee_dict_free(Dict* dict)
{
	EE_ASSERT(dict != NULL, "Trying to free NULL Dict");
	EE_ASSERT(dict->ctrls != NULL, "Trying to free NULL Dict.ctrls");
	EE_ASSERT(dict->slots != NULL, "Trying to free NULL Dict.slots");

	dict->allocator.free_fn(&dict->allocator, dict->ctrls);
	dict->allocator.free_fn(&dict->allocator, dict->slots);

	memset(dict, 0, sizeof(Dict));
}

EE_INLINE s32 ee_dict_insert(Dict* dict, u8* key, u8* val)
{
	EE_ASSERT(dict != NULL, "Trying to insert to NULL Dict");

	u64 hash = ee_hash64(key);
	u64 base_index = (hash >> 7) & dict->mask;
	u8  hash_sign = hash & 0x7F;

	eed_simd_i hash_sign128 = eed_set1_epi8(hash_sign);
	eed_simd_i empty128 = eed_set1_epi8(EE_SLOT_EMPTY);
	eed_simd_i deleted128 = eed_set1_epi8(EE_SLOT_DELETED);

	size_t probe_step = 0;
	size_t first_deleted = (size_t)-1;

	while (probe_step < dict->cap)
	{
		size_t group_index = base_index & EE_GROUP_MASK;

		size_t next_probe_step = probe_step + 1;
		size_t next_base_index = (base_index + EE_GROUP_SIZE * next_probe_step) & dict->mask;
		size_t next_group_index = next_base_index & EE_GROUP_MASK;

		_mm_prefetch((const char*)&dict->ctrls[next_group_index], _MM_HINT_T0);
		_mm_prefetch((const char*)&dict->slots[next_group_index], _MM_HINT_T0);

		eed_simd_i group = eed_loadu_si((eed_simd_i*)&dict->ctrls[group_index]);

		s32 match_mask = eed_movemask_epi8(eed_cmpeq_epi8(group, hash_sign128));
		
		while (match_mask)
		{
			s32 first = ee_first_bit_u32(match_mask);
		
			if (ee_key_cmp(dict->slots[group_index + first].key, key))
			{
				memcpy(&dict->slots[group_index + first].val, val, EE_VAL_SIZE);
			
				return EE_TRUE;
			}
			
			match_mask &= match_mask - 1;
		}

		s32 deleted_mask = eed_movemask_epi8(eed_cmpeq_epi8(group, deleted128));
		
		if (deleted_mask && first_deleted == (size_t)-1)
		{
			first_deleted = group_index + (size_t)ee_first_bit_u32(deleted_mask);
		}

		s32 empty_mask = eed_movemask_epi8(eed_cmpeq_epi8(group, empty128));
		
		if (empty_mask)
		{
			size_t place = (first_deleted != (size_t)-1) ? first_deleted : (group_index + (size_t)ee_first_bit_u32(empty_mask));
			
			memcpy(&dict->slots[place].key, key, EE_KEY_SIZE);
			memcpy(&dict->slots[place].val, val, EE_VAL_SIZE);
			
			dict->ctrls[place] = hash_sign;
			dict->count++;
			
			return EE_TRUE;
		}

		probe_step = next_probe_step;
		base_index = next_base_index;
	}

	if (first_deleted != (size_t)-1)
	{
		size_t place = first_deleted;

		memcpy(&dict->slots[place].key, key, EE_KEY_SIZE);
		memcpy(&dict->slots[place].val, val, EE_VAL_SIZE);
		
		dict->ctrls[place] = hash_sign;
		dict->count++;
		
		return EE_TRUE;
	}
}

EE_INLINE void ee_dict_grow(Dict* dict)
{
	EE_ASSERT(dict != NULL, "Trying to insert to NULL Dict");

	Dict out = ee_dict_new(dict->cap * 2, &dict->allocator);

	for (size_t i = 0; i < dict->cap; ++i)
	{
		if (dict->ctrls[i] != EE_SLOT_EMPTY && dict->ctrls[i] != EE_SLOT_DELETED)
		{
			ee_dict_insert(&out, dict->slots[i].key, dict->slots[i].val);
		}
	}

	dict->allocator.free_fn(&dict->allocator, dict->ctrls);
	dict->allocator.free_fn(&dict->allocator, dict->slots);

	*dict = out;
}

EE_INLINE s32 ee_dict_set(Dict* dict, u8* key, u8* val)
{
	s32 result = ee_dict_insert(dict, key, val);

	if (!result)
	{
		ee_dict_grow(dict);
		result = ee_dict_insert(dict, key, val);

		EE_ASSERT(result == EE_TRUE, "Unable to insert after grow");
	}

	if (dict->count > dict->th)
	{
		ee_dict_grow(dict);
	}

	return result;
}

EE_INLINE void ee_dict_remove(Dict* dict, u8* key)
{
	EE_ASSERT(dict != NULL, "Trying to insert to NULL Dict");

	u64 hash = ee_hash64(key);
	u64 base_index = (hash >> 7) & dict->mask;
	u8  hash_sign = hash & 0x7F;

	eed_simd_i hash_sign128 = eed_set1_epi8(hash_sign);
	eed_simd_i empty128 = eed_set1_epi8(EE_SLOT_EMPTY);

	size_t probe_step = 0;

	while (probe_step < dict->cap)
	{
		size_t group_index = base_index & EE_GROUP_MASK;

		size_t next_probe_step = probe_step + 1;
		size_t next_base_index = (base_index + EE_GROUP_SIZE * next_probe_step) & dict->mask;
		size_t next_group_index = next_base_index & EE_GROUP_MASK;

		_mm_prefetch((const char*)&dict->ctrls[next_group_index], _MM_HINT_T0);
		_mm_prefetch((const char*)&dict->slots[next_group_index], _MM_HINT_T0);

		eed_simd_i group = eed_loadu_si((eed_simd_i*)&dict->ctrls[group_index]);
		eed_simd_i match = eed_cmpeq_epi8(group, hash_sign128);

		s32 match_mask = eed_movemask_epi8(match);

		while (match_mask)
		{
			s32 first = ee_first_bit_u32(match_mask);

			if (ee_key_cmp(dict->slots[group_index + first].key, key))
			{
				dict->ctrls[group_index + first] = EE_SLOT_DELETED;
				dict->count--;

				return;
			}

			match_mask &= match_mask - 1;
		}

		eed_simd_i empty = eed_cmpeq_epi8(group, empty128);

		s32 empty_mask = eed_movemask_epi8(empty);

		if (empty_mask)
		{
			return;
		}

		probe_step = next_probe_step;
		base_index = next_base_index;
	}
}

EE_INLINE u8* ee_dict_at(Dict* dict, u8* key)
{
	EE_ASSERT(dict != NULL, "Trying to insert to NULL Dict");

	u64 hash = ee_hash64(key);
	u64 base_index = (hash >> 7) & dict->mask;
	u8  hash_sign = hash & 0x7F;

	eed_simd_i hash_sign128 = eed_set1_epi8(hash_sign);
	eed_simd_i empty128 = eed_set1_epi8(EE_SLOT_EMPTY);

	size_t probe_step = 0;

	while (probe_step < dict->cap)
	{
		size_t group_index = base_index & EE_GROUP_MASK;

		size_t next_probe_step = probe_step + 1;
		size_t next_base_index = (base_index + EE_GROUP_SIZE * next_probe_step) & dict->mask;
		size_t next_group_index = next_base_index & EE_GROUP_MASK;

		_mm_prefetch((const char*)&dict->ctrls[next_group_index], _MM_HINT_T0);
		_mm_prefetch((const char*)&dict->slots[next_group_index], _MM_HINT_T0);

		eed_simd_i group = eed_loadu_si((eed_simd_i*)&dict->ctrls[group_index]);
		eed_simd_i match = eed_cmpeq_epi8(group, hash_sign128);

		s32 match_mask = eed_movemask_epi8(match);
		
		while (match_mask)
		{
			s32 first = ee_first_bit_u32(match_mask);

			if (ee_key_cmp(dict->slots[group_index + first].key, key))
			{
				return dict->slots[group_index + first].val;
			}

			match_mask &= match_mask - 1;
		}

		eed_simd_i empty = eed_cmpeq_epi8(group, empty128);

		s32 empty_mask = eed_movemask_epi8(empty);

		if (empty_mask)
		{
			return NULL;
		}

		probe_step = next_probe_step;
		base_index = next_base_index;
	}

	return NULL;
}

EE_INLINE int ee_dict_contains(Dict* dict, u8* key)
{
	EE_ASSERT(dict != NULL, "Trying to check NULL Dict");

	u8* val = ee_dict_at(dict, key);

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

EE_INLINE int ee_dict_iter_next(DictIter* it, u8** key, u8** val)
{
	EE_ASSERT(it != NULL && it->dict != NULL, "Trying to iterate over NULL Dict");
	EE_ASSERT(key != NULL, "Trying to dereference NULL u8*");
	EE_ASSERT(val != NULL, "Trying to dereference NULL u8*");

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