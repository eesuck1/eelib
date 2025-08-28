# ee — Lightweight C Utility Library

**ee** is a lightweight, header-only C library that provides **fundamental building blocks** for systems programming and performance-critical applications.  

It offers:  
- **Memory management** via a fast arena allocator.  
- **Dynamic containers** such as resizable vectors and hash maps.  
- **String utilities** built on top of the vector type.  

The goal of the library is to give C developers a **minimal, dependency-free toolkit** with APIs similar in spirit to higher-level languages, while still keeping the full control and efficiency of plain C.  

Key characteristics:  
- **Header-only** — just include the headers, no build system integration needed.  
- **Portable** — standard C11 with optional SIMD optimizations for modern CPUs.  
- **Low-overhead** — designed with performance and cache efficiency in mind.  
- **Extensible** — easy to integrate and adapt into existing projects.  

**Use cases:**  
This library is suitable for projects where performance, simplicity, and minimal dependencies matter, including:  
- Game engines and real-time simulations  
- Embedded systems and firmware  
- High-performance tools and utilities  
- Prototyping and small C projects
  
## Dictionary (Dict)

Dict is a high-performance, open-addressing hash map designed for fixed-size keys and values
It uses SIMD-accelerated group probing for fast insertion and lookup

### Features
- Fast performance: in benchmarks, for a dictionary of size 1024 and ~50% fill,
  insertion takes ~9ns and lookup takes ~6ns per operation
- Fixed-size 8-byte keys and values (uint8_t[8])
- Automatic resizing when the load factor exceeds the threshold
- SIMD-optimized lookup for modern CPUs
- Supports deletion and iteration over all key-value pairs

### Example Usage

#include "stdio.h"
#include "ee_dict.h"

int main() 
{
    // Create a new dictionary
    Dict dict = ee_dict_new(128);

    // Define a key and value
    DictKey key = { 0x01, 0x02, 0x03, 0x04, 0, 0, 0, 0 };
    DictValue val = { 0xAA, 0xBB, 0xCC, 0xDD, 0, 0, 0, 0 };

    // Insert the key-value pair
    ee_dict_set(&dict, key, val);

    // Retrieve a value by key
    DictValue out = ee_dict_get(&dict, key);
    
    // Retrieve a value pointer by key
    DictValue* out_ptr = ee_dict_at(&dict, key);

    // Check if a key exists
    if (ee_dict_contains(&dict, key)) 
    {
        printf("Key exists\n");
    }

    // Iterate over all items
    DictIter it = ee_dict_iter_new(&dict);
    DictKey iter_key;
    DictValue iter_val;
    
    while (ee_dict_iter_next(&it, &iter_key, &iter_val)) 
    {
        // process iter_key and iter_val
    }

    // Free the dictionary
    ee_dict_free(&dict);

    return 0;
}

## Vector (Vec)

Vec is a dynamic, contiguous array for arbitrary element sizes
It provides fast insertion, deletion, and random access
Supports growing capacity automatically when needed

### Features
- Dynamic resizing with configurable element size
- Push, pop, insert, erase, and find operations
- Random access through ee_vec_at and ee_vec_top
- Fast iteration: in benchmarks, iterating over a vector of 1 million elements
  takes ~0.7ns per element (~1.4 billion elements per second)

### Example Usage

#include "stdio.h"
#include "stdint.h"
#include "ee_vec.h"

int main() 
{
    // Create a new vector with 4-byte elements and initial capacity 1024
    Vec vec = ee_vec_new(1024, 4);

    uint32_t value = 1234;

    // Push elements into the vector
    ee_vec_push(&vec, EE_VEC_DT(value));

    // Access elements
    uint32_t* top_val = (uint32_t*)ee_vec_top(&vec);
    printf("Top value: %u\n", *top_val);

    uint32_t* first_val = (uint32_t*)ee_vec_at(&vec, 0);
    printf("First value: %u\n", *first_val);

    // Set element at index 0
    value = 5678;
    ee_vec_set(&vec, 0, EE_VEC_DT(value));

    // Insert element at index 0
    value = 9999;
    ee_vec_insert(&vec, 0, EE_VEC_DT(value));

    // Erase element at index 1
    ee_vec_erase(&vec, 1);

    // Find element
    size_t index = ee_vec_find(&vec, EE_VEC_DT(value));
    if (index != EE_VEC_INVALID) 
    {
        printf("Found value at index %zu\n", index);
    }

    // Pop top element
    ee_vec_pop(&vec);

    // Clear vector
    ee_vec_clear(&vec);

    // Free vector memory
    ee_vec_free(&vec);

    return 0;
}

## Strings (Str, ShortStr, LongStr)

This library provides flexible string types for C, supporting dynamic allocation,
small fixed-size strings, and string views.

### Types

- `Str` – dynamically allocated string with automatic resizing
- `ShortStr` – fixed-size string optimized for short text (16 bytes by default)
- `LongStr` – hybrid string with small prefix and dynamic allocation for longer text
- `StrView` – non-owning view into an existing string buffer

### Features

- Create strings from C-style buffers
- Copy, assign, reset, and clear strings
- Convert `Str` to null-terminated C string
- Fast comparison for `ShortStr` using 128-bit operations

### Example Usage

#include "stdio.h"
#include "ee_string.h"

int main() 
{
    // Create a new dynamic string
    Str s = ee_str_new();
    
    // Assign a C-string
    Str tmp = ee_str_from("hello world");
    ee_str_assign(&s, &tmp);

    // Convert back to C-string
    char* cstr = ee_str_cstr(&s);
    printf("%s\n", cstr);
    free(cstr);

    // Create a short string
    str_dt buffer[16] = { 'f','o','o' };
    ShortStr ss = ee_short_str_new(buffer, 3);

    // Copy dynamic string
    Str s_copy = ee_str_copy(&s);
    
    // Clear and reset strings
    ee_str_clear(&s);
    ee_str_reset(&s_copy);

    // Free memory
    ee_str_free(&s);
    ee_str_free(&s_copy);

    return 0;
}

## Arena (Memory Arena)

The `Arena` provides a fast, linear memory allocator. It allows
you to allocate multiple objects without individual `malloc` calls
and supports efficient mark/rewind semantics for temporary allocations.

### Features

- Fast contiguous allocations
- Manual reset and mark/rewind functionality
- Minimal overhead
- Useful for temporary allocations in performance-critical code

### Example Usage

#include "stdio.h"
#include "ee_arena.h"

int main() 
{
    // Create a new arena with 1024 bytes
    Arena arena = ee_arena_new(1024);

    // Allocate memory for 10 integers
    int* arr = (int*)ee_arena_alloc(&arena, 10, sizeof(int), alignof(int));
    for (int i = 0; i < 10; i++)
    {
        arr[i] = i * i;
    }

    // Print values
    for (int i = 0; i < 10; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");

    // Mark current offset
    ee_arena_mark(&arena);

    // Allocate more memory temporarily
    int* temp = (int*)ee_arena_alloc(&arena, 5, sizeof(int), alignof(int));
    for (int i = 0; i < 5; i++)
    {
        temp[i] = i + 100;
    }

    // Rewind to previous mark, freeing temporary allocations
    ee_arena_rewind(&arena);

    // Reset arena completely
    ee_arena_reset(&arena);

    // Free arena
    ee_arena_free(&arena);

    return 0;
}

### Notes

The arena is ideal for cases where you need many short-lived allocations.
By using marks and rewinds, you can efficiently revert allocations without
calling `free` repeatedly. This pattern improves cache locality and reduces
allocation overhead in high-performance scenarios.
