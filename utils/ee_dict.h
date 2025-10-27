#pragma once

/*

TODO:

1. Safe/Fast versions of copy and comparison functions
2. Bulk table operations

*/

#ifndef EE_DICT_H
#define EE_DICT_H

#include "ee_core.h"

static const u64 EE_ZERO_U64 = 0;
static const u64 EE_ONE_U64  = 1;
static const u64 EE_MAX_U64  = 0xffffffffffffffff;

static const f64 EE_ZERO_F64 = 0.0;
static const f64 EE_ONE_F64  = 1.0;

#ifndef EE_DICT_START_SIZE
#define EE_DICT_START_SIZE           (32)
#endif

#define EE_GROUP_SIZE                (EED_SIMD_BYTES)

#define EE_SLOT_EMPTY                (0x80)
#define EE_SLOT_DELETED              (0xFE)
#define EE_GROUP_MASK                (~(EE_GROUP_SIZE - 1))

#define EE_CONST_ZERO                (EE_RECAST_U8(EE_ZERO_U64))
#define EE_CONST_ONE                 (EE_RECAST_U8(EE_ONE_U64))
#define EE_CONST_MAX_U64             (EE_RECAST_U8(EE_MAX_U64))
#define EE_CONST_ZERO_F64            (EE_RECAST_U8(EE_ZERO_F64))
#define EE_CONST_ONE_F64             (EE_RECAST_U8(EE_ONE_F64))

#ifndef EE_HASH_SAFE
#define EE_HASH_SAFE      (0)
#endif

#ifndef EE_HASH_FAST
#define EE_HASH_FAST      (1)
#endif

#ifndef EE_HASH_ROBUST
#define EE_HASH_ROBUST    (2)
#endif

#ifndef EE_HASH_SIMPLE
#define EE_HASH_SIMPLE    (3)
#endif

#ifndef EE_HASH_SAFETY_TYPE
#define EE_HASH_SAFETY_TYPE EE_HASH_SAFE
#endif

#ifndef EE_HASH_COMP_TYPE
#define EE_HASH_COMP_TYPE EE_HASH_SIMPLE
#endif

// #define EE_DICT_TOMBS_REHASH
typedef u64  (*DictHash)(const u8* key, size_t len);
typedef i32  (*DictEq)(const u8* a, const u8* b, size_t len);
typedef void (*DictCpy)(u8* dest, const u8* src, size_t len);

#define ee_dict_new_conf_m(size, key_type, val_type, config)     ee_dict_new(size, sizeof(key_type), sizeof(val_type), config)
#define ee_dict_new_m(size, key_type, val_type, \
	allocator, hash_fn, eq_fn, key_cpy_fn, val_cpy_fn)           ee_dict_new(size, sizeof(key_type), sizeof(val_type), (DictConfig) { allocator, hash_fn, eq_fn, key_cpy_fn, val_cpy_fn })
#define ee_dict_def_m(size, key_type, val_type)                  ee_dict_new(size, sizeof(key_type), sizeof(val_type), (DictConfig){ 0 })

EE_INLINE u64 ee_hash_u64_fast(const u8* key_ptr, size_t len)
{
	EE_UNUSED(len);

	u64 hash = *(u64*)key_ptr;

	hash *= 0x9E3779B185EBCA87ULL;
	hash ^= hash >> 33;

	return hash;
}

EE_INLINE u64 ee_hash_u64_safe(const u8* key_ptr, size_t len)
{
	EE_UNUSED(len);

	u64 hash;

	memcpy(&hash, key_ptr, sizeof(hash));

	hash *= 0x9E3779B185EBCA87ULL;
	hash ^= hash >> 33;

	return hash;
}

EE_INLINE u64 ee_hash_u32_fast(const u8* key_ptr, size_t len)
{
	EE_UNUSED(len);

	u32 key = *(u32*)key_ptr;
	u64 hash = key;

	hash *= 0x9E3779B185EBCA87ULL;
	hash ^= hash >> 33;

	return hash;
}

EE_INLINE u64 ee_hash_u32_safe(const u8* key_ptr, size_t len)
{
	EE_UNUSED(len);

	u32 key;
	memcpy(&key, key_ptr, sizeof(key));
	u64 hash = key;

	hash *= 0x9E3779B185EBCA87ULL;
	hash ^= hash >> 33;

	return hash;
}

EE_INLINE u64 ee_hash_mm_u32_fast(const u8* key_ptr, size_t len)
{
	EE_UNUSED(len);

	u32 key = *(const u32*)key_ptr;
	u64 hash = key;

	hash ^= hash >> 30;
	hash *= 0xbf58476d1ce4e5b9ULL;
	hash ^= hash >> 27;
	hash *= 0x94d049bb133111ebULL;
	hash ^= hash >> 31;

	return hash;
}

EE_INLINE u64 ee_hash_mm_u32_safe(const u8* key_ptr, size_t len)
{
	EE_UNUSED(len);

	u32 key;
	memcpy(&key, key_ptr, sizeof(u32));
	u64 hash = key;

	hash ^= hash >> 30;
	hash *= 0xbf58476d1ce4e5b9ULL;
	hash ^= hash >> 27;
	hash *= 0x94d049bb133111ebULL;
	hash ^= hash >> 31;

	return hash;
}


