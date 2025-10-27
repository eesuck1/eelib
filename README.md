# ee — Lightweight C Utility Library
![Version](https://img.shields.io/badge/Version-0.0.1-3DA9FC)
![License: MIT](https://img.shields.io/badge/License-MIT-264653)
![C Standard](https://img.shields.io/badge/C-C99-F4A261)

**eelib** is a header-only C library providing common data structures and utilities. 

It includes implementations for arrays, dictionaries, strings, memory management, and other structures. The library is designed to be used directly in C projects without external dependencies.

### **Features**
The library provides a set of building blocks:

- **Memory management**

    - Arena allocator (```ee_arena.h```) for fast, linear allocations.
    - Optional custom allocators via ```ee_core.h```.

- **Dynamic containers**

    - Resizable vectors (```ee_array.h```) with automatic growth.
    - Hash maps (```ee_dict.h```) with open addressing.
    - Binary heaps (```ee_heap.h```) for priority scheduling.
    - Sets (```ee_set.h```) with efficient lookup.
    - 2D grids (```ee_grid.h```) for spatial or game-related logic.
      
- **String utilities**

    - ```ee_string.h``` builds on vectors to support dynamic strings, fixed-length buffers, and lightweight string views.
      
- **System utilities**

    - ```ee_fs.h``` for filesystem traversal and file utilities (Windows support).
    - ```ee_random.h``` for uniform and normal random distributions.  

### **Supported platforms/compilers**

*   Minimum C standard: **C99**
*   Tested with **MSVC**, **GCC**

### **Installation**

There are two ways to use **ee** in your project:

- **Clone the repository**

Clone or download the repository and add the ```utils/``` directory to your include path.
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

- **Copy specific headers**

Since **ee** is header-only, you can also copy a single header (or a subset) into your project.

> [!IMPORTANT]  
> Some headers depend on others. Make sure you copy all required files.

| Header                                                                          | Description                                                                                                                                                                                 | Dependencies                                                                                                                                                                                                                                        |
|---------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`ee_arena.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_arena.h)   | Contains definitions and functions for a linear memory allocator.                                                                                                                           | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h)   | Provides a dynamically resizable contiguous array type for arbitrary element sizes. Supports insertion, deletion, and random access.                                                        | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h)     | Defines core types, macros, SIMD abstractions, and base memory allocators used throughout the library.                                                                                      | Independent.                                                                                                                                                                                                                                        |
| [`ee_deq.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_deq.h)       | Implements a double-ended queue (deque) data structure with dynamic resizing and allocator support.                                                                                         | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_dict.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_dict.h)     | Implements an open-addressing hash map for fixed-size keys and values. Used for key-value storage with constant-time access in average cases.                                               | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_fs.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_fs.h)         | Provides functions for reading files, iterating directories, and handling file paths on Windows systems.                                                                                    | Depends on [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h), [`ee_string.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_string.h).                                                                          |
| [`ee_grid.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_grid.h)     | Defines a 2D grid container with utilities for accessing subregions and performing pathfinding operations.                                                                                  | Depends on [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h), [`ee_dict.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_dict.h), [`ee_heap.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_heap.h). |
| [`ee_heap.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_heap.h)     | Implements a binary heap structure for managing priority queues with custom comparison logic.                                                                                               | Depends on [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h).                                                                                                                                                           |
| [`ee_random.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_random.h) | Provides pseudo-random number generation for integer and floating-point types with uniform and normal distributions.                                                                        | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_set.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_set.h)       | Implements a set container based on a Red-Black tree layout for storing unique elements in sorted order.                                                                                    | Depends on [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h).                                                                                                                                                           |
| [`ee_string.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_string.h) | Provides string types and utilities for dynamic, fixed-size, and view-based string manipulation in C.                                                                                       | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |

### **`EE_NO_ASSERT`** usage

Defining `EE_NO_ASSERT` before including header disables all `EE_ASSERT()` checks at compile time.

| Mode                             | Description                                                 |
|:---------------------------------|:------------------------------------------------------------|
| With `EE_ASSERT` <br/> (default) | `EE_ASSERT` validates conditions and terminates on failure. |
| With `EE_NO_ASSERT`              | Assertions are removed — no checks or runtime overhead.     |

**Usage example**:

```c
#ifndef EE_DICT_EXAMPLE_H
#define EE_DICT_EXAMPLE_H

#define EE_NO_ASSERT

#include "ee_dict.h"
```

Use this to exclude safety checks in release builds for better performance.

### **SIMD Levels**

SIMD levels define the width and type of vector instructions supported by the processor. 

The library supports multiple SIMD optimization levels:

| Macro                             | Value | Description                                                  |
|:----------------------------------|:------|:-------------------------------------------------------------|
| `EE_SIMD_LEVEL_NONE`              | 0     | No SIMD instructions; operations are scalar.                 |
| `EE_SIMD_LEVEL_SSE`               | 1     | Supports 128-bit vector instructions for ints and floats.    |
| `EE_SIMD_LEVEL_AVX`<br/>(default) | 2     | Supports 256-bit vector instructions for higher parallelism. |
