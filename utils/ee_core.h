#ifndef EE_CORE_H
#define EE_CORE_H

#include "stdlib.h"
#include "string.h"
#include "stdint.h"

#if defined(_MSC_VER)
#include "intrin.h"
#endif


//
// Basic
//

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

#define EE_PRINT(fmt, ...)    fprintf(stdout, fmt, ##__VA_ARGS__)
#define EE_PRINTLN(fmt, ...)    fprintf(stdout, fmt "\n", ##__VA_ARGS__)

#endif
#else

#define EE_ASSERT(cond, fmt, ...)    ((void)0)

#endif

#ifndef EE_INLINE
#define EE_INLINE    static inline
#endif

#ifndef EE_FIND_FIRST_BIT_INVALID
#define EE_FIND_FIRST_BIT_INVALID    (32)
#endif

#ifndef EE_TRUE
#define EE_TRUE     (1)
#endif

#ifndef EE_FALSE
#define EE_FALSE    (0)
#endif

#ifndef EE_MEM_SIZES
#define EE_MEM_SIZES

#define EE_KB        (1 << 10)
#define EE_MB        (1 << 20)
#define EE_GB        (1 << 30)
#define EE_TB        (1ull << 40)

#define EE_NKB(n)    (n * EE_KB)
#define EE_NMB(n)    (n * EE_MB)
#define EE_NGB(n)    (n * EE_GB)
#define EE_NTB(n)    ((u64)n * EE_TB)

#endif // EE_MEM_SIZES

//
// Extern C
//

#ifdef __cplusplus
#define EE_EXTERN_C_START    extern "C" {
#define EE_EXTERN_C_END      }
#else
#define EE_EXTERN_C_START
#define EE_EXTERN_C_END
#endif

//
// Types
//

#ifndef EE_TYPES
#define EE_TYPES

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

typedef float       f32;
typedef double      f64;
typedef long double f80;

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
_Static_assert(sizeof(float) == 4, "f32: sizeof(float) != 4");
_Static_assert(sizeof(double) == 8, "f64: sizeof(double) != 8");
#endif

#endif // EE_TYPES

//
// SIMD
//

#ifndef EE_SIMD_LEVEL_NONE
#define EE_SIMD_LEVEL_NONE    (0)
#endif

#ifndef EE_SIMD_LEVEL_SSE2
#define EE_SIMD_LEVEL_SSE2    (1)
#endif

#ifndef EE_SIMD_LEVEL_AVX2
#define EE_SIMD_LEVEL_AVX2    (2)
#endif

#ifndef EE_SIMD_MAX_LEVEL
#define EE_SIMD_MAX_LEVEL     (EE_SIMD_LEVEL_AVX2)
#endif

#ifndef EE_SIMD_DICT_DES_LEVEL
#define EE_SIMD_DICT_DES_LEVEL    (EE_SIMD_LEVEL_SSE2)
#endif

#ifndef EE_SIMD_EFFECTIVE_MAX_LEVEL
#define EE_SIMD_EFFECTIVE_MAX_LEVEL    (EE_SIMD_MAX_LEVEL)
#endif

#ifndef EE_SIMD_DICT_MAX_LEVEL
#if EE_SIMD_DICT_DES_LEVEL > EE_SIMD_MAX_LEVEL
#define EE_SIMD_DICT_MAX_LEVEL    (EE_SIMD_MAX_LEVEL)
#else
#define EE_SIMD_DICT_MAX_LEVEL    (EE_SIMD_DICT_DES_LEVEL)
#endif
#endif

EE_EXTERN_C_START

#ifndef EE_SIMD
#define EE_SIMD

#if EE_SIMD_EFFECTIVE_MAX_LEVEL == EE_SIMD_LEVEL_AVX2

#include "immintrin.h"

typedef __m256i ee_simd_i;

#define EE_SIMD_BYTES         (32)
#define EE_SIMD_PREFETCH_T0   (_MM_HINT_T0)

#define ee_loadu_si           _mm256_loadu_si256
#define ee_load_si            _mm256_load_si256
#define ee_set1_epi8          _mm256_set1_epi8
#define ee_cmpeq_epi8         _mm256_cmpeq_epi8
#define ee_movemask_epi8      _mm256_movemask_epi8
#define ee_or_si              _mm256_or_si256
#define ee_prefetch           _mm_prefetch

#elif EE_SIMD_EFFECTIVE_MAX_LEVEL == EE_SIMD_LEVEL_SSE2

#include "immintrin.h"

typedef __m128i ee_simd_i;

#define EE_SIMD_BYTES         (16)
#define EE_SIMD_PREFETCH_T0   (_MM_HINT_T0)

#define ee_loadu_si           _mm_loadu_si128
#define ee_load_si            _mm_load_si128
#define ee_set1_epi8          _mm_set1_epi8
#define ee_cmpeq_epi8         _mm_cmpeq_epi8
#define ee_movemask_epi8      _mm_movemask_epi8
#define ee_or_si              _mm_or_si128
#define ee_prefetch           _mm_prefetch