EE_INLINE u64 ee_hash_mm_u64_fast(const u8* key_ptr, size_t len)
{
	EE_UNUSED(len);

	u64 hash = *(u64*)key_ptr;

	hash ^= hash >> 30;
	hash *= 0xbf58476d1ce4e5b9ULL;
	hash ^= hash >> 27;
	hash *= 0x94d049bb133111ebULL;
	hash ^= hash >> 31;

	return hash;
}

EE_INLINE u64 ee_hash_mm_u64_safe(const u8* key_ptr, size_t len)
{
	EE_UNUSED(len);

	u64 hash;

	memcpy(&hash, key_ptr, sizeof(u64));

	hash ^= hash >> 30;
	hash *= 0xbf58476d1ce4e5b9ULL;
	hash ^= hash >> 27;
	hash *= 0x94d049bb133111ebULL;
	hash ^= hash >> 31;

	return hash;
}

EE_INLINE u64 ee_hash_u128_fast(const u8* key_ptr, size_t len)
{
	EE_UNUSED(len);

	u64* key = (u64*)key_ptr;
	u64 hash = key[0] * 0x9E3779B185EBCA87ULL;

	hash ^= key[1] + 0xC6BC279692B5C323ULL + (hash << 17);

	return hash;
}

EE_INLINE u64 ee_hash_u128_safe(const u8* key_ptr, size_t len)
{
	EE_UNUSED(len);

	u64 low, high, hash;

	memcpy(&low, key_ptr, sizeof(u64));
	memcpy(&high, &key_ptr[sizeof(u64)], sizeof(u64));

	hash = low * 0x9E3779B185EBCA87ULL;
	hash ^= high + 0xC6BC279692B5C323ULL + (hash << 17);

	return hash;
}

EE_INLINE u64 ee_hash_u256_fast(const u8* key_ptr, size_t len)
{
	EE_UNUSED(len);

	u64* key = (u64*)key_ptr;

	u64 hash = key[0] * 0x9E3779B185EBCA87ULL;
	hash ^= key[1] + 0x9e3779b97f4a7c15ull + (hash << 17);
	hash ^= key[2] + 0xC6BC279692B5C323ULL + (hash << 13);
	hash ^= key[3] + 0x165667B19E3779F9ULL + (hash << 11);

	return hash;
}

EE_INLINE u64 ee_hash_u256_safe(const u8* key_ptr, size_t len)
{
	EE_UNUSED(len);

	u64 k0, k1, k2, k3, hash;

	memcpy(&k0, &key_ptr[0], sizeof(u64));
	memcpy(&k1, &key_ptr[8], sizeof(u64));
	memcpy(&k2, &key_ptr[16], sizeof(u64));
	memcpy(&k3, &key_ptr[24], sizeof(u64));

	hash = k0 * 0x9E3779B185EBCA87ULL;
	hash ^= k1 + 0x9e3779b97f4a7c15ull + (hash << 17);
	hash ^= k2 + 0xC6BC279692B5C323ULL + (hash << 13);
	hash ^= k3 + 0x165667B19E3779F9ULL + (hash << 11);

	return hash;
}


EE_INLINE u64 ee_hash_mm(const u8* key, size_t len)
{
	u64 hash = 0x9e3779b97f4a7c15ull;
	size_t i = 0;

	for (; i + sizeof(u64) <= len; i += sizeof(u64))
	{
		u64 key_u64 = 0;
		memcpy(&key_u64, &key[i], sizeof(u64));

		hash ^= key_u64 + 0x9e3779b97f4a7c15ull + (hash << 6) + (hash >> 2);
		hash ^= hash >> 30;
		hash *= 0xbf58476d1ce4e5b9ULL;
		hash ^= hash >> 27;
	}

	if (len > i)
	{
		u64 key_rem = 0;
		memcpy(&key_rem, &key[i], len - i);

		hash ^= key_rem + 0x9e3779b97f4a7c15ull + (hash << 6) + (hash >> 2);
		hash ^= hash >> 30;
		hash *= 0xbf58476d1ce4e5b9ULL;
		hash ^= hash >> 27;
	}

	return hash;
}

EE_INLINE u64 ee_hash_fast(const u8* key, size_t len)
{
	EE_ALIGNAS(EE_SIMD_BYTES) u64 hash_u64[EE_SIMD_BYTES / 8] = { 0 };

	ee_simd_i hash = ee_set1_epi32(0x6d0f494f);
	u64 hash_out = 0;

	size_t i = 0;
	size_t upper = ee_round_down_pow2(len, EE_SIMD_BYTES);

	for (; i < upper; i += EE_SIMD_BYTES)
	{
		ee_simd_i group = ee_load_si((const ee_simd_i*)&key[i]);

		hash = ee_mullo_epi32(hash, group);
		hash = ee_xor_si(hash, ee_srli_epi32(hash, 17));
	}

	ee_store_si((ee_simd_i*)hash_u64, hash);

	hash_out ^= hash_u64[0] + 0x9e3779b97f4a7c15ull + (hash_u64[1] << 11);
#if EE_SIMD_BYTES > 16
	hash_out ^= hash_u64[2] + 0xbf58476d1ce4e5b9ull + (hash_u64[3] << 17);
#endif

	for (; i + sizeof(u64) <= len; i += sizeof(u64))
	{
		u64 key_u64 = 0;
		memcpy(&key_u64, &key[i], sizeof(u64));

		hash_out ^= key_u64 + 0x9e3779b97f4a7c15ull + (hash_out << 6);
	}

	if (len > i)
	{
		u64 key_rem = 0;
		memcpy(&key_rem, &key[i], len - i);

		hash_out ^= key_rem + 0x9e3779b97f4a7c15ull + (hash_out >> 2);
	}

	return hash_out;
}

