# **ee_arena**

`ee_arena.h` provides a linear memory allocator with optional rewind support.
It defines the `Arena` structure, containing a buffer, allocation offset, mark stack, and allocator callbacks.

Memory is allocated from a contiguous buffer with alignment handling; marks allow rewinding to previous offsets.

The arena can be wrapped as a standard `Allocator` using `ee_arena_allocator()`.

### **Structures**:

```c
typedef struct Arena
{
    size_t* marks;       // Stack of stored memory offsets for rewind points
    u8* buffer;          // Pointer to the main aligned memory buffer
    u8* base;            // Pointer to the raw allocated memory block

    size_t  size;        // Total available arena size in bytes
    size_t  offset;      // Current allocation offset (used size)
    size_t  mark;        // Current depth of the mark stack
    size_t  marks_depth; // Maximum number of mark entries available

    Allocator allocator; // Custom or default allocator used for base memory
} Arena;

```

### **Functions**:

??? "EE_INLINE Arena ee_arena_new(size_t size, size_t rewind_depth, Allocator* allocator)"

    **Decription**
    
    Initialize a new memory arena with a given `size` and optional `rewind_depth`.
    
    If `allocator` is `NULL`, the default allocator is used. Returns an `Arena` structure ready for allocations.

    **Parameters:**
    
    | Name         | Type       | Description                                                                      |
    |--------------|------------|----------------------------------------------------------------------------------|
    | size         | size_t     | Total arena buffer size in bytes.                                                |
    | rewind_depth | size_t     | Number of marks that can be stored for rewinding. Use `EE_NO_REWIND` to disable. |
    | allocator    | Allocator* | 	Optional custom allocator. If `NULL`, defaults are used.                          |

    **Example:**
    
    ```c
    Arena arena = ee_arena_new(1024, 4, NULL);
    void* block = ee_arena_alloc(&arena, 128);
    ```

??? "EE_INLINE void ee_arena_clear(Arena* arena)"
    
    **Description:**
    
    Clears the arena buffer by setting all bytes to zero without freeing memory.

    **Parameters:**
    
    | Name  | Type   | Description                  |
    |-------|--------|------------------------------|
    | arena | Arena* | Pointer to the arena object. |

    **Example:**
    
    ```c
    ee_arena_clear(&arena);
    ```

??? "EE_INLINE void\* ee_arena_alloc(Arena* arena, size_t size)"

    **Description:**
    
    Allocates a memory block of size bytes from the arena.
    
    Returns `NULL` if there isn’t enough space is available.
    
    **Parameters:**
    
    | Name  | Type   | Description                  |
    |-------|--------|------------------------------|
    | arena | Arena* | Pointer to the arena object. |
    | size  | size_t | Number of bytes to allocate. |
    
    **Example:**
    
    ```c
    void* data = ee_arena_alloc(&arena, 256);
    ```

??? "EE_INLINE void\* ee_arena_alloc_al(Arena* arena, size_t size, size_t align)"

    **Description:**
    
    Allocates a memory block of `size` bytes from the arena with a specified alignment.
    
    Returns `NULL` if not enough aligned space is available.
    
    **Parameters:**
    
    | Name  | Type   | Description                                |
    |-------|--------|--------------------------------------------|
    | arena | Arena* | Pointer to the arena object.               |
    | size  | size_t | Number of bytes to allocate.               |
    | align | size_t | Alignment in bytes (must be power of two). |
    
    **Example:**
    
    ```c
    void* aligned_data = ee_arena_alloc_al(&arena, 128, 16);
    ```

??? "EE_INLINE void ee_arena_mark(Arena* arena)"

    **Description:**
    
    Marks the current allocation position in the arena so it can be rewound later.
    Useful for temporary allocations.
    
    **Parameters:**
    
    | Name  | Type   | Description                  |
    |-------|--------|------------------------------|
    | arena | Arena* | Pointer to the arena object. |
    
    **Example:**
    
    ```c
    ee_arena_mark(&arena);
    // allocate something temporary
    ```

