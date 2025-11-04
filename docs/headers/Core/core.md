# **ee_core.h**

`ee_core.h` is the foundational header for the `ee_lib` library. It provides essential definitions that are used by all other modules.

This header includes:

* Core type definitions (`u8`, `u64`, `i32`, etc.).
* The standard `Allocator` interface and its default implementation.
* A SIMD abstraction layer (for AVX/SSE/Fallback).
* Core utility macros (`EE_ASSERT`, `EE_INLINE`, memory sizes, alignment).
* Low-level bitwise and math helper functions.

## Type Definitions

### Core Types

`ee_core.h` provides a set of fixed-size integer and floating-point types for cross-platform consistency.

```c
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

typedef float       f32;
typedef double      f64;
typedef long double f80;
```

## Allocator Interface

<span id="allocator"></span>

Defines a generic interface for memory allocation. Modules like [`ee_arena`](../Arena/arena.md) and [`ee_array`](../Array/array.md) use this structure to manage their memory, allowing users to provide custom allocation strategies.

```c
typedef struct Allocator
{
    void* (*alloc_fn)(struct Allocator* self, size_t size);
    void* (*realloc_fn)(struct Allocator* self, void* buffer, size_t old_size, size_t new_size);
    void  (*free_fn)(struct Allocator* self, void* buffer);
    void* context;
} Allocator;
```

??? "Structure Members"

    | Member       | Type             | Description                                                                                                                                      |
    |--------------|------------------|--------------------------------------------------------------------------------------------------------------------------------------------------|
    | `alloc_fn`   | `void* (*)(...)` | Function pointer for allocating a new block of memory of `size` bytes.                                                                           |
    | `realloc_fn` | `void* (*)(...)` | Function pointer for resizing an existing `buffer` from `old_size` to `new_size`.                                                                |
    | `free_fn`    | `void (*)(...)`  | Function pointer for freeing a `buffer`.                                                                                                         |
    | `context`    | `void*`          | An optional, implementation-defined pointer. It can be used to store state. For example, `ee_arena` stores a pointer to its `Arena` object here. |

## Macros and Defines

This header provides a large set of macros for assertions, utilities, platform abstraction, and SIMD.

### Assertion and Logging
<span id="macros-assert"></span>

??? "EE_ASSERT(cond, fmt, ...)"

    <span id="ee_assert"></span>

    **Syntax**
    ```c
    #define EE_ASSERT(cond, fmt, ...) do { ... } while(0)
    ```
    
    **Description**
    
    The primary assertion macro for `ee_lib`. Checks if `cond` is true.
    
    If `cond` is false, this macro will print a formatted error message (`fmt`, ...) to `stderr`, including the file, line number, and function name where the assertion failed. It will then terminate the program by calling `exit(1)`.
    
    **Disabling Assertions**
    
    If the `EE_NO_ASSERT` macro is defined *before* including `ee_core.h`, all `EE_ASSERT` calls will be compiled out to `((void)0)`, effectively disabling assertions for a production/release build.
    
    **Example**
    ```c
    void* my_func(void* ptr) 
    {
        EE_ASSERT(ptr != NULL, "Input pointer was NULL!");
        // ... function continues if ptr is not NULL ...
    }
    ```

| Macro                  | Description                                                                                                   |
|:-----------------------|:--------------------------------------------------------------------------------------------------------------|
| `EE_NO_ASSERT`         | If defined *before* `ee_core.h` is included, this disables all `EE_ASSERT` macros, compiling them to nothing. |
| `EE_PRINT(fmt, ...)`   | A simple wrapper for `fprintf(stdout, fmt, ...)` without appending a newline.                                 |
| `EE_PRINTLN(fmt, ...)` | A simple wrapper for `fprintf(stdout, fmt "\n", ...)` that automatically appends a newline.                   |

### Memory Size Constants
<span id="macros-mem"></span>

Constants for common memory size calculations.

| Macro       | Value              | Description                          |
|:------------|:-------------------|:-------------------------------------|
| `EE_KB`     | `(1 << 10)`        | One Kilobyte (1,024 bytes).          |
| `EE_MB`     | `(1 << 20)`        | One Megabyte.                        |
| `EE_GB`     | `(1 << 30)`        | One Gigabyte.                        |
| `EE_TB`     | `(1ull << 40)`     | One Terabyte (as a `u64`).           |
| `EE_NKB(n)` | `(n * EE_KB)`      | A helper macro to get `n` Kilobytes. |
| `EE_NMB(n)` | `(n * EE_MB)`      | A helper macro to get `n` Megabytes. |
| `EE_NGB(n)` | `(n * EE_GB)`      | A helper macro to get `n` Gigabytes. |
| `EE_NTB(n)` | `((u64)n * EE_TB)` | A helper macro to get `n` Terabytes. |