#elif EE_SIMD_EFFECTIVE_MAX_LEVEL == EE_SIMD_LEVEL_NONE

typedef u64 ee_simd_i;

#define EE_SIMD_BYTES         (8)
#define EE_SIMD_PREFETCH_T0   (0)

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
_Static_assert(sizeof(ee_simd_i) == EE_SIMD_BYTES, "ee_simd_i size mismatch");
#endif

EE_INLINE ee_simd_i _ee_load_si(const void* p)
{
    ee_simd_i out = 0;
    memcpy(&out, p, sizeof(out));
    return out;
}

EE_INLINE ee_simd_i _ee_set1_epi8(u8 byte)
{
    ee_simd_i out = 0;
    memset(&out, byte, sizeof(out));
    return out;
}

EE_INLINE ee_simd_i _ee_cmpeq_epi8(ee_simd_i a, ee_simd_i b)
{
    ee_simd_i out = 0;

    u8* out_u8 = (u8*)&out;
    const u8* a_u8 = (const u8*)&a;
    const u8* b_u8 = (const u8*)&b;

    for (size_t i = 0; i < EE_SIMD_BYTES; ++i)
    {
        out_u8[i] = (a_u8[i] == b_u8[i]) ? 0xFFu : 0x00u;
    }

    return out;
}

EE_INLINE s32 _ee_movemask_epi8(ee_simd_i a)
{
    s32 m = 0;
    const u8* a_u8 = (const u8*)&a;

    for (size_t i = 0; i < EE_SIMD_BYTES; ++i)
    {
        m |= ((a_u8[i] >> 7) & 1) << i;
    }

    return m;
}

EE_INLINE ee_simd_i _ee_or_si(ee_simd_i a, ee_simd_i b)
{
    return a | b;
}

EE_INLINE void _ee_prefetch(const void* p, s32 sel)
{
    (void)p;
    (void)sel;
}

#define ee_loadu_si           _ee_load_si
#define ee_load_si            _ee_load_si
#define ee_set1_epi8          _ee_set1_epi8
#define ee_cmpeq_epi8         _ee_cmpeq_epi8
#define ee_movemask_epi8      _ee_movemask_epi8
#define ee_or_si              _ee_or_si
#define ee_prefetch           _ee_prefetch

#else
#error Invalid EE_SIMD_EFFECTIVE_MAX_LEVEL value
#endif

#endif // EE_SIMD


#ifndef EE_SIMD_DICT
#define EE_SIMD_DICT

#if EE_SIMD_DICT_MAX_LEVEL == EE_SIMD_LEVEL_AVX2

#include "immintrin.h"

typedef __m256i eed_simd_i;

#define EED_SIMD_BYTES         (32)
#define EED_SIMD_PREFETCH_T0   (_MM_HINT_T0)

#define eed_loadu_si           _mm256_loadu_si256
#define eed_load_si            _mm256_load_si256
#define eed_set1_epi8          _mm256_set1_epi8
#define eed_cmpeq_epi8         _mm256_cmpeq_epi8
#define eed_movemask_epi8      _mm256_movemask_epi8
#define eed_or_si              _mm256_or_si256
#define eed_prefetch           _mm_prefetch

#elif EE_SIMD_DICT_MAX_LEVEL == EE_SIMD_LEVEL_SSE2

#include "immintrin.h"

typedef __m128i eed_simd_i;

#define EED_SIMD_BYTES         (16)
#define EED_SIMD_PREFETCH_T0   (_MM_HINT_T0)

#define eed_loadu_si           _mm_loadu_si128
#define eed_load_si            _mm_load_si128
#define eed_set1_epi8          _mm_set1_epi8
#define eed_cmpeq_epi8         _mm_cmpeq_epi8
#define eed_movemask_epi8      _mm_movemask_epi8
#define eed_or_si              _mm_or_si128
#define eed_prefetch           _mm_prefetch

#elif EE_SIMD_DICT_MAX_LEVEL == EE_SIMD_LEVEL_NONE

typedef u64 eed_simd_i;

#define EED_SIMD_BYTES         (8)
#define EED_SIMD_PREFETCH_T0   (0)

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
_Static_assert(sizeof(eed_simd_i) == EED_SIMD_BYTES, "eed_simd_i size mismatch");
#endif

EE_INLINE eed_simd_i _eed_load_si(const void* p)
{
    eed_simd_i out = 0;
    memcpy(&out, p, sizeof(out));
    return out;
}

EE_INLINE eed_simd_i _eed_set1_epi8(u8 byte)
{
    eed_simd_i out = 0;
    memset(&out, byte, sizeof(out));
    return out;
}

