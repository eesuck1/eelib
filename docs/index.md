# ee — Lightweight C Utility Library
![Version](https://img.shields.io/badge/Version-0.0.1-3DA9FC)
![License: MIT](https://img.shields.io/badge/License-MIT-264653)
![C Standard](https://img.shields.io/badge/C-C99-F4A261)

**ee** is a header-only C library providing common data structures and utilities.

It includes implementations for arrays, dictionaries, strings, memory management, and other structures. The library is designed to be used directly in C projects without external dependencies.

## Features
The library provides a set of building blocks:

- **Memory management**
    - Arena allocator (`ee_arena.h`) for fast, linear allocations.
    - Optional custom allocators via `ee_core.h`.

- **Dynamic containers**
    - Resizable vectors (`ee_array.h`) with automatic growth.
    - Hash maps (`ee_dict.h`) with open addressing.
    - Binary heaps (`ee_heap.h`) for priority scheduling.
    - Sets (`ee_set.h`) with efficient lookup.
    - 2D grids (`ee_grid.h`) for spatial or game-related logic.

- **String utilities**
    - `ee_string.h` provides utilities for dynamic strings, fixed-length buffers, and lightweight string views.

- **System utilities**
    - `ee_fs.h` for filesystem traversal and file utilities (Windows-only).
    - `ee_random.h` for uniform and normal random distributions.

## Platforms and Compilers

* **Minimum C Standard:** C99
* **Tested Compilers:** MSVC, GCC

## Installation

There are two ways to use **ee** in your project:

#### Option 1: Clone the Repository

Clone or download the repository and add the `utils/` directory to your include path.
This way you get access to all modules at once:

```bash
git clone https://github.com/eesuck1/eelib.git
```

Then in your code:

```c
#include "ee_array.h"
#include "ee_dict.h"
#include "ee_string.h"
#include "ee_arena.h"
```

#### Option 2: Copy Individual Headers

Since **ee** is header-only, you can also copy a single header (or a subset) into your project.

!!! note "Note"  
    Some headers depend on others. Make sure you copy all required files.

| Header                                                                          | Description                                                             | Dependencies                                                                                                                                                                                                                                        |
|---------------------------------------------------------------------------------|-------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`ee_arena.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_arena.h)   | Provides a linear memory allocator (arena).                             | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h)   | Provides a dynamic, resizable array (vector).                           | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h)     | Defines core types, macros, SIMD abstractions, and base allocators.     | Independent.                                                                                                                                                                                                                                        |
| [`ee_dict.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_dict.h)     | Provides an open-addressing hash map.                                   | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_random.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_random.h) | Provides PRNG for uniform and normal distributions.                     | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_string.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_string.h) | Provides dynamic strings, fixed-buffers, and string views.              | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |

## Disabling Assertions (`EE_NO_ASSERT`)

Defining `EE_NO_ASSERT` before including header disables all `EE_ASSERT()` checks at compile time.

| Mode                             | Description                                                 |
|:---------------------------------|:------------------------------------------------------------|
| With `EE_ASSERT` <br/> (default) | `EE_ASSERT` validates conditions and terminates on failure. |
| With `EE_NO_ASSERT`              | Assertions are removed — no checks or runtime overhead.     |

**Usage example**:

```c
#define EE_NO_ASSERT

#include "ee_core.h"
```

Use this to exclude safety checks in release builds for better performance.

## **SIMD Levels**

SIMD levels define the width and type of vector instructions supported by the processor.

The library supports multiple SIMD optimization levels:

| Macro                             | Value | Description                                                  |
|:----------------------------------|:------|:-------------------------------------------------------------|
| `EE_SIMD_LEVEL_NONE`              | 0     | No SIMD instructions; operations are scalar.                 |
| `EE_SIMD_LEVEL_SSE`               | 1     | Supports 128-bit vector instructions for ints and floats.    |
| `EE_SIMD_LEVEL_AVX`<br/>(default) | 2     | Supports 256-bit vector instructions for higher parallelism. |

## **Roadmap**

We are actively developing `ee_lib` and plan to release additional modules in the future.
Work is currently in progress for:

* Dynamic Heap (`ee_heap`)
* File System Utilities (`ee_fs`)
* Double-Ended Queue (`ee_deq`)
* ...and more.

These modules will be added to the documentation as soon as they are stable and ready for production use.


---
extra:
page_class: glossary-page
---

# Glossary

This page defines key terms and concepts used throughout the `ee_lib` library. Use the search bar to find a specific term, or browse by category using the Table of Contents on the right.

---

## Core Concepts
<span id="glossary-core"></span>

Core ideas and types that are used by all other modules.