### Platform and Compiler Abstractions
<span id="macros-platform"></span>

| Macro               | Description                                                                                                                      |
|:--------------------|:---------------------------------------------------------------------------------------------------------------------------------|
| `EE_INLINE`         | Defines the inline keyword for the library, typically `static inline`.                                                           |
| `EE_EXTERN_C_START` | Defines `extern "C" {` if compiled as C++, otherwise defines nothing. Used for C++/C compatibility.                              |
| `EE_EXTERN_C_END`   | Defines `}` if compiled as C++, otherwise defines nothing.                                                                       |
| `EE_ALIGNOF(x)`     | A compiler-agnostic wrapper for `alignof(x)` (C11) or a fallback to `EE_MAX_ALIGN`.                                              |
| `EE_ALIGNAS(x)`     | A compiler-agnostic wrapper for `alignas(x)` (C11), `__attribute__((aligned(x)))` (GCC/Clang), or `__declspec(align(x))` (MSVC). |
| `EE_ALLOCA(size)`   | A compiler-agnostic wrapper for `alloca(size)` (stack allocation). May map to `_malloca(size)` on MSVC.                          |
| `EE_FREEA(ptr)`     | The corresponding "free" call for `EE_ALLOCA`. Required for `_malloca` on MSVC; does nothing on other platforms.                 |

### General Utilities
<span id="macros-util"></span>