EE_INLINE u64 ee_hash_safe(const u8* key, size_t len)
{
	EE_ALIGNAS(EE_SIMD_BYTES) u64 hash_u64[EE_SIMD_BYTES / 8] = { 0 };

	ee_simd_i hash = ee_set1_epi32(0x6d0f494f);
	u64 hash_out = 0;

#if EE_SIMD_EFFECTIVE_MAX_LEVEL > EE_SIMD_LEVEL_NONE
	__m128i count = _mm_set1_epi32(17);
#else
	i32 count = 17;
#endif

	size_t i = 0;
	size_t upper = ee_round_down_pow2(len, EE_SIMD_BYTES);

	for (; i < upper; i += EE_SIMD_BYTES)
	{
		ee_simd_i group = ee_loadu_si((const ee_simd_i*)&key[i]);

		hash = ee_mullo_epi32(hash, group);
		hash = ee_xor_si(hash, ee_srl_epi32(hash, count));
	}

	ee_store_si((ee_simd_i*)hash_u64, hash);

	hash_out ^= hash_u64[0] + 0x9e3779b97f4a7c15ull + (hash_u64[1] << 11);
#if EE_SIMD_BYTES > 16
	hash_out ^= hash_u64[2] + 0xbf58476d1ce4e5b9ull + (hash_u64[3] << 17);
#endif

	for (; i + sizeof(u64) <= len; i += sizeof(u64))
	{
		u64 key_u64 = 0;
		memcpy(&key_u64, &key[i], sizeof(u64));

		hash_out ^= key_u64 + 0x9e3779b97f4a7c15ull + (hash_out << 6);
	}

	if (len > i)
	{
		u64 key_rem = 0;
		memcpy(&key_rem, &key[i], len - i);

		hash_out ^= key_rem + 0x9e3779b97f4a7c15ull + (hash_out >> 2);
	}

	return hash_out;
}

//#define EE_HASH_SAFETY_TYPE EE_HASH_FAST
//#define EE_HASH_COMP_TYPE EE_HASH_SIMPLE

#if (EE_HASH_SAFETY_TYPE == EE_HASH_SAFE) && (EE_HASH_COMP_TYPE == EE_HASH_ROBUST)

#define eed_hash_32     ee_hash_mm_u32_safe
#define eed_hash_64     ee_hash_mm_u64_safe
#define eed_hash_128    ee_hash_u128_safe
#define eed_hash_256    ee_hash_u256_safe
#define eed_hash        ee_hash_safe

#elif (EE_HASH_SAFETY_TYPE == EE_HASH_SAFE) && (EE_HASH_COMP_TYPE == EE_HASH_SIMPLE)

#define eed_hash_32     ee_hash_u32_safe
#define eed_hash_64     ee_hash_u64_safe
#define eed_hash_128    ee_hash_u128_safe
#define eed_hash_256    ee_hash_u256_safe
#define eed_hash        ee_hash_safe

#elif (EE_HASH_SAFETY_TYPE == EE_HASH_FAST) && (EE_HASH_COMP_TYPE == EE_HASH_ROBUST)

#define eed_hash_32     ee_hash_mm_u32_fast
#define eed_hash_64     ee_hash_mm_u64_fast
#define eed_hash_128    ee_hash_u128_fast
#define eed_hash_256    ee_hash_u256_fast
#define eed_hash        ee_hash_fast

#elif (EE_HASH_SAFETY_TYPE == EE_HASH_FAST) && (EE_HASH_COMP_TYPE == EE_HASH_SIMPLE)

#define eed_hash_32     ee_hash_u32_fast
#define eed_hash_64     ee_hash_u64_fast
#define eed_hash_128    ee_hash_u128_fast
#define eed_hash_256    ee_hash_u256_fast
#define eed_hash        ee_hash_fast

#else
#error Invalid hash function macro setting
#endif // EE_HASH_TYPE

typedef struct AlignedBuffer
{
	size_t size;
	size_t align;
	u8*    buffer;
	void*  base;
} AlignedBuffer;

EE_INLINE AlignedBuffer ee_aligned_alloc(size_t size, size_t align, Allocator* allocator)
{
	EE_ASSERT(ee_is_pow2(align), "Alignment should be power of two");

	AlignedBuffer out = { 0 };
	size_t total_size = size + align - 1;

	out.size  = size;
	out.align = align;
	
	if (allocator == NULL)
		out.base = malloc(total_size);
	else
		out.base = allocator->alloc_fn(allocator, total_size);

	EE_ASSERT(out.base != NULL, "Unable to allocate (%zu) bytes for aligned alloc", total_size);

	uintptr_t base    = (uintptr_t)out.base;
	uintptr_t aligned = (base + (align - 1)) & ~(uintptr_t)(align - 1);

	out.buffer = (u8*)aligned;

	return out;
}

