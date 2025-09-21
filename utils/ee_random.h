#ifndef EE_RANDOM_H
#define EE_RANDOM_H

#include "stdlib.h"
#include "stdint.h"
#include "math.h"

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
#else
#define EE_ASSERT(cond, fmt, ...)    ((void)0)
#endif // EE_NO_ASSERT

#ifndef EE_INLINE
#define EE_INLINE    static inline
#endif // EE_INLINE

#ifndef EE_TRUE
#define EE_TRUE     (1)
#endif // EE_TRUE

#ifndef EE_FALSE
#define EE_FALSE    (0)
#endif // EE_FALSE

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

#define EE_RNG_STATE_LEN    (2)
#define EE_RNG_EPS          (1e-16)
#define EE_RNG_EPSF         (1e-12f)

typedef struct Rng
{
	u64 state[EE_RNG_STATE_LEN];
	f64 spare;
	s32 have_spare;
} Rng;

EE_INLINE u64 ee_rotl_u64(u64 x, s32 k)
{
	return (x << k) | (x >> (64 - k));
}

EE_INLINE u64 ee_splitmix_u64(u64* state) 
{
	EE_ASSERT(state != NULL, "Trying to mix NULL state");

	u64 z = (*state += 0x9E3779B97F4A7C15ull);

	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;

	return z ^ (z >> 31);
}

EE_INLINE Rng ee_rng_new(u64 seed)
{
	Rng out = { 0 };

	out.state[0] = ee_splitmix_u64(&seed);
	out.state[1] = ee_splitmix_u64(&seed);

	out.have_spare = EE_FALSE;
	out.spare = 0.0;

	return out;
}

EE_INLINE u64 ee_rand_u64(Rng* rng)
{
	EE_ASSERT(rng != NULL, "Trying to sample from NULL RNG");

	u64 s_0 = rng->state[0];
	u64 s_1 = rng->state[1];
	u64 result = s_0 + s_1;

	s_1 = s_0 ^ s_1;

	rng->state[0] = ee_rotl_u64(s_0, 55) ^ s_1 ^ (s_1 << 14);
	rng->state[1] = ee_rotl_u64(s_1, 36);

	return result;
}

EE_INLINE u32 ee_rand_u32(Rng* rng)
{
	return (u32)(ee_rand_u64(rng) >> 32);
}

EE_INLINE f64 ee_rand_f64(Rng* rng)
{
	return (f64)(ee_rand_u64(rng) >> 11) * (1.0 / 9007199254740992.0);
}

EE_INLINE f32 ee_rand_f32(Rng* rng)
{
	return (f32)(ee_rand_u32(rng) >> 8) * (1.0f / 16777216.0f);
}

EE_INLINE u64 ee_rand_u64_b(Rng* rng, u64 bound)
{
	EE_ASSERT(rng != NULL, "Trying to sample from NULL RNG");

	if (bound == 0)
	{
		return 0;
	}

	if ((bound & (bound - 1ull)) == 0ull)
	{
		return ee_rand_u64(rng) & (bound - 1ull);
	}
	
	u64 threshold = (u64)(-(s64)bound) % bound;
	
	while (EE_TRUE)
	{
		u64 result = ee_rand_u64(rng);

		if (result >= threshold)
		{
			return result % bound;
		}
	}
}

EE_INLINE u32 ee_rand_u32_b(Rng* rng, u32 bound)
{
	EE_ASSERT(rng != NULL, "Trying to sample from NULL RNG");

	if (bound == 0)
	{
		return 0;
	}

	if ((bound & (bound - 1ull)) == 0ull) 
	{
		return ee_rand_u32(rng) & (bound - 1ull);
	}

	u32 threshold = (u32)(-(s32)bound) % bound;
	
	while (EE_TRUE)
	{
		u32 result = ee_rand_u32(rng);

		if (result >= threshold)
		{
			return result % bound;
		}
	}
}

EE_INLINE u64 ee_rand_u64_ab(Rng* rng, u64 a, u64 b)
{
	EE_ASSERT(rng != NULL, "Trying to sample from NULL RNG");
	EE_ASSERT(a <= b, "Incorrect bounds (%llu, %llu), 'a' should be smaller than 'b'", a, b);

	if (a == b)
	{
		return a;
	}

	if (a == 0 && b == UINT64_MAX)
	{
		return ee_rand_u64(rng);
	}

	return a + ee_rand_u64_b(rng, b - a);
}

EE_INLINE u32 ee_rand_u32_ab(Rng* rng, u32 a, u32 b)
{
	EE_ASSERT(rng != NULL, "Trying to sample from NULL RNG");
	EE_ASSERT(a <= b, "Incorrect bounds (%u, %u), 'a' should be smaller than 'b'", a, b);

	if (a == b)
	{
		return a;
	}

	if (a == 0 && b == UINT32_MAX)
	{
		return ee_rand_u32(rng);
	}

	return a + ee_rand_u32_b(rng, b - a);
}

EE_INLINE f64 ee_rand_f64_ab(Rng* rng, f64 a, f64 b)
{
	EE_ASSERT(a < b, "Incorrect bounds (%f, %f), 'a' should be smaller than 'b'", a, b);

	return a + (b - a) * ee_rand_f64(rng);
}

EE_INLINE f32 ee_rand_f32_ab(Rng* rng, f32 a, f32 b)
{
	EE_ASSERT(a < b, "Incorrect bounds (%f, %f), 'a' should be smaller than 'b'", a, b);

	return a + (b - a) * ee_rand_f32(rng);
}

EE_INLINE f64 ee_randn_f64(Rng* rng, f64 mean, f64 std)
{
	EE_ASSERT(rng != NULL, "Trying to sample from NULL RNG");
	EE_ASSERT(std >= 0.0, "Invalid value for std (%f)", std);

	if (std == 0.0)
	{
		return mean;
	}

	if (rng->have_spare)
	{
		rng->have_spare = EE_FALSE;

		return mean + std * rng->spare;
	}

	f64 u, v, s;

	do
	{
		u = ee_rand_f64_ab(rng, -1.0, 1.0);
		v = ee_rand_f64_ab(rng, -1.0, 1.0);

		s = u * u + v * v;
	} while (s >= 1.0 || s <= EE_RNG_EPS);

	f64 mul = sqrt(-2.0 * log(s) / s);
	f64 z_0 = u * mul;
	f64 z_1 = v * mul;

	rng->spare = z_1;
	rng->have_spare = EE_TRUE;

	return mean + std * z_0;
}

EE_INLINE f32 ee_randn_f32(Rng* rng, f32 mean, f32 std)
{
	EE_ASSERT(rng != NULL, "Trying to sample from NULL RNG");
	EE_ASSERT(std >= 0.0f, "Invalid value for std (%f)", std);

	if (std == 0.0f)
	{
		return mean;
	}

	if (rng->have_spare)
	{
		rng->have_spare = EE_FALSE;

		return mean + std * (f32)rng->spare;
	}

	f32 u, v, s;

	do
	{
		u = ee_rand_f32_ab(rng, -1.0f, 1.0f);
		v = ee_rand_f32_ab(rng, -1.0f, 1.0f);

		s = u * u + v * v;
	} while (s >= 1.0f || s <= EE_RNG_EPSF);

	f32 mul = sqrtf(-2.0f * logf(s) / s);
	f32 z_0 = u * mul;
	f32 z_1 = v * mul;

	rng->spare = (f64)z_1;
	rng->have_spare = EE_TRUE;

	return mean + std * z_0;
}

#endif // EE_RANDOM_H
