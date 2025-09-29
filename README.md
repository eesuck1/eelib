# ee — Lightweight C Utility Library

**eelib** is a header-only C library providing common data structures and utilities. 

It includes implementations for arrays, dictionaries, strings, memory management, and other structures. The library is designed to be used directly in C projects without external dependencies.

### **Features**:
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
*   Tested with **MSVC**, **GCC** on **Windows**

### **Installation**:

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
> Some headers depend on others, so make sure you copy all required files.

| Header                                                                          | Description                                                                                                                                                         | Dependencies                                                                                                                                                                                                                                        |
|---------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`ee_arena.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_arena.h)   | Provides a fast, linear memory allocator.                                                                                                                           | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h)   | A dynamic, contiguous array for arbitrary element sizes. Provides fast insertion, deletion, and random access. Supports growing capacity automatically when needed. | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h)     | Provides core definitions, utilities, SIMD wrappers, and allocators used across the library.                                                                        | Independent.                                                                                                                                                                                                                                        |
| [`ee_dict.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_dict.h)     | An open-addressing hash map designed for fixed-size keys and values.                                                                                                | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_fs.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_fs.h)         | Provides a filesystem reader with directory traversal, path utilities, and file operations for Windows.                                                             | Depends on [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h), [`ee_string.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_string.h).                                                                          |
| [`ee_grid.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_grid.h)     | Provides a 2D grid structure with utilities for subregions and pathfinding.                                                                                         | Depends on [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h), [`ee_dict.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_dict.h), [`ee_heap.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_heap.h). |
| [`ee_heap.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_heap.h)     | Implements a binary heap (priority queue) on top of the dynamic vector with custom comparison.                                                                      | Depends on [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h).                                                                                                                                                           |
| [`ee_random.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_random.h) | Random number generator utilities providing uniform and normal distributions for integers and floats.                                                               | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |
| [`ee_set.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_set.h)       | Implements a set data structure using nodes and vectors, supporting a Red-Black tree layout.                                                                        | Depends on [`ee_array.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_array.h).                                                                                                                                                           |
| [`ee_string.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_string.h) | Provides string types for C, supporting dynamic allocation, fixed-size strings, and string views.                                                                   | Depends on [`ee_core.h`](https://github.com/eesuck1/eelib/blob/master/utils/ee_core.h).                                                                                                                                                             |