EE_INLINE void ee_aligned_free(AlignedBuffer* buffer, Allocator* allocator)
{
	EE_ASSERT(buffer->base != NULL, "Trying to free invalid aligned buffer, AlignedBuffer.base is NULL");
	EE_ASSERT(buffer->buffer != NULL, "Trying to free invalid aligned buffer, AlignedBuffer.buffer is NULL");

	if (allocator == NULL)
		free(buffer->base);
	else
		allocator->free_fn(allocator, buffer->base);

	memset(buffer, 0, sizeof(*buffer));
}

typedef struct DictConfig
{
	Allocator* allocator;
	DictHash   hash_fn;
	DictEq     eq_fn;
	DictCpy    key_cpy_fn;
	DictCpy    val_cpy_fn;
} DictConfig;

typedef struct Dict
{
	AlignedBuffer keys;
	AlignedBuffer vals;
	AlignedBuffer ctrl;

	size_t count;
	size_t cap;
	size_t mask;
	size_t th;

	size_t key_len;
	size_t val_len;

#ifdef EE_DICT_TOMBS_REHASH
	size_t tombs;
	size_t tombs_th;
#endif

	Allocator allocator;
	DictHash  hash_fn;
	DictEq    eq_fn;
	DictCpy   key_cpy_fn;
	DictCpy   val_cpy_fn;
} Dict;

typedef struct DictIter
{
	const Dict* dict;
	size_t it;
} DictIter;

EE_EXTERN_C_START

EE_INLINE u64 ee_dict_th(u64 cap)
{
	return (cap * 896) >> 10;
}

#ifdef EE_DICT_TOMBS_REHASH
EE_INLINE u64 ee_tombs_th(u64 cap)
{
	return cap >> 3;
}
#endif

EE_INLINE DictConfig ee_dict_config_new(Allocator* allocator, DictHash hash_fn, DictEq eq_fn, DictCpy key_cpy_fn, DictCpy val_cpy_fn)
{
	DictConfig out = { 0 };

	out.allocator  = allocator;
	out.hash_fn    = hash_fn;
	out.eq_fn      = eq_fn;
	out.key_cpy_fn = key_cpy_fn;
	out.val_cpy_fn = val_cpy_fn;

	return out;
}

EE_INLINE DictConfig ee_dict_config_def(void)
{
	DictConfig out = { 0 };

	return out;
}

EE_INLINE u8* ee_dict_key_at(const Dict* dict, size_t i)
{
	EE_ASSERT(dict != NULL, "Trying to acces NULL dict slot");
	EE_ASSERT(i < dict->cap, "Invalid key index (%zu) for dict with cap (%zu)", i, dict->cap);

	return &dict->keys.buffer[i * dict->key_len];
}

EE_INLINE u8* ee_dict_val_at(const Dict* dict, size_t i)
{
	EE_ASSERT(dict != NULL, "Trying to acces NULL dict slot");
	EE_ASSERT(i < dict->cap, "Invalid val index (%zu) for dict with cap (%zu)", i, dict->cap);

	return &dict->vals.buffer[i * dict->val_len];
}

EE_INLINE DictIter ee_dict_iter_new(const Dict* dict)
{
	EE_ASSERT(dict != NULL, "Trying to create iterator over NULL Dict");

	DictIter out = { 0 };

	out.dict = dict;
	out.it = 0;

	return out;
}

EE_INLINE void ee_dict_iter_reset(DictIter* iter)
{
	EE_ASSERT(iter != NULL, "Trying to reset NULL DictIter");

	iter->it = 0;
}

EE_INLINE DictConfig ee_dict_get_config(Dict* dict)
{
	DictConfig out = { 0 };

	out.allocator  = &dict->allocator;
	out.hash_fn    = dict->hash_fn;
	out.eq_fn      = dict->eq_fn;
	out.key_cpy_fn = dict->key_cpy_fn;
	out.val_cpy_fn = dict->val_cpy_fn;

	return out;
}

