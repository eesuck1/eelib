# **ee_random.h**

!!! warning "In progress"
    This documentation page is currently under development.

`ee_random.h` provides a pseudorandom number generator (PRNG) and functions for sampling from various distributions.

The generator is based on the `xorshift128+` algorithm. The `Rng` context is initialized from a 64-bit seed using the `SplitMix64` algorithm. This module provides functions to generate uniformly distributed integers and floats, as well as normally distributed (Gaussian) floats.

## Defines

| Macro              | Value                | Description                                                                                                     |
|:-------------------|:---------------------|:----------------------------------------------------------------------------------------------------------------|
| `EE_RNG_STATE_LEN` | `(2)`                | The number of 64-bit integers in the `Rng` state array.                                                         |
| `EE_RNG_EPS`       | `(1e-16)`            | A small `f64` epsilon value used for numerical stability in the normal distribution generator (`ee_randn_f64`). |
| `EE_RNG_EPSF`      | `(1e-12f)`           | A small `f32` epsilon value used for numerical stability in the normal distribution generator (`ee_randn_f32`). |
| `EE_RNG_SEED_DEF`  | `(0xF23A9BC7...ull)` | A default 64-bit seed constant.                                                                                 |

## Structures

<span id="rng"></span> 
The context structure for the PRNG. It holds the internal state and a cached value for the normal distribution generator.

```c
typedef struct Rng
{
    u64 state[EE_RNG_STATE_LEN];
    f64 spare;
    i32 have_spare;
} Rng;
```

??? "Structure Members"

    | Members      | Type     | Description                                                                                       |
    |:-------------|:---------|:--------------------------------------------------------------------------------------------------|
    | `state`      | `u64[2]` | The internal 128-bit state of the `xorshift128+` generator.                                       |
    | `spare`      | `f64`    | A cached `f64` value from the normal distribution generator, which produces two values at a time. |
    | `have_spare` | `i32`    | A flag (0 or 1) indicating if the `spare` field contains a valid cached value.                    |

## Functions (Lifecycle)

??? "EE_INLINE Rng ee_rng_new(u64 seed)"

    <span id="ee_rng_new"></span>

    **Syntax**
    
    ```c
    Rng ee_rng_new(u64 seed);
    ```
    
    **Description**
    
    Initializes a new `Rng` context from a single 64-bit seed.
    
    It uses the `ee_splitmix_u64` algorithm to expand the seed into the 128-bit (2 x `u64`) internal state required by the `xorshift128+` generator.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `seed` | `u64` | The 64-bit value used to initialize the generator's state. |
    
    **Returns**
    
    An initialized `Rng` structure.
    
    **Example**
    
    ```c
    Rng my_rng = ee_rng_new(12345ull);
    u64 random_val = ee_rand_u64(&my_rng);
    ```

## Functions (Uniform Distribution)

These functions generate uniformly distributed numbers.

??? "EE_INLINE u64 ee_rand_u64(Rng* rng)"

    <span id="ee_rand_u64"></span>

    **Syntax**
    
    ```c
    u64 ee_rand_u64(Rng* rng);
    ```
    
    **Description**
    
    Generates the next raw `u64` random value from the `xorshift128+` algorithm and advances the internal state. This is the core generator function.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `rng` | `Rng*` | Pointer to the `Rng` context to be advanced. |
    
    **Returns**
    
    A 64-bit unsigned integer.

??? "EE_INLINE u32 ee_rand_u32(Rng* rng)"

    <span id="ee_rand_u32"></span>

    **Syntax**

    ```c
    u32 ee_rand_u32(Rng* rng);
    ```
    
    **Description**
    
    Generates a `u32` random value by taking the upper 32 bits of a `u64` value from `ee_rand_u64()`.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `rng` | `Rng*` | Pointer to the `Rng` context. |
    
    **Returns**
    
    A 32-bit unsigned integer.

??? "EE_INLINE f64 ee_rand_f64(Rng* rng)"

    <span id="ee_rand_f64"></span>

    **Syntax**
    
    ```c
    f64 ee_rand_f64(Rng* rng);
    ```
    
    **Description**
    
    Generates a `f64` random value in the range `[0.0, 1.0)`.
    
    It uses the upper 53 bits of `ee_rand_u64()` to provide full 53-bit precision for the `f64` mantissa.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `rng` | `Rng*` | Pointer to the `Rng` context. |
    
    **Returns**
    
    A `f64` value within the range `[0.0, 1.0)` (inclusive of 0.0 and exclusive of 1.0).

??? "EE_INLINE f32 ee_rand_f32(Rng* rng)"

    <span id="ee_rand_f32"></span>
    
    **Syntax**

    ```c
    f32 ee_rand_f32(Rng* rng);
    ```
    
    **Description**
    
    Generates a `f32` random value in the range `[0.0, 1.0)`.
    
    It uses the upper 24 bits of `ee_rand_u32()` to provide full 24-bit precision for the `f32` mantissa.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `rng` | `Rng*` | Pointer to the `Rng` context. |
    
    **Returns**

    A `f32` value within the range `[0.0, 1.0)` (inclusive of 0.0 and exclusive of 1.0).

## Functions (Bounded Distribution)

These functions generate uniformly distributed numbers within a specified range.

