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

#endif
#else

#define EE_ASSERT(cond, fmt, ...)    ((void)0)

#endif

#ifndef EE_PRINT
#define EE_PRINT(fmt, ...)    fprintf(stdout, fmt, ##__VA_ARGS__)
#endif

#ifndef EE_PRINTLN
#define EE_PRINTLN(fmt, ...)    fprintf(stdout, fmt "\n", ##__VA_ARGS__)
#endif

#ifndef EE_INLINE
#define EE_INLINE    static inline
#endif

#ifndef EE_FIND_FIRST_BIT_INVALID
#define EE_FIND_FIRST_BIT_INVALID       (32)
#endif

#ifndef EE_FIND_FIRST_BIT_INVALID_64
#define EE_FIND_FIRST_BIT_INVALID_64    (64)
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

#ifndef EE_RECAST_U8
#define EE_RECAST_U8(x)        ((u8*)(&(x)))
#endif

#ifndef EE_RECAST_U8_PTR
#define EE_RECAST_U8_PTR(x)    ((u8**)(&(x)))
#endif

#ifndef EE_MAX_ALIGN
#define EE_MAX_ALIGN     (16)
#endif

#ifndef EE_ALIGN_MASK
#define EE_ALIGN_MASK    (~(EE_MAX_ALIGN - 1))
#endif

#ifndef EE_UNUSED

#define EE_UNUSED_1(a, ...)                ((void)(a))
#define EE_UNUSED_2(a, b, ...)             EE_UNUSED_1(a); EE_UNUSED_1(b)
#define EE_UNUSED_3(a, b, c, ...)          EE_UNUSED_1(a); EE_UNUSED_1(b); EE_UNUSED_1(c)
#define EE_UNUSED_4(a, b, c, d, ...)       EE_UNUSED_1(a); EE_UNUSED_1(b); EE_UNUSED_1(c); EE_UNUSED_1(d)
#define EE_UNUSED_5(a, b, c, d, e)         EE_UNUSED_1(a); EE_UNUSED_1(b); EE_UNUSED_1(c); EE_UNUSED_1(d); EE_UNUSED_1(e)

#define EE_UNUSED_IMPL(_1, _2, _3, _4, _5, NAME, ...)    NAME(_1, _2, _3, _4, _5)
#define EE_UNUSED(...)    EE_UNUSED_IMPL(__VA_ARGS__, EE_UNUSED_5, EE_UNUSED_4, EE_UNUSED_3, EE_UNUSED_2, EE_UNUSED_1)

#endif

#ifndef EE_ALIGNOF
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#include "stdalign.h"
#define EE_ALIGNOF(x)   alignof(x) 
#else
#define EE_ALIGNOF(x)   (EE_MAX_ALIGN)
#endif
#endif

#ifndef EE_ALIGNAS
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#include "stdalign.h"
#define EE_ALIGNAS(x)    alignas(x)
#elif defined(__GNUC__) || defined(__clang__)
#define EE_ALIGNAS(x)    __attribute__((aligned(x)))
#elif defined(_MSC_VER)
#define EE_ALIGNAS(x)    __declspec(align(x))
#else
#error "EE_ALIGNAS: unknown compiler, cannot set alignment"
#endif
#endif

#define EE_DEFINE_EQ_FN_SAFE(size)                                                \
    EE_INLINE i32 ee_eq_safe_##size(const u8* a_ptr, const u8* b_ptr, size_t len) \
    {                                                                             \
        EE_UNUSED_1(len);                                                         \
                                                                                  \
        u##size a, b;                                                             \
                                                                                  \
        memcpy(&a, a_ptr, sizeof(a));                                             \
        memcpy(&b, b_ptr, sizeof(b));                                             \
                                                                                  \
        return a == b;                                                            \
    }                                                               
                                                                    