EE_INLINE i32 ee_dict_iter_sp_next(DictIter* iter, u8* key_out, u8* val_out)
{
	EE_ASSERT(iter != NULL, "Trying to dereference NULL DictIter");
	EE_ASSERT(key_out != NULL, "Trying to dereference NULL key");
	EE_ASSERT(val_out != NULL, "Trying to dereference NULL value");

	if (iter->it >= iter->dict->cap)
	{
		return EE_FALSE;
	}

	const u8* ctrls = (const u8*)iter->dict->ctrl.buffer;
	size_t i = iter->it;
	size_t high = ee_round_up_pow2(iter->it, EED_SIMD_BYTES);

	for (; i < high; ++i)
	{
		if (ctrls[i] & 0x80)
			continue;

		iter->dict->key_cpy_fn(key_out, ee_dict_key_at(iter->dict, i), iter->dict->key_len);
		iter->dict->val_cpy_fn(val_out, ee_dict_val_at(iter->dict, i), iter->dict->val_len);

		iter->it = i + 1;

		return EE_TRUE;
	}

	eed_simd_i p_empty = eed_set1_epi8(EE_SLOT_EMPTY);
	eed_simd_i p_deleted = eed_set1_epi8(EE_SLOT_DELETED);

	for (; i < iter->dict->cap; i += EED_SIMD_BYTES)
	{
		eed_simd_i group = eed_load_si((const eed_simd_i*)&ctrls[i]);
		eed_simd_i match = eed_or_si(eed_cmpeq_epi8(group, p_empty), eed_cmpeq_epi8(group, p_deleted));

		i32 mask = (~eed_movemask_epi8(match)) & ((1u << EED_SIMD_BYTES) - 1);

		if (mask)
		{
			i32 first = ee_first_bit_u32(mask);
			size_t pos = (size_t)first + i;

			iter->dict->key_cpy_fn(key_out, ee_dict_key_at(iter->dict, i), iter->dict->key_len);
			iter->dict->val_cpy_fn(val_out, ee_dict_val_at(iter->dict, i), iter->dict->val_len);

			iter->it = pos + 1;

			return EE_TRUE;
		}
	}

	return EE_FALSE;
}

EE_INLINE i32 ee_dict_iter_sp_next_ptr(DictIter* iter, u8** key_out, u8** val_out)
{
	EE_ASSERT(iter != NULL, "Trying to dereference NULL DictIter");
	EE_ASSERT(key_out != NULL, "Trying to dereference NULL key");
	EE_ASSERT(val_out != NULL, "Trying to dereference NULL value");

	if (iter->it >= iter->dict->cap)
	{
		return EE_FALSE;
	}

	const u8* ctrls = (const u8*)iter->dict->ctrl.buffer;
	size_t i = iter->it;
	size_t high = ee_round_up_pow2(iter->it, EED_SIMD_BYTES);

	for (; i < high; ++i)
	{
		if (ctrls[i] & 0x80)
			continue;

		*key_out = ee_dict_key_at(iter->dict, i);
		*val_out = ee_dict_val_at(iter->dict, i);

		iter->it = i + 1;

		return EE_TRUE;
	}

	eed_simd_i p_empty = eed_set1_epi8(EE_SLOT_EMPTY);
	eed_simd_i p_deleted = eed_set1_epi8(EE_SLOT_DELETED);

	for (; i < iter->dict->cap; i += EED_SIMD_BYTES)
	{
		eed_simd_i group = eed_load_si((const eed_simd_i*)&ctrls[i]);
		eed_simd_i match = eed_or_si(eed_cmpeq_epi8(group, p_empty), eed_cmpeq_epi8(group, p_deleted));

		i32 mask = (~eed_movemask_epi8(match)) & ((1u << EED_SIMD_BYTES) - 1);

		if (mask)
		{
			i32 first = ee_first_bit_u32(mask);
			size_t pos = (size_t)first + i;

			*key_out = ee_dict_key_at(iter->dict, pos);
			*val_out = ee_dict_val_at(iter->dict, pos);

			iter->it = pos + 1;

			return EE_TRUE;
		}
	}

	return EE_FALSE;
}

EE_INLINE i32 ee_dict_iter_next(DictIter* iter, u8* key_out, u8* val_out)
{
	EE_ASSERT(iter != NULL, "Trying to dereference NULL DictIter");
	EE_ASSERT(key_out != NULL, "Trying to dereference NULL key");
	EE_ASSERT(val_out != NULL, "Trying to dereference NULL value");

	if (iter->it >= iter->dict->cap)
	{
		return EE_FALSE;
	}

	const u8* ctrl = iter->dict->ctrl.buffer;

	for (size_t i = iter->it; i < iter->dict->cap; ++i)
	{
		if (ctrl[i] & 0x80)
			continue;

		iter->dict->key_cpy_fn(key_out, ee_dict_key_at(iter->dict, i), iter->dict->key_len);
		iter->dict->val_cpy_fn(val_out, ee_dict_val_at(iter->dict, i), iter->dict->val_len);

		iter->it = i + 1;

		return EE_TRUE;
	}

	return EE_FALSE;
}

EE_INLINE i32 ee_dict_iter_next_ptr(DictIter* iter, u8** key_out, u8** val_out)
{
	EE_ASSERT(iter != NULL, "Trying to dereference NULL DictIter");
	EE_ASSERT(key_out != NULL, "Trying to dereference NULL key");
	EE_ASSERT(val_out != NULL, "Trying to dereference NULL value");

	if (iter->it >= iter->dict->cap)
	{
		return EE_FALSE;
	}

	const u8* ctrl = iter->dict->ctrl.buffer;

	for (size_t i = iter->it; i < iter->dict->cap; ++i)
	{
		if (ctrl[i] & 0x80)
			continue;

		*key_out = ee_dict_key_at(iter->dict, i);
		*val_out = ee_dict_val_at(iter->dict, i);

		iter->it = i + 1;

		return EE_TRUE;
	}

	return EE_FALSE;
}

