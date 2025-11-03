# ee_arena.h

`ee_arena.h` provides a **linear memory allocator** with optional **rewind support**.

It defines the primary `Arena` structure, which manages memory allocation from a contiguous buffer. This module is ideal for temporary allocations where memory can be freed all at once or reverted to a previous state efficiently.

The arena can be wrapped as a standard `Allocator` interface using `ee_arena_allocator()` for compatibility with other `ee_lib` components that require an allocator.

## Structures

Structure `struct Arena` representing the memory arena. It manages the underlying buffer, tracks the current allocation offset, and maintains the optional rewind stack.

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

## Functions

??? "EE_INLINE Arena ee_arena_new(size_t size, size_t rewind_depth, Allocator* allocator)"

    <span id="ee_arena_new"></span>

    **Syntax**
    
    ```c
    Arena ee_arena_new(size_t size, size_t rewind_depth, Allocator* allocator);
    ```
    
    **Description**
    
    Initializes and allocates the memory for a new `Arena` object.
    
    This function reserves a contiguous block of memory from the specified `allocator` (or the default if `NULL`). It also reserves space for the rewind stack if `rewind_depth` is not `EE_NO_REWIND`.
    
    **Parameters**

    | Name           | Type         | Description                                                                                   |
    |----------------|--------------|-----------------------------------------------------------------------------------------------|
    | `size`         | `size_t`     | Total capacity of the arena buffer in bytes.                                                  |
    | `rewind_depth` | `size_t`     | Number of marks that can be stored for rewinding. Use `EE_NO_REWIND` to disable this feature. |
    | `allocator`    | `Allocator*` | Optional custom allocator.<br/> **Note**: Pass `NULL` to use the default allocator.           |

    **Returns**
    
    An initialized `Arena` structure. On allocation failure, this function will assert (if `EE_ASSERT` is enabled).
    
    **Example**
    
    ```c
    // Create a 1MB arena with 16 rewind levels
    Arena arena = ee_arena_new(1024 * 1024, 16, NULL);
    
    // ... use the arena ...
    
    // Always free the arena when done
    ee_arena_free(&arena);
    ```