#define EE_DEFINE_EQ_FN(size)                                                     \
    EE_INLINE i32 ee_eq_##size(const u8* a_ptr, const u8* b_ptr, size_t len)      \
    {                                                                             \
        EE_UNUSED_1(len);                                                         \
                                                                                  \
        return *(const u##size*)a_ptr == *(const u##size*)b_ptr;                  \
    }

#define EE_DEFINE_CPY_FN(size)                                                    \
    EE_INLINE void ee_cpy_##size(u8* a_ptr, const u8* b_ptr, size_t len)          \
    {                                                                             \
        EE_UNUSED_1(len);                                                         \
                                                                                  \
        *(u##size*)a_ptr = *(const u##size*)b_ptr;                                \
    }

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

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

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

#ifndef EE_SIMD_LEVEL_SSE
#define EE_SIMD_LEVEL_SSE    (1)
#endif

#ifndef EE_SIMD_LEVEL_AVX
#define EE_SIMD_LEVEL_AVX    (2)
#endif

#ifndef EE_SIMD_MAX_LEVEL
#define EE_SIMD_MAX_LEVEL     (EE_SIMD_LEVEL_AVX)
#endif

#ifndef EE_SIMD_DICT_DES_LEVEL
#define EE_SIMD_DICT_DES_LEVEL    (EE_SIMD_LEVEL_SSE)
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


//#define EE_SIMD_EFFECTIVE_MAX_LEVEL EE_SIMD_LEVEL_SSE
//#define EE_SIMD_EFFECTIVE_MAX_LEVEL EE_SIMD_LEVEL_NONE

#if EE_SIMD_EFFECTIVE_MAX_LEVEL == EE_SIMD_LEVEL_AVX

#include "immintrin.h"

typedef __m256i  ee_simd_i; // integer
typedef __m256   ee_simd_f; // float
typedef __m256d  ee_simd_d; // double

#define EE_SIMD_BYTES         32
#define EE_SIMD_DWORDS        (EE_SIMD_BYTES / 4)
#define EE_SIMD_PREFETCH_T0   (_MM_HINT_T0)

#define ee_loadu_si           _mm256_loadu_si256
#define ee_load_si            _mm256_load_si256
#define ee_store_si           _mm256_store_si256

#define ee_set1_epi8          _mm256_set1_epi8
#define ee_set1_epi16         _mm256_set1_epi16
#define ee_set1_epi32         _mm256_set1_epi32
#define ee_set1_epi64         _mm256_set1_epi64x

#define ee_cmpeq_epi8         _mm256_cmpeq_epi8
#define ee_cmpeq_epi16        _mm256_cmpeq_epi16
#define ee_cmpeq_epi32        _mm256_cmpeq_epi32
#define ee_cmpeq_epi64        _mm256_cmpeq_epi64

#define ee_castsi_ps          _mm256_castsi256_ps
#define ee_castsi_pd          _mm256_castsi256_pd
#define ee_movemask_ps        _mm256_movemask_ps
#define ee_movemask_pd        _mm256_movemask_pd
#define ee_movemask_epi8      _mm256_movemask_epi8
#define ee_packs_epi16        _mm256_packs_epi16
#define ee_setzero_si         _mm256_setzero_si256

#define ee_or_si              _mm256_or_si256
#define ee_xor_si             _mm256_xor_si256
#define ee_and_si             _mm256_and_si256
#define ee_srl_epi16          _mm256_srl_epi16
#define ee_srl_epi32          _mm256_srl_epi32
#define ee_srl_epi64          _mm256_srl_epi64
#define ee_sll_epi16          _mm256_sll_epi16
#define ee_sll_epi32          _mm256_sll_epi32
#define ee_sll_epi64          _mm256_sll_epi64
#define ee_srli_epi16         _mm256_srli_epi16
#define ee_srli_epi32         _mm256_srli_epi32
#define ee_srli_epi64         _mm256_srli_epi64
#define ee_slli_epi16         _mm256_slli_epi16
#define ee_slli_epi32         _mm256_slli_epi32
#define ee_slli_epi64         _mm256_slli_epi64
#define ee_prefetch           _mm_prefetch

#define ee_min_epi32          _mm256_min_epi32
#define ee_max_epi32          _mm256_max_epi32

#define ee_mul_epi32          _mm256_mul_epi32
#define ee_mul_epu32          _mm256_mul_epu32
#define ee_add_epi64          _mm256_add_epi64
#define ee_mullo_epi32        _mm256_mullo_epi32

#define ee_unpackhi_epi32     _mm256_unpackhi_epi32
#define ee_unpacklo_epi32     _mm256_unpacklo_epi32

#define ee_mullo_epi64        _ee_mullo_epi64

#elif EE_SIMD_EFFECTIVE_MAX_LEVEL == EE_SIMD_LEVEL_SSE

#include "immintrin.h"

typedef __m128i  ee_simd_i; // integer
typedef __m128   ee_simd_f; // float
typedef __m128d  ee_simd_d; // double

#define EE_SIMD_BYTES         16
#define EE_SIMD_DWORDS        (EE_SIMD_BYTES / 4)
#define EE_SIMD_PREFETCH_T0   (_MM_HINT_T0)

#define ee_loadu_si           _mm_loadu_si128
#define ee_load_si            _mm_load_si128
#define ee_store_si           _mm_store_si128

#define ee_set1_epi8          _mm_set1_epi8
#define ee_set1_epi16         _mm_set1_epi16
#define ee_set1_epi32         _mm_set1_epi32
#define ee_set1_epi64         _mm_set1_epi64x

#define ee_cmpeq_epi8         _mm_cmpeq_epi8
#define ee_cmpeq_epi16        _mm_cmpeq_epi16
#define ee_cmpeq_epi32        _mm_cmpeq_epi32
#define ee_cmpeq_epi64        _mm_cmpeq_epi64

#define ee_castsi_ps          _mm_castsi128_ps
#define ee_castsi_pd          _mm_castsi128_pd
#define ee_movemask_ps        _mm_movemask_ps
#define ee_movemask_pd        _mm_movemask_pd
#define ee_movemask_epi8      _mm_movemask_epi8
#define ee_packs_epi16        _mm_packs_epi16
#define ee_setzero_si         _mm_setzero_si128

#define ee_or_si              _mm_or_si128
#define ee_xor_si             _mm_xor_si128
#define ee_and_si             _mm_and_si128
#define ee_srl_epi16          _mm_srl_epi16
#define ee_srl_epi32          _mm_srl_epi32
#define ee_srl_epi64          _mm_srl_epi64
#define ee_sll_epi16          _mm_sll_epi16
#define ee_sll_epi32          _mm_sll_epi32
#define ee_sll_epi64          _mm_sll_epi64
#define ee_srli_epi16         _mm_srli_epi16
#define ee_srli_epi32         _mm_srli_epi32
#define ee_srli_epi64         _mm_srli_epi64
#define ee_slli_epi16         _mm_slli_epi16
#define ee_slli_epi32         _mm_slli_epi32
#define ee_slli_epi64         _mm_slli_epi64
#define ee_prefetch           _mm_prefetch

#define ee_min_epi32          _mm_min_epi32
#define ee_max_epi32          _mm_max_epi32

#define ee_mul_epi32          _mm_mul_epi32
#define ee_mul_epu32          _mm_mul_epu32
#define ee_add_epi64          _mm_add_epi64
#define ee_mullo_epi32        _mm_mullo_epi32

#define ee_unpackhi_epi32     _mm_unpackhi_epi32
#define ee_unpacklo_epi32     _mm_unpacklo_epi32

#define ee_mullo_epi64        _ee_mullo_epi64

#elif EE_SIMD_EFFECTIVE_MAX_LEVEL == EE_SIMD_LEVEL_NONE

typedef u64 ee_simd_i; // integer
typedef f64 ee_simd_f; // float
typedef f64 ee_simd_d; // double

#define EE_SIMD_BYTES         8
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

EE_INLINE void _ee_store_si(const void* dest, ee_simd_i a)
{
    *(ee_simd_i*)dest = a;
}

EE_INLINE void _ee_storeu_si(const void* dest, ee_simd_i a)
{
    memcpy(dest, a, sizeof(a));
}

EE_INLINE ee_simd_i _ee_set1_epi8(u8 byte)
{
    ee_simd_i out = 0;
    memset(&out, byte, sizeof(out));
    return out;
}

EE_INLINE ee_simd_i _ee_set1_epi16(u16 val)
{
    ee_simd_i out = 0;
    
    u16* out_p = (u16*)&out;

    out_p[0] = val;
    out_p[1] = val;
    out_p[2] = val;
    out_p[3] = val;

    return out;
}

EE_INLINE ee_simd_i _ee_set1_epi32(u32 val)
{
    ee_simd_i out = 0;    
    
    u32* out_p = (u32*)&out;

    out_p[0] = val;
    out_p[1] = val;

    return out;
}

EE_INLINE ee_simd_i _ee_set1_epi64(u64 val)
{
    return val;
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

EE_INLINE ee_simd_i _ee_cmpeq_epi16(ee_simd_i a, ee_simd_i b)
{
    ee_simd_i out = 0;

    u16* out_u16 = (u16*)&out;
    const u16* a_u16 = (const u16*)&a;
    const u16* b_u16 = (const u16*)&b;

    for (size_t i = 0; i < EE_SIMD_BYTES / 2; ++i)
    {
        out_u16[i] = (a_u16[i] == b_u16[i]) ? 0xFFFFu : 0x0000u;
    }

    return out;
}

EE_INLINE ee_simd_i _ee_cmpeq_epi32(ee_simd_i a, ee_simd_i b)
{
    ee_simd_i out = 0;

    u32* out_u32 = (u32*)&out;
    const u32* a_u32 = (const u32*)&a;
    const u32* b_u32 = (const u32*)&b;

    for (size_t i = 0; i < EE_SIMD_BYTES / 4; ++i)
    {
        out_u32[i] = (a_u32[i] == b_u32[i]) ? 0xFFFFFFFFu : 0x00000000u;
    }

    return out;
}

EE_INLINE ee_simd_i _ee_cmpeq_epi64(ee_simd_i a, ee_simd_i b)
{
    return a == b ? 0xFFFFFFFFFFFFFFFFu : 0x0000000000000000u;
}

EE_INLINE i32 _ee_movemask_epi8(ee_simd_i a)
{
    i32 m = 0;
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

EE_INLINE ee_simd_i _ee_xor_si(ee_simd_i a, ee_simd_i b)
{
    return a ^ b;
}

EE_INLINE void _ee_prefetch(const void* p, i32 sel)
{
    (void)p;
    (void)sel;
}

EE_INLINE ee_simd_i _ee_castst_ps(ee_simd_i a)
{
    return a;
}

EE_INLINE ee_simd_i _ee_castst_pd(ee_simd_i a)
{
    return a;
}

EE_INLINE i32 _ee_movemask_ps(ee_simd_i a)
{
    i32 m = 0;

    const u32* a_u32 = (const u32*)&a;

    m |= ((a_u32[0] >> 31) & 1) << 0;
    m |= ((a_u32[1] >> 31) & 1) << 1;

    return m;
}

EE_INLINE i32 _ee_movemask_pd(ee_simd_i a)
{
    return (a >> 63) & 1;
}

EE_INLINE ee_simd_i _ee_packs_epi16(ee_simd_i a, ee_simd_i b)
{
    ee_simd_i out = 0;

    for (int i = 0; i < 4; ++i)
    {
        i16 ai = (a >> (i * 16)) & 0xFFFF;

        if (ai > 127) 
            ai = 127;
        else if (ai < -128) 
            ai = -128;

        out |= ((ee_simd_i)(ai & 0xFF)) << (i * 8);

        i16 bi = (b >> (i * 16)) & 0xFFFF;

        if (bi > 127) 
            bi = 127;
        else if (bi < -128) 
            bi = -128;

        out |= ((ee_simd_i)(bi & 0xFF)) << ((i + 4) * 8);
    }

    return out;
}

EE_INLINE ee_simd_i _ee_setzero_si()
{
    return 0;
}

EE_INLINE ee_simd_i _ee_srl_epi16(ee_simd_i a, ee_simd_i count)
{
    ee_simd_i out = 0;
    u16* out_u16 = (u16*)&out;

    const u16* a_u16 = (u16*)&a;

    if (count > 15)
    {
        return 0;
    }

    out_u16[0] = a_u16[0] >> count;
    out_u16[1] = a_u16[1] >> count;
    out_u16[2] = a_u16[2] >> count;
    out_u16[3] = a_u16[3] >> count;

    return out;
}

EE_INLINE ee_simd_i _ee_and_si(ee_simd_i a, ee_simd_i b)
{
    return a & b;
}

EE_INLINE ee_simd_i _ee_min_epi32(ee_simd_i a, ee_simd_i b)
{
    ee_simd_i out = 0;
    u32* out_u32 = (u32*)&out;

    const u32* a_u32 = (u32*)&a;
    const u32* b_u32 = (u32*)&b;

    out_u32[0] = a_u32[0] < b_u32[0] ? a_u32[0] : b_u32[0];
    out_u32[1] = a_u32[1] < b_u32[1] ? a_u32[1] : b_u32[1];

    return out;
}

EE_INLINE ee_simd_i _ee_max_epi32(ee_simd_i a, ee_simd_i b)
{
    ee_simd_i out = 0;
    u32* out_u32 = (u32*)&out;

    const u32* a_u32 = (u32*)&a;
    const u32* b_u32 = (u32*)&b;

    out_u32[0] = a_u32[0] > b_u32[0] ? a_u32[0] : b_u32[0];
    out_u32[1] = a_u32[1] > b_u32[1] ? a_u32[1] : b_u32[1];

    return out;
}

EE_INLINE ee_simd_i _ee_mul_epi32(ee_simd_i a, ee_simd_i b)
{
    const u32* a32 = (u32*)&a;
    const u32* b32 = (u32*)&b;

    ee_simd_i out = 0;
    
    u64* out64 = (u64*)&out;

    out64[0] = (u64)a32[0] * (u64)b32[0];
    out64[1] = (u64)a32[2] * (u64)b32[2];

    return out;
}

EE_INLINE ee_simd_i _ee_mullo_epi32(ee_simd_i a, ee_simd_i b)
{
    const u32* a32 = (u32*)&a;
    const u32* b32 = (u32*)&b;

    ee_simd_i out = 0;

    u32* out32 = (u32*)&out;

    out32[0] = a32[0] * b32[0];
    out32[1] = a32[1] * b32[1];
    out32[2] = a32[2] * b32[2];
    out32[3] = a32[3] * b32[3];

    return out;
}

EE_INLINE ee_simd_i _ee_srl_epi32(ee_simd_i a, ee_simd_i count)
{
    ee_simd_i out = 0;
    u32* out32 = (u32*)&out;
    const u32* a32 = (const u32*)&a;

    for (int i = 0; i < 4; ++i)
    {
        if (count >= 32)
            out32[i] = 0;
        else
            out32[i] = a32[i] >> count;
    }

    return out;
}

EE_INLINE ee_simd_i _ee_sll_epi16(ee_simd_i a, ee_simd_i count)
{
    ee_simd_i out = 0;
    u16* out16 = (u16*)&out;
    const u16* a16 = (const u16*)&a;

    for (int i = 0; i < 4; ++i)
    {
        if (count >= 16)
            out16[i] = 0;
        else
            out16[i] = a16[i] << count;
    }

    return out;
}

EE_INLINE ee_simd_i _ee_sll_epi32(ee_simd_i a, ee_simd_i count)
{
    ee_simd_i out = 0;
    u32* out32 = (u32*)&out;
    const u32* a32 = (const u32*)&a;

    for (int i = 0; i < 4; ++i)
    {
        if (count >= 32)
            out32[i] = 0;
        else
            out32[i] = a32[i] << count;
    }

    return out;
}

#define ee_loadu_si           _ee_load_si
#define ee_load_si            _ee_load_si
#define ee_store_si           _ee_store_si
#define ee_storeu_si          _ee_storeu_si

#define ee_set1_epi8          _ee_set1_epi8
#define ee_set1_epi16         _ee_set1_epi16
#define ee_set1_epi32         _ee_set1_epi32
#define ee_set1_epi64         _ee_set1_epi64

#define ee_cmpeq_epi8         _ee_cmpeq_epi8
#define ee_cmpeq_epi16        _ee_cmpeq_epi16
#define ee_cmpeq_epi32        _ee_cmpeq_epi32
#define ee_cmpeq_epi64        _ee_cmpeq_epi64

#define ee_castsi_ps          _ee_castst_ps
#define ee_castsi_pd          _ee_castst_pd
#define ee_movemask_epi8      _ee_movemask_epi8
#define ee_movemask_ps        _ee_movemask_ps
#define ee_movemask_pd        _ee_movemask_pd
#define ee_packs_epi16        _ee_packs_epi16
#define ee_setzero_si         _ee_setzero_si

#define ee_or_si              _ee_or_si
#define ee_xor_si             _ee_xor_si
#define ee_and_si             _ee_and_si
#define ee_srl_epi16          _ee_srl_epi16
#define ee_srl_epi32          _ee_srl_epi32
#define ee_sll_epi16          _ee_sll_epi16
#define ee_sll_epi32          _ee_sll_epi32
#define ee_prefetch           _ee_prefetch
#define ee_min_epi32          _ee_min_epi32
#define ee_max_epi32          _ee_max_epi32

#define ee_mul_epi32          _ee_mul_epi32
#define ee_mullo_epi32        _ee_mullo_epi32

#else
#error Invalid EE_SIMD_EFFECTIVE_MAX_LEVEL value
#endif

#endif // EE_SIMD

#ifndef EE_SIMD_DICT
#define EE_SIMD_DICT

#if EE_SIMD_DICT_MAX_LEVEL == EE_SIMD_LEVEL_AVX

#include "immintrin.h"

typedef __m256i eed_simd_i;

#define EED_SIMD_BYTES         (32)
#define EED_SIMD_PREFETCH_T0   (_MM_HINT_T0)

#define eed_loadu_si           _mm256_loadu_si256
#define eed_load_si            _mm256_load_si256

#define eed_set1_epi8          _mm256_set1_epi8
#define eed_set1_epi16         _mm256_set1_epi16
#define eed_set1_epi32         _mm256_set1_epi32
#define eed_set1_epi64         _mm256_set1_epi64x

#define eed_cmpeq_epi8         _mm256_cmpeq_epi8
#define eed_cmpeq_epi16        _mm256_cmpeq_epi16
#define eed_cmpeq_epi32        _mm256_cmpeq_epi32
#define eed_cmpeq_epi64        _mm256_cmpeq_epi64

#define eed_castsi_ps          _mm256_castsi256_ps
#define eed_castsi_pd          _mm256_castsi256_pd
#define eed_movemask_ps        _mm256_movemask_ps
#define eed_movemask_pd        _mm256_movemask_pd
#define eed_movemask_epi8      _mm256_movemask_epi8
#define eed_packs_epi16        _mm256_packs_epi16
#define eed_setzero_si         _mm256_setzero_si256

#define eed_or_si              _mm256_or_si256
#define eed_and_si             _mm256_and_si256
#define eed_srl_epi16          _mm256_srl_epi16
#define eed_prefetch           _mm_prefetch

#define eed_min_epi32          _mm256_min_epi32
#define eed_max_epi32          _mm256_max_epi32

#elif EE_SIMD_DICT_MAX_LEVEL == EE_SIMD_LEVEL_SSE

#include "immintrin.h"

typedef __m128i eed_simd_i;

#define EED_SIMD_BYTES         (16)
#define EED_SIMD_PREFETCH_T0   (_MM_HINT_T0)

#define eed_loadu_si           _mm_loadu_si128
#define eed_load_si            _mm_load_si128

#define eed_set1_epi8          _mm_set1_epi8
#define eed_set1_epi16         _mm_set1_epi16
#define eed_set1_epi32         _mm_set1_epi32
#define eed_set1_epi64         _mm_set1_epi64x

#define eed_cmpeq_epi8         _mm_cmpeq_epi8
#define eed_cmpeq_epi16        _mm_cmpeq_epi16
#define eed_cmpeq_epi32        _mm_cmpeq_epi32
#define eed_cmpeq_epi64        _mm_cmpeq_epi64

#define eed_castsi_ps          _mm_castsi128_ps
#define eed_castsi_pd          _mm_castsi128_pd
#define eed_movemask_ps        _mm_movemask_ps
#define eed_movemask_pd        _mm_movemask_pd
#define eed_movemask_epi8      _mm_movemask_epi8
#define eed_packs_epi16        _mm_packs_epi16
#define eed_setzero_si         _mm_setzero_si128

#define eed_or_si              _mm_or_si128
#define eed_and_si             _mm_and_si128
#define eed_srl_epi16          _mm_srl_epi16
#define eed_prefetch           _mm_prefetch

#define eed_min_epi32          _mm_min_epi32
#define eed_max_epi32          _mm_max_epi32

#elif EE_SIMD_DICT_MAX_LEVEL == EE_SIMD_LEVEL_NONE

typedef ee_simd_i eed_simd_i;

#define EED_SIMD_BYTES         (8)
#define EED_SIMD_PREFETCH_T0   (0)

#define eed_loadu_si           _ee_load_si
#define eed_load_si            _ee_load_si

#define eed_set1_epi8          _ee_set1_epi8
#define eed_set1_epi16         _ee_set1_epi16
#define eed_set1_epi32         _ee_set1_epi32
#define eed_set1_epi64         _ee_set1_epi64

#define eed_cmpeq_epi8         _ee_cmpeq_epi8
#define eed_cmpeq_epi16        _ee_cmpeq_epi16
#define eed_cmpeq_epi32        _ee_cmpeq_epi32
#define eed_cmpeq_epi64        _ee_cmpeq_epi64

#define eed_castsi_ps          _ee_castst_ps
#define eed_castsi_pd          _ee_castst_pd
#define eed_movemask_epi8      _ee_movemask_epi8
#define eed_movemask_ps        _ee_movemask_ps
#define eed_movemask_pd        _ee_movemask_pd
#define eed_packs_epi16        _ee_packs_epi16
#define eed_setzero_si         _ee_setzero_si

#define eed_or_si              _ee_or_si
#define eed_and_si             _ee_and_si
#define eed_srl_epi16          _ee_srl_epi16
#define eed_prefetch           _ee_prefetch
#define eed_min_epi32          _ee_min_epi32
#define eed_max_epi32          _ee_max_epi32

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

EE_INLINE i32 ee_first_bit_u32(u32 x)
{
#if defined(__BMI__)
    return _tzcnt_u32(x);
#elif defined(__GNUC__) || defined(__clang__)
    return x ? __builtin_ctz(x) : EE_FIND_FIRST_BIT_INVALID;
#elif defined(_MSC_VER)
    unsigned long i;

    if (_BitScanForward(&i, x))
    {
        return (i32)i;
    }
    else
    {
        return EE_FIND_FIRST_BIT_INVALID;
    }
#else
    for (i32 i = 0; i < 32; ++i)
    {
        if (x & (1u << i))
        {
            return i;
        }
    }

    return EE_FIND_FIRST_BIT_INVALID;
#endif
}

EE_INLINE i32 ee_first_bit_u64(u64 x)
{
#if defined(__BMI__)
    return _tzcnt_u64(x);
#elif defined(__GNUC__) || defined(__clang__)
    return x ? __builtin_ctzll(x) : EE_FIND_FIRST_BIT_INVALID_64;
#elif defined(_MSC_VER)
    unsigned long i;

    if (_BitScanForward64(&i, x))
    {
        return (i32)i;
    }
    else
    {
        return EE_FIND_FIRST_BIT_INVALID_64;
    }
#else
    for (i32 i = 0; i < 64; ++i)
    {
        if (x & (1u << i))
        {
            return i;
        }
    }

    return EE_FIND_FIRST_BIT_INVALID_64;
#endif
}

EE_INLINE i32 ee_first_zero_u32(u32 x)
{
    return ee_first_bit_u32(~x);
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

EE_INLINE i32 ee_popcnt_u32(u32 x)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcount(x);
#elif defined(_MSC_VER)
    return __popcnt(x);
#else
    i32 count = 0;

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

EE_INLINE u64 ee_max_u64(u64 a, u64 b)
{
    return a > b ? a : b;
}

EE_INLINE size_t ee_round_up_pow2(size_t x, size_t r)
{
    return (x + r - 1) & ~(r - 1);
}

EE_INLINE size_t ee_round_down_pow2(size_t x, size_t r)
{
    return x & (~(r - 1));
}

EE_INLINE i32 ee_eq_def(const u8* a_ptr, const u8* b_ptr, size_t len)
{
    return memcmp(a_ptr, b_ptr, len) == 0;
}

EE_INLINE void ee_cpy_def(u8* a_ptr, const u8* b_ptr, size_t len)
{
    memcpy(a_ptr, b_ptr, len);
}

#if EE_SIMD_EFFECTIVE_MAX_LEVEL >= EE_SIMD_LEVEL_SSE
EE_INLINE i32 ee_eq_safe_128(const u8* a_ptr, const u8* b_ptr, size_t len)
{
    EE_UNUSED(len);

    __m128i a = _mm_loadu_si128((const __m128i*)a_ptr);
    __m128i b = _mm_loadu_si128((const __m128i*)b_ptr);
    __m128i cmp = _mm_cmpeq_epi8(a, b);

    return _mm_movemask_epi8(cmp) == 0xFFFF;
}
#endif

#if EE_SIMD_EFFECTIVE_MAX_LEVEL >= EE_SIMD_LEVEL_AVX
EE_INLINE i32 ee_eq_safe_256(const u8* a_ptr, const u8* b_ptr, size_t len)
{
    EE_UNUSED(len);
    
    __m256i a = _mm256_loadu_si256((const __m256i*)a_ptr);
    __m256i b = _mm256_loadu_si256((const __m256i*)b_ptr);
    __m256i cmp = _mm256_cmpeq_epi8(a, b);

    return _mm256_movemask_epi8(cmp) == 0xFFFFFFFF;
}
#endif

EE_INLINE ee_simd_i _ee_mullo_epi64(ee_simd_i ab, ee_simd_i cd)
{
    ee_simd_i ac = ee_mul_epu32(ab, cd);
    ee_simd_i b = ee_srli_epi64(ab, 32);
    ee_simd_i bc = ee_mul_epu32(b, cd);
    ee_simd_i d = ee_srli_epi64(cd, 32);
    ee_simd_i ad = ee_mul_epu32(ab, d);
    ee_simd_i high = ee_add_epi64(bc, ad);

    high = ee_slli_epi64(high, 32);

    return ee_add_epi64(high, ac);
}

EE_DEFINE_EQ_FN(8);
EE_DEFINE_EQ_FN(16);
EE_DEFINE_EQ_FN(32);
EE_DEFINE_EQ_FN(64);

EE_DEFINE_EQ_FN_SAFE(8);
EE_DEFINE_EQ_FN_SAFE(16);
EE_DEFINE_EQ_FN_SAFE(32);
EE_DEFINE_EQ_FN_SAFE(64);

EE_DEFINE_CPY_FN(8);
EE_DEFINE_CPY_FN(16);
EE_DEFINE_CPY_FN(32);
EE_DEFINE_CPY_FN(64);

EE_EXTERN_C_END

EE_INLINE size_t ee_strnlen(const char* str, size_t max_len)
{
    EE_ASSERT(str != NULL, "Trying to get length of NULL string");

    size_t upper = ee_round_down_pow2(max_len, EE_SIMD_BYTES);
    size_t i = 0;

    ee_simd_i pattern = ee_set1_epi8('\0');

    for (; i < upper; i += EE_SIMD_BYTES)
    {
        ee_simd_i group = ee_loadu_si((const ee_simd_i*)&str[i]);
        ee_simd_i match = ee_cmpeq_epi8(group, pattern);

        i32 mask = ee_movemask_epi8(match);

        if (mask)
        {
            size_t found = ee_first_bit_u32(mask);

            return i + found;
        }
    }

    for (; i < max_len; ++i)
    {
        if (str[i] == '\0')
        {
            return i;
        }
    }

    return i;
}

EE_INLINE size_t ee_strlen(const char* str)
{
    EE_ASSERT(str != NULL, "Trying to get length of NULL string");

    ee_simd_i pattern = ee_set1_epi8('\0');

    for (size_t i = 0;; i += EE_SIMD_BYTES)
    {
        ee_simd_i group = ee_loadu_si((const ee_simd_i*)&str[i]);
        ee_simd_i match = ee_cmpeq_epi8(group, pattern);

        i32 mask = ee_movemask_epi8(match);

        if (mask)
        {
            size_t found = ee_first_bit_u32(mask);

            return i + found;
        }
    }

    return (size_t)-1;
}

//
// End
//

#endif // EE_CORE_H