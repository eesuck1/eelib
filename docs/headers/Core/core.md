[//]: # (# **ee_core.h**)

[//]: # ()
[//]: # (`ee_core.h` is the foundational header for the `eelib` library. It provides essential definitions that are used by all other modules.)

[//]: # ()
[//]: # (This header includes:)

[//]: # ()
[//]: # (* Core type definitions &#40;`u8`, `u64`, `i32`, etc.&#41;.)

[//]: # (* The standard `Allocator` interface and its default implementation.)

[//]: # (* A SIMD abstraction layer &#40;for AVX/SSE/Fallback&#41;.)

[//]: # (* Core utility macros &#40;`EE_ASSERT`, `EE_INLINE`, memory sizes, alignment&#41;.)

[//]: # (* Low-level bitwise and math helper functions.)

[//]: # ()
[//]: # (## Type Definitions)

[//]: # ()
[//]: # (### Core Types)

[//]: # ()
[//]: # (`ee_core.h` provides a set of fixed-size integer and floating-point types for cross-platform consistency.)

[//]: # ()
[//]: # (```c)

[//]: # (typedef uint8_t     u8;)

[//]: # (typedef uint16_t    u16;)

[//]: # (typedef uint32_t    u32;)

[//]: # (typedef uint64_t    u64;)

[//]: # ()
[//]: # (typedef int8_t      i8;)

[//]: # (typedef int16_t     i16;)

[//]: # (typedef int32_t     i32;)

[//]: # (typedef int64_t     i64;)

[//]: # ()
[//]: # (typedef float       f32;)

[//]: # (typedef double      f64;)

[//]: # (typedef long double f80;)

[//]: # (```)

[//]: # ()
[//]: # (## Allocator Interface)

[//]: # ()
[//]: # (<span id="allocator"></span>)

[//]: # ()
[//]: # (Defines a generic interface for memory allocation. Modules like [`ee_arena`]&#40;../Arena/arena.md&#41; and [`ee_array`]&#40;../Array/array.md&#41; use this structure to manage their memory, allowing users to provide custom allocation strategies.)

[//]: # ()
[//]: # (```c)

[//]: # (typedef struct Allocator)

[//]: # ({)

[//]: # (    void* &#40;*alloc_fn&#41;&#40;struct Allocator* self, size_t size&#41;;)

[//]: # (    void* &#40;*realloc_fn&#41;&#40;struct Allocator* self, void* buffer, size_t old_size, size_t new_size&#41;;)

[//]: # (    void  &#40;*free_fn&#41;&#40;struct Allocator* self, void* buffer&#41;;)

[//]: # (    void* context;)

[//]: # (} Allocator;)

[//]: # (```)

[//]: # ()
[//]: # (??? "Structure Members")

[//]: # ()
[//]: # (    | Member       | Type             | Description                                                                                                                                      |)

[//]: # (    |--------------|------------------|--------------------------------------------------------------------------------------------------------------------------------------------------|)

[//]: # (    | `alloc_fn`   | `void* &#40;*&#41;&#40;...&#41;` | Function pointer for allocating a new block of memory of `size` bytes.                                                                           |)

[//]: # (    | `realloc_fn` | `void* &#40;*&#41;&#40;...&#41;` | Function pointer for resizing an existing `buffer` from `old_size` to `new_size`.                                                                |)

[//]: # (    | `free_fn`    | `void &#40;*&#41;&#40;...&#41;`  | Function pointer for freeing a `buffer`.                                                                                                         |)

[//]: # (    | `context`    | `void*`          | An optional, implementation-defined pointer. It can be used to store state. For example, `ee_arena` stores a pointer to its `Arena` object here. |)

[//]: # ()
[//]: # (## Macros and Defines)

[//]: # ()
[//]: # (This header provides a large set of macros for assertions, utilities, platform abstraction, and SIMD.)

[//]: # ()
[//]: # (### Assertion and Logging)

[//]: # (<span id="macros-assert"></span>)

[//]: # ()
[//]: # (??? "EE_ASSERT&#40;cond, fmt, ...&#41;")

[//]: # ()
[//]: # (    <span id="ee_assert"></span>)

[//]: # ()
[//]: # (    **Syntax**)

[//]: # (    ```c)

[//]: # (    #define EE_ASSERT&#40;cond, fmt, ...&#41; do { ... } while&#40;0&#41;)

[//]: # (    ```)

[//]: # (    )
[//]: # (    **Description**)

[//]: # (    )
[//]: # (    The primary assertion macro for `eelib`. Checks if `cond` is true.)

[//]: # (    )
[//]: # (    If `cond` is false, this macro will print a formatted error message &#40;`fmt`, ...&#41; to `stderr`, including the file, line number, and function name where the assertion failed. It will then terminate the program by calling `exit&#40;1&#41;`.)

[//]: # (    )
[//]: # (    **Disabling Assertions**)

[//]: # (    )
[//]: # (    If the `EE_NO_ASSERT` macro is defined *before* including `ee_core.h`, all `EE_ASSERT` calls will be compiled out to `&#40;&#40;void&#41;0&#41;`, effectively disabling assertions for a production/release build.)

[//]: # (    )
[//]: # (    **Example**)

[//]: # (    ```c)

[//]: # (    void* my_func&#40;void* ptr&#41; )

[//]: # (    {)

[//]: # (        EE_ASSERT&#40;ptr != NULL, "Input pointer was NULL!"&#41;;)

[//]: # (        // ... function continues if ptr is not NULL ...)

[//]: # (    })

[//]: # (    ```)

[//]: # ()
[//]: # (| Macro                  | Description                                                                                                   |)

[//]: # (|:-----------------------|:--------------------------------------------------------------------------------------------------------------|)

[//]: # (| `EE_NO_ASSERT`         | If defined *before* `ee_core.h` is included, this disables all `EE_ASSERT` macros, compiling them to nothing. |)

[//]: # (| `EE_PRINT&#40;fmt, ...&#41;`   | A simple wrapper for `fprintf&#40;stdout, fmt, ...&#41;` without appending a newline.                                 |)

[//]: # (| `EE_PRINTLN&#40;fmt, ...&#41;` | A simple wrapper for `fprintf&#40;stdout, fmt "\n", ...&#41;` that automatically appends a newline.                   |)

[//]: # ()
[//]: # (### Memory Size Constants)

[//]: # (<span id="macros-mem"></span>)

[//]: # ()
[//]: # (Constants for common memory size calculations.)

[//]: # ()
[//]: # (| Macro       | Value              | Description                          |)

[//]: # (|:------------|:-------------------|:-------------------------------------|)

[//]: # (| `EE_KB`     | `&#40;1 << 10&#41;`        | One Kilobyte &#40;1,024 bytes&#41;.          |)

[//]: # (| `EE_MB`     | `&#40;1 << 20&#41;`        | One Megabyte.                        |)

[//]: # (| `EE_GB`     | `&#40;1 << 30&#41;`        | One Gigabyte.                        |)

[//]: # (| `EE_TB`     | `&#40;1ull << 40&#41;`     | One Terabyte &#40;as a `u64`&#41;.           |)

[//]: # (| `EE_NKB&#40;n&#41;` | `&#40;n * EE_KB&#41;`      | A helper macro to get `n` Kilobytes. |)

[//]: # (| `EE_NMB&#40;n&#41;` | `&#40;n * EE_MB&#41;`      | A helper macro to get `n` Megabytes. |)

[//]: # (| `EE_NGB&#40;n&#41;` | `&#40;n * EE_GB&#41;`      | A helper macro to get `n` Gigabytes. |)

[//]: # (| `EE_NTB&#40;n&#41;` | `&#40;&#40;u64&#41;n * EE_TB&#41;` | A helper macro to get `n` Terabytes. |)

[//]: # ()
[//]: # (### Platform and Compiler Abstractions)

[//]: # (<span id="macros-platform"></span>)

[//]: # ()
[//]: # (| Macro               | Description                                                                                                                      |)

[//]: # (|:--------------------|:---------------------------------------------------------------------------------------------------------------------------------|)

[//]: # (| `EE_INLINE`         | Defines the inline keyword for the library, typically `static inline`.                                                           |)

[//]: # (| `EE_EXTERN_C_START` | Defines `extern "C" {` if compiled as C++, otherwise defines nothing. Used for C++/C compatibility.                              |)

[//]: # (| `EE_EXTERN_C_END`   | Defines `}` if compiled as C++, otherwise defines nothing.                                                                       |)

[//]: # (| `EE_ALIGNOF&#40;x&#41;`     | A compiler-agnostic wrapper for `alignof&#40;x&#41;` &#40;C11&#41; or a fallback to `EE_MAX_ALIGN`.                                              |)

[//]: # (| `EE_ALIGNAS&#40;x&#41;`     | A compiler-agnostic wrapper for `alignas&#40;x&#41;` &#40;C11&#41;, `__attribute__&#40;&#40;aligned&#40;x&#41;&#41;&#41;` &#40;GCC/Clang&#41;, or `__declspec&#40;align&#40;x&#41;&#41;` &#40;MSVC&#41;. |)

[//]: # (| `EE_ALLOCA&#40;size&#41;`   | A compiler-agnostic wrapper for `alloca&#40;size&#41;` &#40;stack allocation&#41;. May map to `_malloca&#40;size&#41;` on MSVC.                          |)

[//]: # (| `EE_FREEA&#40;ptr&#41;`     | The corresponding "free" call for `EE_ALLOCA`. Required for `_malloca` on MSVC; does nothing on other platforms.                 |)

[//]: # ()
[//]: # (### General Utilities)

[//]: # (<span id="macros-util"></span>)

[//]: # ()
[//]: # (| Macro                       | Description                                                                                                       |)

[//]: # (|:----------------------------|:------------------------------------------------------------------------------------------------------------------|)

[//]: # (| `EE_TRUE` / `EE_FALSE`      | Defines `&#40;1&#41;` and `&#40;0&#41;` for boolean logic.                                                                        |)

[//]: # (| `EE_FIND_FIRST_BIT_INVALID` | A sentinel value &#40;typically `32`&#41; returned by [`ee_first_bit_u32&#40;&#41;`]&#40;#ee_first_bit_u32&#41; when no set bit is found. |)

[//]: # (| `EE_MAX_ALIGN`              | Defines the default maximum alignment for the library, typically `16` bytes.                                      |)

[//]: # (| `EE_ALIGN_MASK`             | A bitmask &#40;`~&#40;EE_MAX_ALIGN - 1&#41;`&#41; used for alignment calculations.                                                |)

[//]: # (| `EE_RECAST_U8&#40;x&#41;`           | Helper macro to safely cast a variable `x` to a `u8*` pointer: `&#40;&#40;u8*&#41;&#40;&&#40;x&#41;&#41;&#41;`.                                   |)

[//]: # (| `EE_RECAST_U8_PTR&#40;x&#41;`       | Helper macro to safely cast a pointer `x` to a `u8**` pointer: `&#40;&#40;u8**&#41;&#40;&&#40;x&#41;&#41;&#41;`.                                  |)

[//]: # (| `EE_UNUSED&#40;...&#41;`            | A variadic macro to suppress "unused parameter" warnings from the compiler. It supports 1 to 5 arguments.         |)

[//]: # ()
[//]: # (### SIMD Abstraction)

[//]: # (<span id="macros-simd"></span>)

[//]: # ()
[//]: # (`ee_core.h` provides an abstraction layer to normalize SIMD operations. The behavior is controlled by pre-defining `EE_SIMD_EFFECTIVE_MAX_LEVEL`.)

[//]: # ()
[//]: # (| Macro                                 | Description                                                                                                                                                                                                                                  |)

[//]: # (|:--------------------------------------|:---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|)

[//]: # (| `EE_SIMD_LEVEL_NONE`                  | Constant `&#40;0&#41;`. Represents no SIMD, using a C fallback.                                                                                                                                                                                      |)

[//]: # (| `EE_SIMD_LEVEL_SSE`                   | Constant `&#40;1&#41;`. Represents using SSE instructions &#40;128-bit&#41;.                                                                                                                                                                                 |)

[//]: # (| `EE_SIMD_LEVEL_AVX`                   | Constant `&#40;2&#41;`. Represents using AVX instructions &#40;256-bit&#41;.                                                                                                                                                                                 |)

[//]: # (| `EE_SIMD_MAX_LEVEL`                   | The highest SIMD level supported by the library &#40;defaults to `EE_SIMD_LEVEL_AVX`&#41;.                                                                                                                                                           |)

[//]: # (| `EE_SIMD_EFFECTIVE_MAX_LEVEL`         | **&#40;User-configurable&#41;** This controls which SIMD implementation is compiled. Defaults to `EE_SIMD_MAX_LEVEL` but can be manually defined by the user to force a lower level &#40;e.g., `#define EE_SIMD_EFFECTIVE_MAX_LEVEL EE_SIMD_LEVEL_SSE`&#41;. |)

[//]: # (| `ee_loadu_si`, `ee_cmpeq_epi8`, ...   | A large set of SIMD-wrapper macros that map to `_mm256_...`, `_mm_...`, or C fallback functions based on `EE_SIMD_EFFECTIVE_MAX_LEVEL`. &#40;See the "SIMD Abstraction Layer" section for full details&#41;.                                         |)

[//]: # (| `eed_loadu_si`, `eed_cmpeq_epi8`, ... | A *second, independent* set of SIMD wrappers for the `ee_dictionary` module, controlled by `EE_SIMD_DICT_MAX_LEVEL`.                                                                                                                         |)

[//]: # ()
[//]: # (### Code Generation Macros)

[//]: # (<span id="macros-gen"></span>)

[//]: # ()
[//]: # (These macros are used internally by `ee_core.h` to generate multiple fixed-size versions of memory functions.)

[//]: # ()
[//]: # (| Macro                        | Description                                                                                       |)

[//]: # (|:-----------------------------|:--------------------------------------------------------------------------------------------------|)

[//]: # (| `EE_DEFINE_EQ_FN_SAFE&#40;size&#41;` | Generates a **safe** &#40;aliasing-safe, `memcpy`-based&#41; equality function named `ee_eq_safe_##size`. |)

[//]: # (| `EE_DEFINE_EQ_FN&#40;size&#41;`      | Generates an **unsafe** &#40;type-punning&#41; equality function named `ee_eq_##size`.                    |)

[//]: # (| `EE_DEFINE_CPY_FN&#40;size&#41;`     | Generates an **unsafe** &#40;type-punning&#41; copy function named `ee_cpy_##size`.                       |)

[//]: # ()
[//]: # (## SIMD Abstraction Layer)

[//]: # ()
[//]: # (<span id="simd"></span>)

[//]: # ()
[//]: # (`ee_core.h` provides an abstraction layer to normalize SIMD &#40;Single Instruction, Multiple Data&#41; operations across different instruction sets &#40;AVX2, SSE2&#41; and a C fallback.)

[//]: # ()
[//]: # (The effective SIMD level is controlled by defining `EE_SIMD_EFFECTIVE_MAX_LEVEL` to one of the following macros before including `ee_core.h`.)

[//]: # ()
[//]: # (| Macro                         | Value                             | Description                                          |)

[//]: # (|:------------------------------|:----------------------------------|:-----------------------------------------------------|)

[//]: # (| `EE_SIMD_LEVEL_NONE`          | `&#40;0&#41;`                             | Force C fallback &#40;no SIMD&#41;.                          |)

[//]: # (| `EE_SIMD_LEVEL_SSE`           | `&#40;1&#41;`                             | Use SSE instructions &#40;128-bit&#41;.                      |)

[//]: # (| `EE_SIMD_LEVEL_AVX`           | `&#40;2&#41;`                             | Use AVX instructions &#40;256-bit&#41;.                      |)

[//]: # (| `EE_SIMD_MAX_LEVEL`           | `&#40;EE_SIMD_LEVEL_AVX&#41;`             | The highest SIMD level supported by the library.     |)

[//]: # (| `EE_SIMD_EFFECTIVE_MAX_LEVEL` | &#40;Defaults to `EE_SIMD_MAX_LEVEL`&#41; | This controls which SIMD implementation is compiled. |)

[//]: # ()
[//]: # (### SIMD Types)

[//]: # ()
[//]: # (| Type        | ,AVX &#40;256-bit&#41;,, | SSE &#40;128-bit&#41; | None &#40;Fallback&#41; |)

[//]: # (|:------------|:-----------------|:--------------|:----------------|)

[//]: # (| `ee_simd_i` | `__m256i`        | `__m128i`     | `u64`           |)

[//]: # (| `ee_simd_f` | `__m256`         | `__m128`      | `f64`           |)

[//]: # (| `ee_simd_d` | `__m256d`        | `__m128d`     | `f64`           |)

[//]: # ()
[//]: # (### SIMD Macros &#40;`ee_...`&#41;)

[//]: # ()
[//]: # (These macros map to the correct intrinsic &#40;e.g., `_mm256_...`, `_mm_...`&#41; or C function &#40;`_ee_...`&#41; based on the compiled SIMD level.)

[//]: # ()
[//]: # (| Macro                                                               | Description                                                                      |)

[//]: # (|:--------------------------------------------------------------------|:---------------------------------------------------------------------------------|)

[//]: # (| EE_SIMD_BYTES                                                       | "The size of the SIMD register in bytes &#40;e.g., 32 for AVX, 16 for SSE&#41;."         |)

[//]: # (| ee_loadu_si                                                         | Loads unaligned data into a ee_simd_i register.                                  |)

[//]: # (| ee_load_si                                                          | Loads aligned data into a ee_simd_i register.                                    |)

[//]: # (| ee_store_si                                                         | Stores data from a ee_simd_i register to an aligned address.                     |)

[//]: # (| ee_set1_epi8/16/32/64                                               | "Broadcasts a single value &#40;8, 16, 32, or 64-bit&#41; to all lanes of the register." |)

[//]: # (| ee_cmpeq_epi8/16/32/64                                              | Performs a per-lane comparison for equality.                                     |)

[//]: # (| ee_movemask_epi8                                                    | Creates a bitmask from the most significant bit of each 8-bit lane.              |)

[//]: # (| ee_setzero_si                                                       | Returns a ee_simd_i register filled with zeros.                                  |)

[//]: # (| ee_or_si / ee_xor_si / ee_and_si                                    | "Per-lane bitwise OR, XOR, and AND operations."                                  |)

[//]: # (| ee_srl_epi... / ee_sll_epi...                                       | Per-lane bitwise shift right/left.                                               |)

[//]: # (| ee_prefetch                                                         | Prefetches data into the cache.                                                  |)

[//]: # (| &#40;...etc.&#41;,&#40;The header defines many other standard SIMD operations.&#41; |                                                                                  |    )