??? "EE_INLINE u64 ee_rand_u64_b(Rng* rng, u64 bound)"

    <span id="ee_rand_u64_b"></span>
    
    **Syntax**
    
    ```c
    u64 ee_rand_u64_b(Rng* rng, u64 bound);
    ```
    
    **Description**
    
    Generates a `u64` random value in the range `[0, bound)`. The upper bound is exclusive.
    
    It uses a debiasing loop (the "threshold" method) to ensure a uniform distribution, even when `bound` is not a power of two.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `rng` | `Rng*` | Pointer to the `Rng` context. |
    | `bound`| `u64` | The exclusive upper bound. If `0`, the function returns `0`. |
    
    **Returns**
    
    A `u64` value within the range `[0, bound)` (inclusive of 0 and exclusive of `bound`).

??? "EE_INLINE u32 ee_rand_u32_b(Rng* rng, u32 bound)"

    <span id="ee_rand_u32_b"></span>
    
    **Syntax**
    
    ```c
    u32 ee_rand_u32_b(Rng* rng, u32 bound);
    ```
    
    **Description**
    
    Generates a `u32` random value in the range `[0, bound)`. The upper bound is exclusive.
    
    It uses a debiasing loop to ensure a uniform distribution.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `rng` | `Rng*` | Pointer to the `Rng` context. |
    | `bound`| `u32` | The exclusive upper bound. If `0`, the function returns `0`. |
    
    **Returns**
    
    A `u32` value within the range `[0, bound)` (inclusive of 0 and exclusive of `bound`).

??? "EE_INLINE u64 ee_rand_u64_ab(Rng* rng, u64 a, u64 b)"

    <span id="ee_rand_u64_ab"></span>
    
    **Syntax**
    
    ```c
    u64 ee_rand_u64_ab(Rng* rng, u64 a, u64 b);
    ```
    
    **Description**
    
    Generates a `u64` random value in the range `[a, b)`. The upper bound `b` is exclusive.
    
    Asserts if `a > b`. If `a == b`, returns `a`.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `rng` | `Rng*` | Pointer to the `Rng` context. |
    | `a` | `u64` | The inclusive lower bound. |
    | `b` | `u64` | The exclusive upper bound. |
    
    **Returns**
    
    A `u64` value within the range `[a, b)` (inclusive of `a` and exclusive of `b`).

??? "EE_INLINE u32 ee_rand_u32_ab(Rng* rng, u32 a, u32 b)"

    <span id="ee_rand_u32_ab"></span>
    
    **Syntax**
    
    ```c
    u32 ee_rand_u32_ab(Rng* rng, u32 a, u32 b);
    ```
    
    **Description**
    
    Generates a `u32` random value in the range `[a, b)`. The upper bound `b` is exclusive.
    
    Asserts if `a > b`. If `a == b`, returns `a`.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `rng` | `Rng*` | Pointer to the `Rng` context. |
    | `a` | `u32` | The inclusive lower bound. |
    | `b` | `u32` | The exclusive upper bound. |
    
    **Returns**
    
    A `u32` value within the range `[a, b)` (inclusive of `a` and exclusive of `b`).

??? "EE_INLINE f64 ee_rand_f64_ab(Rng* rng, f64 a, f64 b)"

    <span id="ee_rand_f64_ab"></span>
    
    **Syntax**
    
    ```c
    f64 ee_rand_f64_ab(Rng* rng, f64 a, f64 b);
    ```
    
    **Description**
    
    Generates a `f64` random value in the range `[a, b)`. The upper bound `b` is exclusive.
    
    Asserts if `a >= b`.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `rng` | `Rng*` | Pointer to the `Rng` context. |
    | `a` | `f64` | The inclusive lower bound. |
    | `b` | `f64` | The exclusive upper bound. |
    
    **Returns**
    
    A `f64` value within the range `[a, b)` (inclusive of `a` and exclusive of `b`).

??? "EE_INLINE f32 ee_rand_f32_ab(Rng* rng, f32 a, f32 b)"

    <span id="ee_rand_f32_ab"></span>
    
    **Syntax**
    
    ```c
    f32 ee_rand_f32_ab(Rng* rng, f32 a, f32 b);
    ```
    
    **Description**
    
    Generates a `f32` random value in the range `[a, b)`. The upper bound `b` is exclusive.
    
    Asserts if `a >= b`.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `rng` | `Rng*` | Pointer to the `Rng` context. |
    | `a` | `f32` | The inclusive lower bound. |
    | `b` | `f32` | The exclusive upper bound. |
    
    **Returns**
    
    A `f32` value within the range `[a, b)` (inclusive of `a` and exclusive of `b`).

## Functions (Normal Distribution)

??? "EE_INLINE f64 ee_randn_f64(Rng* rng, f64 mean, f64 std)"

<span id="ee_randn_f64"></span>

**Syntax**

```c
f64 ee_randn_f64(Rng* rng, f64 mean, f64 std);
```

**Description**

Generates a normally distributed `f64` random value (also known as a Gaussian random value).

It uses the **Marsaglia polar method** (a variant of the Box-Muller transform) to generate two random values at a time. One value is returned, and the other is cached in the `Rng` context's `spare` field for the next call.

**Parameters**

| Name   | Type   | Description                                                            |
|:-------|:-------|:-----------------------------------------------------------------------|
| `rng`  | `Rng*` | Pointer to the `Rng` context.                                          |
| `mean` | `f64`  | The mean (average) &mu of the distribution.                            |
| `std`  | `f64`  | The standard deviation &sigma of the distribution. Asserts if $\lt 0$. |

**Returns**

A `f64` random value sampled from the specified normal distribution.

??? "EE_INLINE f32 ee_randn_f32(Rng* rng, f32 mean, f32 std)"



## Internal Helper Functions

These functions are used internally by `ee_random.h` and are not typically called directly by the end-user.

??? "EE_INLINE u64 ee_rotl_u64(u64 x, i32 k)"

??? "EE_INLINE u64 ee_splitmix_u64(u64* state)"
