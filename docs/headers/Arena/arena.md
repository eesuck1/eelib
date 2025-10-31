# **ee_arena**

??? "EE_INLINE Arena ee_arena_new(size_t size, size_t rewind_depth, Allocator* allocator)"

    **Decription**
    
    Initialize a new memory arena with a given `size` and optional `rewind_depth`.
    
    If `allocator` is `NULL`, the default allocator is used. Returns an `Arena` structure ready for allocations.

    **Parameters**
    
    | Name         | Type       | Description                                                                      |
    |--------------|------------|----------------------------------------------------------------------------------|
    | size         | size_t     | Total arena buffer size in bytes.                                                |
    | rewind_depth | size_t     | Number of marks that can be stored for rewinding. Use `EE_NO_REWIND` to disable. |
    | allocator    | Allocator* | 	Optional custom allocator. If `NULL`, defaults are used.                          |

    **Example**
    
    ```c
    Allocator my_alloc = {0}; // optional custom allocator
    Arena arena = ee_arena_new(1024, 4, &my_alloc);
    ```

??? "EE_INLINE void ee_arena_clear(Arena* arena)"
    
    **Description**
    
    Clears the arena buffer by setting all bytes to zero without freeing memory.

    **Parameters**
    
    | Name  | Type   | Description                  |
    |-------|--------|------------------------------|
    | arena | Arena* | Pointer to the arena object. |

    **Example**
    
    ```c
    ee_arena_clear(&arena);
    ```

??? "EE_INLINE void\* ee_arena_alloc(Arena* arena, size_t size)"

    **Description**
    
    Allocates a memory block of size bytes from the arena.
    
    Returns `NULL` if there isn’t enough available memory.
    
    **Parameters**
    
    | Name  | Type   | Description                  |
    |-------|--------|------------------------------|
    | arena | Arena* | Pointer to the arena object. |
    | size  | size_t | Number of bytes to allocate. |
    
    **Example**
    
    ```c
    void* data = ee_arena_alloc(&arena, 256);
    ```

??? "EE_INLINE void\* ee_arena_alloc_al(Arena* arena, size_t size, size_t align)"

    **Description**
    
    Allocates a memory block of `size` bytes from the arena with a specified alignment.
    
    Returns `NULL` if not enough aligned space is available.
    
    **Parameters**
    
    | Name  | Type   | Description                                |
    |-------|--------|--------------------------------------------|
    | arena | Arena* | Pointer to the arena object.               |
    | size  | size_t | Number of bytes to allocate.               |
    | align | size_t | Alignment in bytes (must be power of two). |
    
    **Example**
    
    ```c
    void* aligned_data = ee_arena_alloc_al(&arena, 128, 16);
    ```