### Allocator (Interface)
<span id="glossary-allocator"></span>
The standard `ee_lib` interface (defined in [`ee_core.h`](headers/Core/core.md#allocator-interface)) that provides a generic API for memory operations (`alloc_fn`, `realloc_fn`, `free_fn`).

### BinCmp
<span id="glossary-bincmp"></span>
A function pointer type (`int (*)(const void* a, const void* b)`) used for generic comparison operations, such as sorting. It must return a negative, zero, or positive value based on the comparison.

### Type-punning
<span id="glossary-type-punning"></span>
An operation (e.g., `*(u64*)ptr`) that reinterprets a block of memory as a different type. `ee_lib` provides safe (`ee_eq_safe_...`) and unsafe (`ee_eq_...`) versions of these operations.

---

## Data Structures
<span id="glossary-datastructures"></span>

The main data structure modules provided by `ee_lib`.

### Arena
<span id="glossary-arena"></span>
A **Linear Allocator** that allocates memory sequentially from a single, contiguous block. Allocations are fast, but individual blocks cannot be freed.

### Array (Dynamic)
<span id="glossary-array"></span>
A type-generic dynamic array (vector) that manages a contiguous, resizable memory buffer. It operates on raw bytes by tracking an `elem_size`.

---

## Memory Management
<span id="glossary-memory"></span>

Terms related to how memory is allocated, aligned, and freed.

### Alignment
<span id="glossary-alignment"></span>
The requirement that a memory address must be a multiple of a certain value (e.g., 16 bytes, defined by `EE_MAX_ALIGN`).

### `base` (Arena)
<span id="glossary-base"></span>
A pointer (`u8*`) within the `Arena` struct that points to the **raw**, **unaligned** memory block returned by the underlying **Allocator**. This is the pointer that must be used to free the entire arena's memory.

### `buffer` (Arena)
<span id="glossary-buffer"></span>
A pointer (`u8*`) within the `Arena` struct that points to the **aligned**, **usable** start of the memory block from which allocations are made. This pointer is derived from `base` but adjusted for alignment.

### `EE_NO_REWIND`
<span id="glossary-no-rewind"></span>
A constant (defined as `0`) used as the `rewind_depth` parameter in `ee_arena_new()`. It signifies that the arena should not allocate a **Mark** stack, thus disabling the `ee_arena_mark()` and `ee_arena_rewind()` features for that instance.

### Free (Arena)
<span id="glossary-free"></span>
In the context of an arena, this term refers to deallocating the _entire_ memory block used by the arena. This is done by calling `ee_arena_free()`, which uses the underlying **Allocator** to free the `base` pointer. It does _not_ refer to freeing individual allocations.

### Linear Allocation
<span id="glossary-linear-allocation"></span>
The strategy used by an **Arena**. Memory is allocated by simply advancing an "offset" pointer (or "watermark") in a single buffer.

### Mark (Arena)
<span id="glossary-mark"></span>
A snapshot of an **Arena's** current allocation offset, stored on a stack. This allows the arena to **Rewind** to this "save point".

### Offset (Arena)
<span id="glossary-offset"></span>
An internal `size_t` value in the `Arena` struct that tracks the current "top" or "watermark" of allocations. It represents the total number of bytes used from the `buffer` so far.

### Reset (Arena)
<span id="glossary-reset"></span>
The operation of setting an arena's **Offset** (and `mark` count) back to zero. This effectively "frees" all memory within the arena, making its full capacity available for new allocations without deallocating or reallocating the underlying `buffer`.

### Rewind (Arena)
<span id="glossary-rewind"></span>
The operation of restoring an **Arena's** allocation **Offset** to the value of the most recently saved **Mark**. This effectively frees all memory that was allocated _after_ that mark was created.

---

## Algorithms
<span id="glossary-algorithms"></span>

Specific algorithms used within `ee_lib` modules.

### Introsort
<span id="glossary-introsort"></span>
A hybrid sorting algorithm used by `ee_array_sort()`. It begins as a **Quicksort** but switches to a **Heapsort** if the recursion depth exceeds a limit, guaranteeing O(n log n) performance.

### Swap and Pop
<span id="glossary-swap-n-pop"></span>
An O(1) (constant time) algorithm for removing an element from an `Array` by overwriting it with the last element and then shrinking the array size by one. This operation does not preserve the array's order.

---

## Platform & SIMD
<span id="glossary-platform"></span>

Terms related to platform-specific, low-level, or compiler-dependent features.

### SIMD
<span id="glossary-simd"></span>
(Single Instruction, Multiple Data). A class of CPU instructions (like **SSE** or **AVX**) that perform the same operation on multiple data points simultaneously. `ee_core.h` provides an abstraction layer over these.

### SSE / AVX
<span id="glossary-sse-avx"></span>
(Streaming SIMD Extensions / Advanced Vector Extensions). Specific **SIMD** instruction sets supported by modern x86 CPUs.