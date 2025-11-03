# ee — Lightweight C Utility Library
![Version](https://img.shields.io/badge/Version-0.0.1-3DA9FC)
![License: MIT](https://img.shields.io/badge/License-MIT-264653)
![C Standard](https://img.shields.io/badge/C-C99-F4A261)

**ee** is a header-only C library providing common data structures and utilities.

It includes implementations for arrays, dictionaries, strings, memory management, and other structures. The library is designed to be used directly in C projects without external dependencies.

### Features
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

### Platforms and Compilers

* **Minimum C Standard:** C99
* **Tested Compilers:** MSVC, GCC

### Installation

There are two ways to use **ee** in your project:

#### Option 1: Clone the Repository

Clone or download the repository and add the `utils/` directory to your include path.
This way you get access to all modules at once:

```bash
git clone [https://github.com/eesuck1/eelib.git](https://github.com/eesuck1/eelib.git)
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

| Header                                                                          | Description                                                                                                                                                                                 | Dependencies                                                                                                                                                                                                                                        |
|---------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`ee_arena.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_arena.h)   | Provides a linear memory allocator (arena).                                                                                                                           | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h)   | Provides a dynamic, resizable array (vector).                                                        | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h)     | Defines core types, macros, SIMD abstractions, and base allocators.                                                                                      | Independent.                                                                                                                                                                                                                                        |
| [`ee_deq.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_deq.h)       | Provides a double-ended queue (deque).                                                                                         | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_dict.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_dict.h)     | Provides an open-addressing hash map.                                               | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_fs.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_fs.h)         | File system utilities (read file, iterate directory). **Windows-only**.                                                                                    | Depends on [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h), [`ee_string.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_string.h).                                                                          |
| [`ee_grid.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_grid.h)     | Provides a 2D grid container with pathfinding utilities.                                                                                  | Depends on [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h), [`ee_dict.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_dict.h), [`ee_heap.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_heap.h). |
| [`ee_heap.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_heap.h)     | Provides a binary heap (priority queue).                                                                                               | Depends on [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h).                                                                                                                                                           |
| [`ee_random.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_random.h) | Provides PRNG for uniform and normal distributions.                                                                        | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_set.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_set.h)       | Provides a set container (implemented as a Red-Black tree).                                                                                    | Depends on [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h).                                                                                                                                                           |
| [`ee_string.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_string.h) | Provides dynamic strings, fixed-buffers, and string views.                                                                                       | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |

### **Disabling Assertions (`EE_NO_ASSERT`)**

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

### **SIMD Levels**

SIMD levels define the width and type of vector instructions supported by the processor.

The library supports multiple SIMD optimization levels:

| Macro                             | Value | Description                                                  |
|:----------------------------------|:------|:-------------------------------------------------------------|
| `EE_SIMD_LEVEL_NONE`              | 0     | No SIMD instructions; operations are scalar.                 |
| `EE_SIMD_LEVEL_SSE`               | 1     | Supports 128-bit vector instructions for ints and floats.    |
| `EE_SIMD_LEVEL_AVX`<br/>(default) | 2     | Supports 256-bit vector instructions for higher parallelism. |

