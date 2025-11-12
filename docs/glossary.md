---
hide:
  - toc
---

# Glossary

This page defines key terms and concepts used throughout the `eelib` library. 

To find a specific term, use: 

* the search box on the top panel
* the category buttons below to filter the list

<div id="glossary-filter-buttons" class="filter-buttons">
  <button class="filter-btn" data-tag="core">Core Concepts</button>
  <button class="filter-btn" data-tag="datastructures">Data Structures</button>
  <button class="filter-btn" data-tag="memory">Memory Management</button>
  <button class="filter-btn" data-tag="algorithms">Algorithms</button>
  <button class="filter-btn" data-tag="platform">Platform & SIMD</button>
</div>
---

## Core Concepts {: data-tags="core"}
<span id="glossary-core"></span>

Core ideas and types that are used by all other modules.

### Allocator (Interface)
<span id="glossary-allocator"></span>
The standard `eelib` interface (defined in [`ee_core.h`](headers/Core/core.md#allocator-interface)) that provides a generic API for memory operations (`alloc_fn`, `realloc_fn`, `free_fn`).

### BinCmp
<span id="glossary-bincmp"></span>
A function pointer type (`int (*)(const void* a, const void* b)`) used for generic comparison operations, such as sorting. It must return a negative, zero, or positive value based on the comparison.

### Type-punning
<span id="glossary-type-punning"></span>
An operation (e.g., `*(u64*)ptr`) that reinterprets a block of memory as a different type. `eelib` provides safe (`ee_eq_safe_...`) and fast (`ee_eq_...`) versions of these operations.

---

## Data Structures {: data-tags="datastructures"}
<span id="glossary-datastructures"></span>

The main data structure modules provided by `eelib`.

### Arena
<span id="glossary-arena"></span>
A **Linear Allocator** that allocates memory sequentially from a single, contiguous block. Allocations are fast, but individual blocks cannot be freed.

### Array (Dynamic)
<span id="glossary-array"></span>
A type-generic dynamic array (vector) that manages a contiguous, resizable memory buffer. It operates on raw bytes by tracking an `elem_size`.

---

## Memory Management {: data-tags="memory"}
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

## Algorithms {: data-tags="algorithms"}
<span id="glossary-algorithms"></span>

Specific algorithms used within `eelib` modules.

### Introsort
<span id="glossary-introsort"></span>
A hybrid sorting algorithm used by `ee_array_sort()`. It begins as a **Quicksort** but switches to a **Heapsort** if the recursion depth exceeds a limit, guaranteeing O(n log n) performance.

### Swap and Pop
<span id="glossary-swap-n-pop"></span>
An O(1) (constant time) algorithm for removing an element from an `Array` by overwriting it with the last element and then shrinking the array size by one. This operation does not preserve the array's order.

---

## Platform & SIMD {: data-tags="platform"}
<span id="glossary-platform"></span>

Terms related to platform-specific, low-level, or compiler-dependent features.

### SIMD
<span id="glossary-simd"></span>
(Single Instruction, Multiple Data). A class of CPU instructions (like **SSE** or **AVX**) that perform the same operation on multiple data points simultaneously. `ee_core.h` provides an abstraction layer over these.

### SSE / AVX
<span id="glossary-sse-avx"></span>
(Streaming SIMD Extensions / Advanced Vector Extensions). Specific **SIMD** instruction sets supported by modern x86 CPUs.

