EE_INLINE Dict ee_dict_new(size_t size, size_t key_len, size_t val_len, DictConfig config)
{
	EE_ASSERT(key_len > 0, "Invalid key_len (%zu)", key_len);
	EE_ASSERT(val_len > 0, "Invalid val_len (%zu)", val_len);

	Dict out = { 0 };

	if (size < EE_DICT_START_SIZE)
	{
		size = EE_DICT_START_SIZE;
	}

	if (config.allocator == NULL)
	{
		out.allocator.alloc_fn = ee_default_alloc;
		out.allocator.realloc_fn = ee_default_realloc;
		out.allocator.free_fn = ee_default_free;
		out.allocator.context = NULL;
	}
	else
	{
		memcpy(&out.allocator, config.allocator, sizeof(Allocator));
	}

	EE_ASSERT(out.allocator.alloc_fn != NULL, "Trying to set NULL alloc callback");
	EE_ASSERT(out.allocator.realloc_fn != NULL, "Trying to set NULL realloc callback");
	EE_ASSERT(out.allocator.free_fn != NULL, "Trying to set NULL free callback");

	size_t cap = ee_next_pow_2(size);

	out.key_len = key_len;
	out.val_len = val_len;

	out.keys = ee_aligned_alloc(cap * out.key_len, EE_MAX_ALIGN, &out.allocator);
	out.vals = ee_aligned_alloc(cap * out.val_len, EE_MAX_ALIGN, &out.allocator);
	out.ctrl = ee_aligned_alloc(cap, EE_MAX_ALIGN, &out.allocator);

	out.count = 0;
	out.cap = cap;
	out.mask = out.cap - 1;
	out.th = ee_dict_th(out.cap);
	
	if (config.hash_fn == NULL)
	{
		if (out.key_len == 4)
			out.hash_fn = eed_hash_32;
		else if (out.key_len == 8)
			out.hash_fn = eed_hash_64;
		else if (out.key_len == 16)
			out.hash_fn = eed_hash_128;
		else if (out.key_len == 32)
			out.hash_fn = eed_hash_256;
		else
			out.hash_fn = eed_hash;
	}
	else
	{
		out.hash_fn = config.hash_fn;
	}

	if (config.eq_fn == NULL)
	{
		if (out.key_len == 1)
			out.eq_fn = ee_eq_safe_8;
		else if (out.key_len == 2)
			out.eq_fn = ee_eq_safe_16;
		else if (out.key_len == 4)
			out.eq_fn = ee_eq_safe_32;
		else if (out.key_len == 8)
			out.eq_fn = ee_eq_safe_64;
#if EE_SIMD_EFFECTIVE_MAX_LEVEL >= EE_SIMD_LEVEL_SSE
		else if (out.key_len == 16)
			out.eq_fn = ee_eq_safe_128;
#endif
#if EE_SIMD_EFFECTIVE_MAX_LEVEL >= EE_SIMD_LEVEL_AVX
		else if (out.key_len == 32)
			out.eq_fn = ee_eq_safe_256;
#endif
		else
			out.eq_fn = ee_eq_def;
	}
	else
	{
		out.eq_fn = config.eq_fn;
	}

	if (config.key_cpy_fn == NULL)
	{
		if (out.key_len == 1)
			out.key_cpy_fn = ee_cpy_8;
		else if (out.key_len == 2)
			out.key_cpy_fn = ee_cpy_16;
		else if (out.key_len == 4)
			out.key_cpy_fn = ee_cpy_32;
		else if (out.key_len == 8)
			out.key_cpy_fn = ee_cpy_64;
		else
			out.key_cpy_fn = ee_cpy_def;
	}
	else
	{
		out.key_cpy_fn = config.key_cpy_fn;
	}

	if (config.val_cpy_fn == NULL)
	{
		if (out.val_len == 1)
			out.val_cpy_fn = ee_cpy_8;
		else if (out.val_len == 2)
			out.val_cpy_fn = ee_cpy_16;
		else if (out.val_len == 4)
			out.val_cpy_fn = ee_cpy_32;
		else if (out.val_len == 8)
			out.val_cpy_fn = ee_cpy_64;
		else
			out.val_cpy_fn = ee_cpy_def;
	}
	else
	{
		out.val_cpy_fn = config.val_cpy_fn;
	}

#ifdef EE_DICT_TOMBS_REHASH
	out.tombs = 0;
	out.tombs_th = ee_tombs_th(out.cap);
#endif

	EE_ASSERT(out.ctrl.buffer != NULL, "NULL control buffer");

	if (out.ctrl.buffer != NULL)
		memset(out.ctrl.buffer, EE_SLOT_EMPTY, cap);

	return out;
}

EE_INLINE void ee_dict_free(Dict* dict)
{
	EE_ASSERT(dict != NULL, "Trying to free NULL Dict");

	ee_aligned_free(&dict->keys, &dict->allocator);
	ee_aligned_free(&dict->vals, &dict->allocator);
	ee_aligned_free(&dict->ctrl, &dict->allocator);

	memset(dict, 0, sizeof(Dict));
}