| Macro                       | Description                                                                                                       |
|:----------------------------|:------------------------------------------------------------------------------------------------------------------|
| `EE_TRUE` / `EE_FALSE`      | Defines `(1)` and `(0)` for boolean logic.                                                                        |
| `EE_FIND_FIRST_BIT_INVALID` | A sentinel value (typically `32`) returned by [`ee_first_bit_u32()`](#ee_first_bit_u32) when no set bit is found. |
| `EE_MAX_ALIGN`              | Defines the default maximum alignment for the library, typically `16` bytes.                                      |
| `EE_ALIGN_MASK`             | A bitmask (`~(EE_MAX_ALIGN - 1)`) used for alignment calculations.                                                |
| `EE_RECAST_U8(x)`           | Helper macro to safely cast a variable `x` to a `u8*` pointer: `((u8*)(&(x)))`.                                   |
| `EE_RECAST_U8_PTR(x)`       | Helper macro to safely cast a pointer `x` to a `u8**` pointer: `((u8**)(&(x)))`.                                  |
| `EE_UNUSED(...)`            | A variadic macro to suppress "unused parameter" warnings from the compiler. It supports 1 to 5 arguments.         |

### SIMD Abstraction
<span id="macros-simd"></span>

`ee_core.h` provides an abstraction layer to normalize SIMD operations. The behavior is controlled by pre-defining `EE_SIMD_EFFECTIVE_MAX_LEVEL`.

| Macro                                 | Description                                                                                                                                                                                                                                  |
|:--------------------------------------|:---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `EE_SIMD_LEVEL_NONE`                  | Constant `(0)`. Represents no SIMD, using a C fallback.                                                                                                                                                                                      |
| `EE_SIMD_LEVEL_SSE`                   | Constant `(1)`. Represents using SSE instructions (128-bit).                                                                                                                                                                                 |
| `EE_SIMD_LEVEL_AVX`                   | Constant `(2)`. Represents using AVX instructions (256-bit).                                                                                                                                                                                 |
| `EE_SIMD_MAX_LEVEL`                   | The highest SIMD level supported by the library (defaults to `EE_SIMD_LEVEL_AVX`).                                                                                                                                                           |
| `EE_SIMD_EFFECTIVE_MAX_LEVEL`         | **(User-configurable)** This controls which SIMD implementation is compiled. Defaults to `EE_SIMD_MAX_LEVEL` but can be manually defined by the user to force a lower level (e.g., `#define EE_SIMD_EFFECTIVE_MAX_LEVEL EE_SIMD_LEVEL_SSE`). |
| `ee_loadu_si`, `ee_cmpeq_epi8`, ...   | A large set of SIMD-wrapper macros that map to `_mm256_...`, `_mm_...`, or C fallback functions based on `EE_SIMD_EFFECTIVE_MAX_LEVEL`. (See the "SIMD Abstraction Layer" section for full details).                                         |
| `eed_loadu_si`, `eed_cmpeq_epi8`, ... | A *second, independent* set of SIMD wrappers for the `ee_dictionary` module, controlled by `EE_SIMD_DICT_MAX_LEVEL`.                                                                                                                         |

### Code Generation Macros
<span id="macros-gen"></span>

These macros are used internally by `ee_core.h` to generate multiple fixed-size versions of memory functions.

| Macro                        | Description                                                                                       |
|:-----------------------------|:--------------------------------------------------------------------------------------------------|
| `EE_DEFINE_EQ_FN_SAFE(size)` | Generates a **safe** (aliasing-safe, `memcpy`-based) equality function named `ee_eq_safe_##size`. |
| `EE_DEFINE_EQ_FN(size)`      | Generates an **unsafe** (type-punning) equality function named `ee_eq_##size`.                    |
| `EE_DEFINE_CPY_FN(size)`     | Generates an **unsafe** (type-punning) copy function named `ee_cpy_##size`.                       |

## SIMD Abstraction Layer

<span id="simd"></span>

`ee_core.h` provides an abstraction layer to normalize SIMD (Single Instruction, Multiple Data) operations across different instruction sets (AVX2, SSE2) and a C fallback.

The effective SIMD level is controlled by defining `EE_SIMD_EFFECTIVE_MAX_LEVEL` to one of the following macros before including `ee_core.h`.

| Macro                         | Value                             | Description                                          |
|:------------------------------|:----------------------------------|:-----------------------------------------------------|
| `EE_SIMD_LEVEL_NONE`          | `(0)`                             | Force C fallback (no SIMD).                          |
| `EE_SIMD_LEVEL_SSE`           | `(1)`                             | Use SSE instructions (128-bit).                      |
| `EE_SIMD_LEVEL_AVX`           | `(2)`                             | Use AVX instructions (256-bit).                      |
| `EE_SIMD_MAX_LEVEL`           | `(EE_SIMD_LEVEL_AVX)`             | The highest SIMD level supported by the library.     |
| `EE_SIMD_EFFECTIVE_MAX_LEVEL` | (Defaults to `EE_SIMD_MAX_LEVEL`) | This controls which SIMD implementation is compiled. |

### SIMD Types

| Type        | ,AVX (256-bit),, | SSE (128-bit) | None (Fallback) |
|:------------|:-----------------|:--------------|:----------------|
| `ee_simd_i` | `__m256i`        | `__m128i`     | `u64`           |
| `ee_simd_f` | `__m256`         | `__m128`      | `f64`           |
| `ee_simd_d` | `__m256d`        | `__m128d`     | `f64`           |

### SIMD Macros (`ee_...`)

These macros map to the correct intrinsic (e.g., `_mm256_...`, `_mm_...`) or C function (`_ee_...`) based on the compiled SIMD level.

| Macro                                                               | ,Description                                                                      |
|:--------------------------------------------------------------------|:----------------------------------------------------------------------------------|
| EE_SIMD_BYTES                                                       | ,"The size of the SIMD register in bytes (e.g., 32 for AVX, 16 for SSE)."         |
| ee_loadu_si                                                         | ,Loads unaligned data into a ee_simd_i register.                                  |
| ee_load_si                                                          | ,Loads aligned data into a ee_simd_i register.                                    |
| ee_store_si                                                         | ,Stores data from a ee_simd_i register to an aligned address.                     |
| ee_set1_epi8/16/32/64                                               | ,"Broadcasts a single value (8, 16, 32, or 64-bit) to all lanes of the register." |
| ee_cmpeq_epi8/16/32/64                                              | ,Performs a per-lane comparison for equality.                                     |
| ee_movemask_epi8                                                    | ,Creates a bitmask from the most significant bit of each 8-bit lane.              |
| ee_setzero_si                                                       | ,Returns a ee_simd_i register filled with zeros.                                  |
| ee_or_si / ee_xor_si / ee_and_si                                    | ,"Per-lane bitwise OR, XOR, and AND operations."                                  |
| ee_srl_epi... / ee_sll_epi...                                       | ,Per-lane bitwise shift right/left.                                               |
| ee_prefetch                                                         | ,Prefetches data into the cache.                                                  |
| (...etc.),(The header defines many other standard SIMD operations.) |                                                                                   |    