??? "EE_INLINE void ee_arena_rewind(Arena* arena)"

    **Description:**
    
    Restores the arena’s allocation state to the last saved mark, effectively freeing allocations made after that mark.

    **Parameters:**
    
    | Name  | Type   | Description                  |
    |-------|--------|------------------------------|
    | arena | Arena* | Pointer to the arena object. |

    **Example:**
    
    ```c
    ee_arena_rewind(&arena);
    ```

??? "EE_INLINE void ee_arena_reset(Arena* arena)"

    **Description:**
    
    Resets the arena to its initial state, clearing all marks and allocations.  
    Does not free memory — only resets internal offsets.

    **Parameters:**
    
    | Name  | Type   | Description                  |
    |-------|--------|------------------------------|
    | arena | Arena* | Pointer to the arena object. |

    **Example:**
    
    ```c
    ee_arena_reset(&arena);
    ```

??? "EE_INLINE void ee_arena_free(Arena* arena)"

    **Description:**
    
    Frees all arena memory and resets its internal state to zero.

    **Parameters:**
    
    | Name  | Type   | Description                  |
    |-------|--------|------------------------------|
    | arena | Arena* | Pointer to the arena object. |

    **Example:**
    
    ```c
    ee_arena_free(&arena);
    ```

??? "EE_INLINE void\* eev_arena_alloc_fn(Allocator\* allocator, size_t size)"

    **Description:**
    
    Allocator callback for arena-based allocation.  
    This function integrates the arena with generic allocator interfaces.

    **Parameters:**
    
    | Name      | Type        | Description                      |
    |------------|-------------|----------------------------------|
    | allocator  | Allocator*  | Pointer to the allocator object. |
    | size       | size_t      | Number of bytes to allocate.     |

    **Example:**
    
    ```c
    Allocator a = ee_arena_allocator(&arena);
    void* mem = eev_arena_alloc_fn(&a, 64);
    ```

??? "EE_INLINE void\* eev_arena_realloc_fn(Allocator\* allocator, void* buffer, size_t old_size, size_t new_size)"

    **Description:**
    
    Stub implementation of realloc for arena allocator (always returns `NULL`).  
    Arena memory is not resizable.

    **Parameters:**
    
    | Name      | Type        | Description                         |
    |------------|-------------|-------------------------------------|
    | allocator  | Allocator*  | Pointer to the allocator object.    |
    | buffer     | void*       | Pointer to existing buffer.         |
    | old_size   | size_t      | Original buffer size (unused).      |
    | new_size   | size_t      | Requested size (unused).            |

    **Example:**
    
    ```c
    void* p = eev_arena_realloc_fn(&allocator, ptr, 64, 128); // always returns NULL
    ```

??? "EE_INLINE void eev_arena_free_fn(Allocator\* allocator, void\* buffer)"

    **Description:**
    
    Stub implementation of free for arena allocator (does nothing).  
    Arena allocations are released only via `ee_arena_reset` or `ee_arena_free`.

    **Parameters:**
    
    | Name      | Type        | Description                      |
    |------------|-------------|----------------------------------|
    | allocator  | Allocator*  | Pointer to allocator.            |
    | buffer     | void*       | Pointer to buffer (unused).      |

    **Example:**
    
    ```c
    eev_arena_free_fn(&allocator, ptr); // no effect
    ```

??? "EE_INLINE Allocator ee_arena_allocator(Arena* arena)"

    **Description:**
    
    Creates an `Allocator` object that uses the specified `Arena` for its memory operations.

    **Parameters:**
    
    | Name  | Type   | Description                  |
    |-------|--------|------------------------------|
    | arena | Arena* | Pointer to the arena object. |

    **Example:**
    
    ```c
    Allocator a = ee_arena_allocator(&arena);
    ```