EE_INLINE i32 ee_dict_insert(Dict* dict, const u8* key, const u8* val)
{
	EE_ASSERT(dict != NULL, "Trying to dereference NULL Dict");
	EE_ASSERT(key != NULL, "Trying to dereference NULL key");
	EE_ASSERT(val != NULL, "Trying to dereference NULL value");

	u64 hash = dict->hash_fn(key, dict->key_len);
	u64 base_index = (hash >> 7) & dict->mask;
	u8  hash_sign = hash & 0x7F;

	eed_simd_i hash_sign_wide = eed_set1_epi8(hash_sign);
	eed_simd_i empty_wide = eed_set1_epi8(EE_SLOT_EMPTY);
	eed_simd_i del_wide = eed_set1_epi8(EE_SLOT_DELETED);

	size_t probe_step = 0;
	size_t first_deleted = (size_t)-1;

	while (probe_step < dict->cap)
	{
		size_t group_index = base_index & EE_GROUP_MASK;

		size_t next_probe_step = probe_step + 1;
		size_t next_base_index = (base_index + EE_GROUP_SIZE * next_probe_step) & dict->mask;
		size_t next_group_index = next_base_index & EE_GROUP_MASK;

		eed_prefetch((const char*)&dict->ctrl.buffer[next_group_index], EED_SIMD_PREFETCH_T0);
		eed_prefetch((const char*)ee_dict_key_at(dict, next_group_index), EED_SIMD_PREFETCH_T0);

		eed_simd_i group = eed_load_si((eed_simd_i*)&dict->ctrl.buffer[group_index]);

		i32 match_mask = eed_movemask_epi8(eed_cmpeq_epi8(group, hash_sign_wide));
		
		while (match_mask)
		{
			i32 first = ee_first_bit_u32(match_mask);
		
			if (dict->eq_fn(ee_dict_key_at(dict, group_index + first), key, dict->key_len))
			{
				dict->val_cpy_fn(ee_dict_val_at(dict, group_index + first), val, dict->val_len);
			
				return EE_TRUE;
			}
			
			match_mask &= match_mask - 1;
		}

		i32 empty_mask = eed_movemask_epi8(eed_cmpeq_epi8(group, empty_wide));

		if (empty_mask)
		{
			size_t place = (first_deleted != (size_t)-1) ? first_deleted : (group_index + (size_t)ee_first_bit_u32(empty_mask));

			dict->key_cpy_fn(ee_dict_key_at(dict, place), key, dict->key_len);
			dict->val_cpy_fn(ee_dict_val_at(dict, place), val, dict->val_len);

			dict->ctrl.buffer[place] = hash_sign;
			dict->count++;

			return EE_TRUE;
		}

		i32 deleted_mask = eed_movemask_epi8(eed_cmpeq_epi8(group, del_wide));
		
		if (deleted_mask && first_deleted == (size_t)-1)
		{
			first_deleted = group_index + (size_t)ee_first_bit_u32(deleted_mask);
		}

		probe_step = next_probe_step;
		base_index = next_base_index;
	}

	if (first_deleted != (size_t)-1)
	{
		size_t place = first_deleted;

		dict->key_cpy_fn(ee_dict_key_at(dict, place), key, dict->key_len);
		dict->val_cpy_fn(ee_dict_val_at(dict, place), val, dict->val_len);

		dict->ctrl.buffer[place] = hash_sign;
		dict->count++;

		return EE_TRUE;
	}

	return EE_FALSE;
}

EE_INLINE void ee_dict_grow(Dict* dict, size_t new_cap)
{
	EE_ASSERT(dict != NULL, "Trying to insert to NULL Dict");

	DictConfig config = ee_dict_get_config(dict);
	Dict out = ee_dict_new(dict->cap * 2, dict->key_len, dict->val_len, config);

	for (size_t i = 0; i < dict->cap; ++i)
	{
		if (dict->ctrl.buffer[i] & 0x80)
			continue;

		ee_dict_insert(&out, ee_dict_key_at(dict, i), ee_dict_val_at(dict, i));
	}

	ee_aligned_free(&dict->keys, &dict->allocator);
	ee_aligned_free(&dict->vals, &dict->allocator);
	ee_aligned_free(&dict->ctrl, &dict->allocator);

	*dict = out;
}

EE_INLINE void ee_dict_rehash(Dict* dict)
{
	EE_ASSERT(dict != NULL, "Trying to insert to NULL Dict");

	DictConfig config = ee_dict_get_config(dict);
	Dict out = ee_dict_new(dict->cap, dict->key_len, dict->val_len, config);

	for (size_t i = 0; i < dict->cap; ++i)
	{
		if (dict->ctrl.buffer[i] & 0x80)
			continue;

		ee_dict_insert(&out, ee_dict_key_at(dict, i), ee_dict_val_at(dict, i));
	}

	ee_aligned_free(&dict->keys, &dict->allocator);
	ee_aligned_free(&dict->vals, &dict->allocator);
	ee_aligned_free(&dict->ctrl, &dict->allocator);

	*dict = out;
}

EE_INLINE i32 ee_dict_set(Dict* dict, const u8* key, const u8* val)
{
	if (dict->count + 1 > dict->th)
	{
		ee_dict_grow(dict, 2 * dict->cap);
	}

	i32 result = ee_dict_insert(dict, key, val);

	return result;
}