EE_INLINE eed_simd_i _eed_cmpeq_epi8(eed_simd_i a, eed_simd_i b)
{
    eed_simd_i out = 0;

    u8* out_u8 = (u8*)&out;
    const u8* a_u8 = (const u8*)&a;
    const u8* b_u8 = (const u8*)&b;

    for (size_t i = 0; i < EED_SIMD_BYTES; ++i)
    {
        out_u8[i] = (a_u8[i] == b_u8[i]) ? 0xFFu : 0x00u;
    }

    return out;
}

EE_INLINE s32 _eed_movemask_epi8(eed_simd_i a)
{
    s32 m = 0;
    const u8* a_u8 = (const u8*)&a;

    for (size_t i = 0; i < EED_SIMD_BYTES; ++i)
    {
        m |= ((a_u8[i] >> 7) & 1) << i;
    }

    return m;
}

EE_INLINE eed_simd_i _eed_or_si(eed_simd_i a, eed_simd_i b)
{
    return a | b;
}

EE_INLINE void _eed_prefetch(const void* p, s32 sel)
{
    (void)p;
    (void)sel;
}

#define eed_loadu_si           _eed_load_si
#define eed_load_si            _eed_load_si
#define eed_set1_epi8          _eed_set1_epi8
#define eed_cmpeq_epi8         _eed_cmpeq_epi8
#define eed_movemask_epi8      _eed_movemask_epi8
#define eed_or_si              _eed_or_si
#define eed_prefetch           _eed_prefetch

#else
#error Invalid EE_SIMD_DICT_MAX_LEVEL value
#endif

#endif // EE_SIMD_DICT

//
// Allocator
//

#ifndef EE_ALLOCATOR
#define EE_ALLOCATOR

typedef struct Allocator
{
	void* (*alloc_fn)(struct Allocator* self, size_t size);
	void* (*realloc_fn)(struct Allocator* self, void* buffer, size_t old_size, size_t new_size);
	void  (*free_fn)(struct Allocator* self, void* buffer);
	void* context;
} Allocator;

EE_INLINE void* ee_default_alloc(Allocator* allocator, size_t size)
{
	(void)allocator;

	return malloc(size);
}

EE_INLINE void* ee_default_realloc(Allocator* allocator, void* buffer, size_t old_size, size_t new_size)
{
	(void)allocator;
	(void)old_size;

	return realloc(buffer, new_size);
}

EE_INLINE void ee_default_free(Allocator* allocator, void* buffer)
{
	(void)allocator;

	free(buffer);
}

#endif // EE_ALLOCATOR

//
// Alloca
//

#ifndef EE_ALLOCA
#ifdef _MSC_VER

#ifdef EE_USE_MALLOCA
#define EE_ALLOCA(size)    (_malloca(size))
#define EE_FREEA(ptr)      (_freea(ptr))
#else
#define EE_ALLOCA(size)    (alloca(size))
#define EE_FREEA(ptr)      ((void)(ptr))
#endif // EE_USE_MALLOCA

#else

#define EE_ALLOCA(size)    (alloca(size))
#define EE_FREEA(ptr)      ((void)(ptr))

#endif // _MSC_VER
#endif // EE_ALLOCA

//
// Binary Comparator
//

#ifndef EE_BIN_CMP
#define EE_BIN_CMP
typedef int (*BinCmp)(const void* a, const void* b);
#endif // EE_BIN_CMP

//
// Functions
//

EE_INLINE s32 ee_first_bit_u32(u32 x)
{
#if defined(__BMI__)
    return _tzcnt_u32(x);
#elif defined(__GNUC__) || defined(__clang__)
    return x ? __builtin_ctz(x) : EE_FIND_FIRST_BIT_INVALID;
#elif defined(_MSC_VER)
    unsigned long i;

    if (_BitScanForward(&i, x))
    {
        return (s32)i;
    }
    else
    {
        return EE_FIND_FIRST_BIT_INVALID;
    }
#else
    for (s32 i = 0; i < 32; ++i)
    {
        if (x & (1u << i))
        {
            return i;
        }
    }

    return EE_FIND_FIRST_BIT_INVALID;
#endif
}

EE_INLINE u64 ee_next_pow_2(u64 x)
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

EE_INLINE s32 ee_popcnt_u32(u32 x)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcount(x);
#elif defined(_MSC_VER)
    return __popcnt(x);
#else
    s32 count = 0;

    while (x)
    {
        x &= x - 1;
        count++;
    }

    return count;
#endif
}

EE_INLINE int ee_is_pow2(u64 x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}

EE_INLINE int ee_log2_u32(u32 x)
{
#if defined(__GNUC__) || defined(__clang__)
    return x ? 31 - __builtin_clz(x) : -1;
#elif defined(_MSC_VER)
    unsigned long i;

    if (_BitScanReverse(&i, x))
    {
        return i;
    }
    else
    {
        return -1;
    }
#else
    int out = -1;

    while (x)
    {
        out++;
        x >>= 1;
    }

    return out;
#endif
}

EE_INLINE u64 ee_min_u64(u64 a, u64 b)
{
    return a < b ? a : b;
}

EE_EXTERN_C_END

//
// End
//

#endif // EE_CORE_H