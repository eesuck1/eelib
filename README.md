# ee — Lightweight C Utility Library
![Version](https://img.shields.io/badge/Version-0.0.1-3DA9FC)
![License: MIT](https://img.shields.io/badge/License-MIT-264653)
![C Standard](https://img.shields.io/badge/C-C99-F4A261)

**ee** is a header-only C library that provides common data structures and utilities. It is designed to be self-contained and easily integrated into any C99 project, as it has **no external dependencies**.

### Features
The library provides the following modules:

- **Memory management**
  - `ee_arena.h`: A fast, linear arena allocator.
  - `ee_core.h`: Support for optional custom allocators.

- **Dynamic containers**
  - `ee_array.h`: Dynamic arrays (also known as resizable vectors).
  - `ee_dict.h`: Hash maps with open addressing.
  - `ee_heap.h`: Binary heaps, often used for priority queues.
  - `ee_set.h`: Hash sets for efficient item lookup.
  - `ee_grid.h`: 2D grids, useful for spatial data or games.

- **String utilities**
  - `ee_string.h`: Utilities for dynamic strings, fixed-length buffers, and lightweight string views.

- **System utilities**
  - `ee_fs.h`: Filesystem traversal and file utilities (Windows-only).
  - `ee_random.h`: Random number generation (uniform and normal distributions).

### Platforms and Compilers

* **Minimum C Standard:** C99
* **Tested Compilers:** MSVC, GCC

### Installation

As **ee** is a header-only library, you do not need to build it separately. You can use it in your project in two ways.

#### Option 1: Clone the repository

> [!NOTE]  
> This method gives you access to all modules.

1. Clone the repository:
```bash
git clone https://github.com/eesuck1/eelib.git
```

2. Add the `utils/` directory to your project's include path.

3. Include the headers you need in your C source files:

```c
#include "ee_array.h"
#include "ee_dict.h"
#include "ee_string.h.h"
// ...and so on
```

#### Option 2: Copy headers individually

You can also copy individual header files directly into your project.

> [!IMPORTANT]  
> Some headers depend on others. Make sure you copy all required files.

| Header                                                                          | Description                                                             | Dependencies                                                                                                                                                                                                                                        |
|---------------------------------------------------------------------------------|-------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`ee_arena.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_arena.h)   | Provides a linear memory allocator (arena).                             | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h)   | Provides a dynamic, resizable array (vector).                           | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h)     | Defines core types, macros, SIMD abstractions, and base allocators.     | Independent.                                                                                                                                                                                                                                        |
| [`ee_dict.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_dict.h)     | Provides an open-addressing hash map.                                   | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_random.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_random.h) | Provides PRNG for uniform and normal distributions.                     | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_string.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_string.h) | Provides dynamic strings, fixed-buffers, and string views.              | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
### **Configuration**

You can configure the library's behavior by defining specific macros before including its headers.

#### **Disabling assertions (`EE_NO_ASSERT`)**

By default, the library uses `EE_ASSERT()` to validate conditions at runtime (e.g., checking for null pointers). You can disable these checks for performance in a release build.

Define `EE_NO_ASSERT` **before** including any `ee` headers to remove all assertion checks at compile time.

| Mode                             | Description                                                 |
|:---------------------------------|:------------------------------------------------------------|
| With `EE_ASSERT` <br/> (default) | `EE_ASSERT` validates conditions and terminates on failure. |
| With `EE_NO_ASSERT`              | Assertions are removed — no checks or runtime overhead.     |

**Usage example**:

```c
// Disable all assertions for this build
#define EE_NO_ASSERT

#include "ee_core.h"
#include "ee_array.h"
```

#### **SIMD support**

The library includes support for SIMD (Single Instruction, Multiple Data) optimizations. The available levels are:

| Macro                             | Value | Description                                                  |
|:----------------------------------|:------|:-------------------------------------------------------------|
| `EE_SIMD_LEVEL_NONE`              | 0     | No SIMD instructions; all operations are scalar.                 |
| `EE_SIMD_LEVEL_SSE`               | 1     | Supports 128-bit vector instructions for ints and floats.    |
| `EE_SIMD_LEVEL_AVX`<br/>(default) | 2     | Supports 256-bit vector instructions for higher parallelism. |

### **Roadmap**

Work is currently in progress for:

* Dynamic Heap (`ee_heap`)
* File System Utilities (`ee_fs`)
* Double-Ended Queue (`ee_deq`)
* ...and more.

These modules will be added to the documentation as they become stable.