EE_INLINE i32 ee_dict_remove(Dict* dict, const u8* key)
{
	EE_ASSERT(dict != NULL, "Trying to dereference NULL Dict");
	EE_ASSERT(key != NULL, "Trying to dereference NULL key");

	u64 hash = dict->hash_fn(key, dict->key_len);
	u64 base_index = (hash >> 7) & dict->mask;
	u8  hash_sign = hash & 0x7F;

	eed_simd_i hash_sign_wide = eed_set1_epi8(hash_sign);
	eed_simd_i empty_wide = eed_set1_epi8(EE_SLOT_EMPTY);

	size_t probe_step = 0;

	while (probe_step < dict->cap)
	{
		size_t group_index = base_index & EE_GROUP_MASK;

		size_t next_probe_step = probe_step + 1;
		size_t next_base_index = (base_index + EE_GROUP_SIZE * next_probe_step) & dict->mask;
		size_t next_group_index = next_base_index & EE_GROUP_MASK;

		eed_prefetch((const char*)&dict->ctrl.buffer[next_group_index], EED_SIMD_PREFETCH_T0);
		eed_prefetch((const char*)ee_dict_key_at(dict, next_group_index), EED_SIMD_PREFETCH_T0);

		eed_simd_i group = eed_load_si((eed_simd_i*)&dict->ctrl.buffer[group_index]);
		eed_simd_i match = eed_cmpeq_epi8(group, hash_sign_wide);

		i32 match_mask = eed_movemask_epi8(match);

		while (match_mask)
		{
			i32 first = ee_first_bit_u32(match_mask);

			if (dict->eq_fn(ee_dict_key_at(dict, group_index + first), key, dict->key_len))
			{
				dict->ctrl.buffer[group_index + first] = EE_SLOT_DELETED;
				dict->count--;

				#ifdef EE_DICT_TOMBS_REHASH
				dict->tombs++;

				if (dict->tombs >= dict->tombs_th)
				{
					ee_dict_rehash(dict);
				}
				#endif

				return EE_TRUE;
			}

			match_mask &= match_mask - 1;
		}

		eed_simd_i empty = eed_cmpeq_epi8(group, empty_wide);

		i32 empty_mask = eed_movemask_epi8(empty);

		if (empty_mask)
		{
			return EE_FALSE;
		}

		probe_step = next_probe_step;
		base_index = next_base_index;
	}

	return EE_FALSE;
}

EE_INLINE u8* ee_dict_at(const Dict* dict, const u8* key)
{
	EE_ASSERT(dict != NULL, "Trying to dereference NULL Dict");
	EE_ASSERT(key != NULL, "Trying to dereference NULL key");

	u64 hash = dict->hash_fn(key, dict->key_len);
	u64 base_index = (hash >> 7) & dict->mask;
	u8  hash_sign = hash & 0x7F;

	eed_simd_i hash_sign_wide = eed_set1_epi8(hash_sign);
	eed_simd_i empty_wide = eed_set1_epi8(EE_SLOT_EMPTY);

	size_t probe_step = 0;

	while (probe_step < dict->cap)
	{
		size_t group_index = base_index & EE_GROUP_MASK;

		size_t next_probe_step = probe_step + 1;
		size_t next_base_index = (base_index + EE_GROUP_SIZE * next_probe_step) & dict->mask;
		size_t next_group_index = next_base_index & EE_GROUP_MASK;

		eed_prefetch((const char*)&dict->ctrl.buffer[next_group_index], EED_SIMD_PREFETCH_T0);
		eed_prefetch((const char*)ee_dict_key_at(dict, next_group_index), EED_SIMD_PREFETCH_T0);

		eed_simd_i group = eed_load_si((eed_simd_i*)&dict->ctrl.buffer[group_index]);
		eed_simd_i match = eed_cmpeq_epi8(group, hash_sign_wide);

		i32 match_mask = eed_movemask_epi8(match);
		
		while (match_mask)
		{
			i32 first = ee_first_bit_u32(match_mask);

			if (dict->eq_fn(ee_dict_key_at(dict, group_index + first), key, dict->key_len))
			{
				return ee_dict_val_at(dict, group_index + first);
			}

			match_mask &= match_mask - 1;
		}

		eed_simd_i empty = eed_cmpeq_epi8(group, empty_wide);

		i32 empty_mask = eed_movemask_epi8(empty);

		if (empty_mask)
		{
			return NULL;
		}

		probe_step = next_probe_step;
		base_index = next_base_index;
	}

	return NULL;
}

EE_INLINE int ee_dict_contains(const Dict* dict, const u8* key)
{
	EE_ASSERT(dict != NULL, "Trying to check NULL Dict");
	EE_ASSERT(key != NULL, "Trying to check NULL key");

	u8* val = ee_dict_at(dict, key);

	return val != NULL;
}

EE_INLINE size_t ee_dict_count(const Dict* dict)
{
	EE_ASSERT(dict != NULL, "Trying to dereference NULL Dict");

	return dict->count;
}

EE_EXTERN_C_END

#endif // EE_DICT_H