??? "EE_INLINE void ee_arena_clear(Arena* arena)"

    <span id="ee_arena_clear"></span>
    
    **Syntax**
    
    ```c
    void ee_arena_clear(Arena* arena);
    ```
    
    **Description**
    
    Clears the entire arena buffer by setting its contents to zero using `memset`.
    
    !!! warning "Important"
        This function **does not** reset the `offset` or `mark` counters. To "free" all memory and restart allocation, use `ee_arena_reset()`.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `arena` | `Arena*` | A pointer to the arena object to clear. |
    
    **See Also**
    
    * [`ee_arena_reset()`](#ee_arena_reset)

??? "EE_INLINE void\* ee_arena_alloc(Arena* arena, size_t size)"

    <span id="ee_arena_alloc"></span>

    **Syntax**
    
    ```c
    void* ee_arena_alloc(Arena* arena, size_t size);
    ```
    
    **Description**
    
    Allocates a block of memory from the arena with default alignment (defined by `EE_MAX_ALIGN`). This is the primary, general-purpose allocation function.
    
    Allocations are linear and advance the arena's internal offset.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `arena` | `Arena*` | A pointer to the arena object. |
    | `size` | `size_t` | The number of bytes to allocate. |
    
    **Returns**
    
    A pointer to the allocated block, or `NULL` if the arena does not have enough remaining space.
    
    **Example**
    
    ```c
    MyStruct* s = ee_arena_alloc(&arena, sizeof(*s));
    
    if (s != NULL)
    {
    // ... initialize and use s ...
    }
    ```
    
    **See Also**
    
    * [`ee_arena_alloc_al()`](#ee_arena_alloc_al)

??? "EE_INLINE void\* ee_arena_alloc_al(Arena* arena, size_t size, size_t align)"

    <span id="ee_arena_alloc_al"></span>

    **Syntax**
    
    ```c
    void* ee_arena_alloc_al(Arena* arena, size_t size, size_t align);
    ```
    
    **Description**
    
    Allocates a block of memory from the arena with a **custom alignment**.
    
    The `align` parameter must be a power of two. If `align` is 1, no alignment padding is added beyond the natural offset.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `arena` | `Arena*` | A pointer to the arena object. |
    | `size` | `size_t` | The number of bytes to allocate. |
    | `align` | `size_t` | The required alignment for the returned pointer (must be a power of two). |
    
    **Returns**
    
    A pointer to the allocated block with the specified alignment, or `NULL` if the arena does not have enough remaining space.
    
    **Example**
    
    ```c
    // Allocate 64 bytes aligned to a 16-byte boundary
    void* aligned_mem = ee_arena_alloc_al(&arena, 64, 16);
    ```

??? "EE_INLINE void ee_arena_mark(Arena* arena)"

    <span id="ee_arena_mark"></span>

    **Syntax**
    
    ```c
    void ee_arena_mark(Arena* arena);
    ```
    
    **Description**
    
    Saves the current allocation offset (`arena->offset`) onto the rewind stack. This creates a "save point" that can be returned to using `ee_arena_rewind()`.
    
    This function will assert if the arena was created with `EE_NO_REWIND` or if the mark stack is full.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `arena` | `Arena*` | A pointer to the arena object. |
    
    **See Also**
    
    * [`ee_arena_rewind()`](#ee_arena_rewind)

??? "EE_INLINE void ee_arena_rewind(Arena* arena)"

    <span id="ee_arena_rewind"></span>

    **Syntax**
    
    ```c
    void ee_arena_rewind(Arena* arena);
    ```
    
    **Description**
    
    Restores the arena's allocation offset to the last saved mark from the rewind stack. This effectively "frees" all memory allocated since that mark was saved.
    
    This function will assert if the mark stack is empty (a "stack underflow").
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `arena` | `Arena*` | A pointer to the arena object. |
    
    **Example**
    
    ```c
    // Save state
    ee_arena_mark(&arena);
    
    // Perform temporary allocations
    void* temp_obj_1 = ee_arena_alloc(&arena, 32);
    void* temp_obj_2 = ee_arena_alloc(&arena, 64);
    
    // ... use temp objects ...
    
    // Free all memory back to the mark
    ee_arena_rewind(&arena); 
    ```

??? "EE_INLINE void ee_arena_reset(Arena* arena)"

    <span id="ee_arena_reset"></span>

    **Syntax**
    
    ```c
    void ee_arena_reset(Arena* arena);
    ```
    
    **Description**
    
    Resets the arena to its initial state by setting the allocation offset and mark count to zero. This is the fastest way to free **all** memory in the arena for reuse.
    
    This function does **not** zero the memory content.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `arena` | `Arena*` | A pointer to the arena object. |
    
    **See Also**
    
    * [`ee_arena_clear()`](#ee_arena_clear)

??? "EE_INLINE void ee_arena_free(Arena* arena)"

    <span id="ee_arena_free"></span>

    **Syntax**
    
    ```c
    void ee_arena_free(Arena* arena);
    ```
    
    **Description**
    
    Frees the **entire** memory block used by the arena, including the buffer and mark stack. This calls the `free_fn` of the underlying allocator on the `base` pointer.
    
    After this call, the `Arena` struct is zeroed and must not be used again unless re-initialized with `ee_arena_new()`.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `arena` | `Arena*` | A pointer to the arena object to be destroyed. |
    
    **See Also**
    
    * [`ee_arena_new()`](#ee_arena_new)

??? "EE_INLINE Allocator ee_arena_allocator(Arena* arena)"

    <span id="ee_arena_allocator"></span>

    **Syntax**
    
    ```c
    Allocator ee_arena_allocator(Arena* arena);
    ```
    
    **Description**
    
    Wraps an existing `Arena` instance in a standard `Allocator` interface.
    
    This allows the arena to be passed to other functions or systems that require a generic `Allocator` object (e.g., data structures like lists or hash maps). The `arena` pointer is stored in the `Allocator.context` field.

    !!! info "Interface Behavior"
        
        The returned allocator has specific behaviors tied to the arena's linear nature:
        
        * `alloc_fn`: Calls [`eev_arena_alloc_fn()`](#eev_arena_alloc_fn), which allocates memory from the arena.
        * `realloc_fn`: Calls [`eev_arena_realloc_fn()`](#eev_arena_realloc_fn), which **always returns `NULL`**. Arenas do not support reallocation.
        * `free_fn`: Calls [`eev_arena_free_fn()`](#eev_arena_free_fn), which **does nothing**. Arenas do not support freeing individual blocks.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `arena` | `Arena*` | A pointer to the initialized arena object to be wrapped. |
    
    **Returns**
    
    An `Allocator` struct configured to use the provided `arena` for allocations.
    
[//]: # (    **Example**)

[//]: # (    )
[//]: # (    ```c)

[//]: # (    // Create a 4K scratch arena for temporary objects)

[//]: # (    Arena scratch_arena = ee_arena_new&#40;4096, EE_NO_REWIND, NULL&#41;;)

[//]: # (    )
[//]: # (    // Get the generic allocator interface for it)

[//]: # (    Allocator arena_iface = ee_arena_allocator&#40;&scratch_arena&#41;;)

[//]: # (    )
[//]: # (    // Pass the interface to another system &#40;e.g., a list constructor&#41;)

[//]: # (    MyList* list = my_list_create&#40;&arena_iface&#41;;)

[//]: # (    my_list_push&#40;list, "hello"&#41;; // This allocation uses the arena)

[//]: # (    )
[//]: # (    // ... use the list for a short time ...)

[//]: # (    )
[//]: # (    // When done, free the *entire* arena at once)

[//]: # (    ee_arena_free&#40;&scratch_arena&#41;;)

[//]: # (    // The 'list' and its data are now invalid)

[//]: # (    ```)
    
    **See Also**
    
    * [`ee_arena_new()`](#ee_arena_new)
    * [`ee_arena_alloc()`](#ee_arena_alloc)
    * [`eev_arena_alloc_fn()`](#eev_arena_alloc_fn)
    * [`eev_arena_realloc_fn()`](#eev_arena_realloc_fn)
    * [`eev_arena_free_fn()`](#eev_arena_free_fn)
    * [`struct Allocator`](../Core/core.md##Structures)

## Allocator Functions

These functions are the internal implementations used by the [`struct Allocator`](../Core/core.md##Structures) returned by `ee_arena_allocator()`. End-users typically do not call these functions directly.

??? "EE_INLINE void\* eev_arena_alloc_fn(Allocator\* allocator, size_t size)"

    <span id="eev_arena_alloc_fn"></span>

    **Syntax**
    
    ```c
    void* eev_arena_alloc_fn(Allocator* allocator, size_t size);
    ```
    
    **Description**
    
    The `alloc_fn` implementation for the arena-backed allocator.
    
    It retrieves the `Arena` pointer from `allocator->context` and calls [`ee_arena_alloc()`](#ee_arena_alloc) to perform a linear allocation.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `allocator` | `Allocator*` | The allocator interface, which must have its `context` set to a valid `Arena*`. |
    | `size` | `size_t` | The number of bytes to allocate. |
    
    **Returns**
    
    A pointer to the allocated memory, or `NULL` if the arena is full.
    
    **See Also**
    
    * [`ee_arena_allocator()`](#ee_arena_allocator)
    * [`ee_arena_alloc()`](#ee_arena_alloc)

??? "EE_INLINE void\* eev_arena_realloc_fn(Allocator\* allocator, void\* buffer, size_t old_size, size_t new_size)"

    <span id="eev_arena_realloc_fn"></span>

    **Syntax**
    
    ```c
    void* eev_arena_realloc_fn(Allocator* allocator, void* buffer, size_t old_size, size_t new_size);
    ```
    
    **Description**
    
    The `realloc_fn` implementation for the arena-backed allocator.
    
    This function **always returns `NULL`**. Due to their linear nature, arenas do not support reallocating individual memory blocks.
    
    **Parameters**
    
    ***Note**: As this operation is not supported by the arena, all parameters are ignored.*
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `allocator` | `Allocator*` | — |
    | `buffer` | `void*` | — |
    | `old_size` | `size_t` | — |
    | `new_size` | `size_t` | — |
    
    **Returns**
    
    Always `NULL`.
    
    **See Also**
    
    * [`ee_arena_allocator()`](#ee_arena_allocator)

??? "EE_INLINE void eev_arena_free_fn(Allocator\* allocator, void\* buffer)"

    <span id="eev_arena_free_fn"></span>

    **Syntax**
    
    ```c
    void eev_arena_free_fn(Allocator* allocator, void* buffer);
    ```
    
    **Description**
    
    The `free_fn` implementation for the arena-backed allocator.
    
    This function **does nothing** (it is a "no-op"). Arenas do not support freeing individual memory blocks. All memory must be freed at once by resetting or freeing the entire arena.
    
    **Parameters**
    
    ***Note**: As this operation is not supported by the arena, all parameters are ignored.*
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `allocator` | `Allocator*` | — |
    | `buffer` | `void*` | — |
    
    **See Also**
    
    * [`ee_arena_allocator()`](#ee_arena_allocator)
    * [`ee_arena_reset()`](#ee_arena_reset)
    * [`ee_arena_free()`](#ee_arena_free)