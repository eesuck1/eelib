[//]: # (# **ee_array**)

[//]: # ()
[//]: # (The `ee_array.h` defines a generic, dynamically resizable array implementation with customizable memory management.  )

[//]: # (It provides low-level operations for allocation, resizing, element access, and sorting, built around an explicit memory model using a user-defined or default `Allocator`.)

[//]: # ()
[//]: # (### **Structures**:)

[//]: # ()
[//]: # (Structure `struct Array` defines the core container for a resizable, contiguous array.)

[//]: # (All array operations depend on these fields to manage allocation and indexing.)

[//]: # ()
[//]: # (```c)

[//]: # (typedef struct Array )

[//]: # ({)

[//]: # (    size_t top;          // Current used memory offset)

[//]: # (    size_t cap;          // Total allocated capacity in bytes)

[//]: # (    size_t elem_size;    // Size of each element)

[//]: # (    u8* buffer;          // Pointer to data buffer)

[//]: # (    Allocator allocator; // Memory allocator for dynamic operations)

[//]: # (} Array;)

[//]: # (```)

[//]: # ()
[//]: # (Structure `enum ArraySortType` enumerates available sorting algorithms for internal operations.)

[//]: # ()
[//]: # (```c)

[//]: # (typedef enum ArraySortType )

[//]: # ({)

[//]: # (	EE_SORT_DEFAULT = 0,)

[//]: # (	EE_SORT_INSERT  = 1,)

[//]: # (	EE_SORT_QUICK   = 2,)

[//]: # (	EE_SORT_HEAP    = 3,)

[//]: # (	EE_SORT_INTRO   = 4,)

[//]: # (} ArraySortType;)

[//]: # (```)

[//]: # ()
[//]: # (### **Functions**:)

[//]: # ()
[//]: # (??? "EE_INLINE Array ee_array_new&#40;size_t size, size_t elem_size, const Allocator* allocator&#41;")

[//]: # ()
[//]: # (    **Description**)

[//]: # (    )
[//]: # (    Initializes a new dynamic array with the specified initial capacity and element size. If `allocator` is `NULL`, the function uses the default memory allocation callbacks.  )

[//]: # (    )
[//]: # (    The returned `Array` structure is ready for element insertion and management operations.)

[//]: # (    )
[//]: # (    **Parameters:**)

[//]: # (    )
[//]: # (    | Name       | Type             | Description |)

[//]: # (    |-------------|------------------|--------------|)

[//]: # (    | size        | size_t           | Initial number of elements to allocate space for. |)

[//]: # (    | elem_size   | size_t           | Size &#40;in bytes&#41; of a single element. |)

[//]: # (    | allocator   | const Allocator* | Optional custom allocator. If `NULL`, default allocation functions are used. |)

[//]: # (    )
[//]: # (    **Example:**)

[//]: # (    )
[//]: # (    ```c)

[//]: # (    Array arr = ee_array_new&#40;16, sizeof&#40;int&#41;, NULL&#41;;)

[//]: # (    )
[//]: # (    int value = 42;)

[//]: # (    ee_array_push&#40;&arr, &#40;u8*&#41;&value&#41;;)

[//]: # (    ```)

[//]: # ()
[//]: # ()
[//]: # (??? "EE_INLINE int ee_array_full&#40;const Array* array&#41;")

[//]: # ()
[//]: # (    **Description**)

[//]: # (    )
[//]: # (    Checks whether the given array has reached its current capacity.  )

[//]: # (    Returns non-zero &#40;`true`&#41; if the array is full and cannot accommodate more elements without reallocation.)

[//]: # (    )
[//]: # (    **Parameters:**)

[//]: # (    )
[//]: # (    | Name   | Type         | Description |)

[//]: # (    |--------|--------------|--------------|)

[//]: # (    | array  | const Array* | Pointer to the array to check. Must not be `NULL`. |)

[//]: # (    )
[//]: # (    **Example:**)

[//]: # (    )
[//]: # (    ```c)

[//]: # (    if &#40;ee_array_full&#40;&arr&#41;&#41; {)

[//]: # (        ee_array_grow&#40;&arr, 32&#41;;)

[//]: # (    })

[//]: # (    ```)

[//]: # ()
[//]: # (??? "EE_INLINE int ee_array_empty&#40;const Array* array&#41;")

[//]: # ()
[//]: # (    **Description**)

[//]: # (    )
[//]: # (    Checks whether the given array is empty.  )

[//]: # (    Returns non-zero &#40;`true`&#41; if the array currently contains no elements.)

[//]: # (    )
[//]: # (    **Parameters:**)

[//]: # (    )
[//]: # (    | Name   | Type         | Description |)

[//]: # (    |--------|--------------|--------------|)

[//]: # (    | array  | const Array* | Pointer to the array to check. |)

[//]: # (    )
[//]: # (    **Example:**)

[//]: # (    )
[//]: # (    ```c)

[//]: # (    if &#40;ee_array_empty&#40;&arr&#41;&#41; {)

[//]: # (        printf&#40;"Array is empty!\n"&#41;;)

[//]: # (    })

[//]: # (    ```)

[//]: # ()
[//]: # (??? "EE_INLINE size_t ee_array_len&#40;const Array* array&#41;")

[//]: # ()
[//]: # (    **Description**)

[//]: # (    )
[//]: # (    Returns the number of elements currently stored in the array.)

[//]: # (    )
[//]: # (    **Parameters:**)

[//]: # (    )
[//]: # (    | Name   | Type         | Description |)

[//]: # (    |--------|--------------|--------------|)

[//]: # (    | array  | const Array* | Pointer to the array whose element count is queried. |)

[//]: # (    )
[//]: # (    **Example:**)

[//]: # (    )
[//]: # (    ```c)

[//]: # (    size_t count = ee_array_len&#40;&arr&#41;;)

[//]: # (    printf&#40;"Array length: %zu\n", count&#41;;)

[//]: # (    ```)

[//]: # ()
[//]: # (??? "EE_INLINE size_t ee_array_size&#40;const Array* array&#41;")

[//]: # ()
[//]: # (    **Description**)

[//]: # (    )
[//]: # (    Returns the total number of bytes currently used by the array’s stored elements.  )

[//]: # (    This value equals `top`, which represents the current write position in bytes.)

[//]: # (    )
[//]: # (    **Parameters:**)

[//]: # (    )
[//]: # (    | Name   | Type         | Description |)

[//]: # (    |--------|--------------|--------------|)

[//]: # (    | array  | const Array* | Pointer to the array to query. |)

[//]: # (    )
[//]: # (    **Example:**)

[//]: # (    )
[//]: # (    ```c)

[//]: # (    size_t used_bytes = ee_array_size&#40;&arr&#41;;)

[//]: # (    printf&#40;"Used memory: %zu bytes\n", used_bytes&#41;;)

[//]: # (    ```)

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_free&#40;Array* array&#41;")

[//]: # ()
[//]: # (    **Description**)

[//]: # (    )
[//]: # (    Releases all memory used by the given array and resets its fields to zero.  )

[//]: # (    After calling this function, the `Array` structure becomes invalid for further use until reinitialized.  )

[//]: # (    The function uses the array’s internal allocator to free its buffer.)

[//]: # (    )
[//]: # (    **Parameters:**)

[//]: # (    )
[//]: # (    | Name  | Type   | Description |)

[//]: # (    |--------|--------|--------------|)

[//]: # (    | array  | Array* | Pointer to the array to free. Must not be `NULL`, and its buffer must be allocated. |)

[//]: # (    )
[//]: # (    **Example:**)

[//]: # (    )
[//]: # (    ```c)

[//]: # (    Array arr = ee_array_new&#40;32, sizeof&#40;int&#41;, NULL&#41;;)

[//]: # (    ee_array_push&#40;&arr, &#40;u8*&#41;&value&#41;;)

[//]: # (    )
[//]: # (    ee_array_free&#40;&arr&#41;; // Frees memory and resets the array)

[//]: # (    ```)

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_clear&#40;Array\* array&#41;")

[//]: # ()
[//]: # (    **Description**)

[//]: # (    )
[//]: # (    Resets the array’s content without freeing its allocated memory.  )

[//]: # (    This function simply sets the internal `top` pointer to zero, effectively marking the array as empty while preserving its capacity.  )

[//]: # (    It is useful when reusing an existing array without reallocating memory.)

[//]: # (    )
[//]: # (    **Parameters:**)

[//]: # (    )
[//]: # (    | Name  | Type   | Description |)

[//]: # (    |--------|--------|--------------|)

[//]: # (    | array  | Array* | Pointer to the array to clear. Must not be `NULL`, and its buffer must be valid. |)

[//]: # (    )
[//]: # (    **Example:**)

[//]: # (    )
[//]: # (    ```c)

[//]: # (    Array arr = ee_array_new&#40;16, sizeof&#40;int&#41;, NULL&#41;;)

[//]: # (    ee_array_push&#40;&arr, &#40;u8*&#41;&value&#41;;)

[//]: # (    )
[//]: # (    ee_array_clear&#40;&arr&#41;; // Array is now empty but still allocated)

[//]: # (    ```)

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_reserve&#40;Array\* array, size_t size&#41;")

[//]: # ()
[//]: # (    **Description**)

[//]: # (    )
[//]: # (    Expands the array’s capacity to accommodate at least `size` elements.  )

[//]: # (    The function reallocates the internal buffer using the array’s allocator if the requested capacity exceeds the current one.  )

[//]: # (    It does not modify existing elements or reduce the capacity if `size` is smaller than the current capacity.)

[//]: # (    )
[//]: # (    **Parameters:**)

[//]: # (    )
[//]: # (    | Name  | Type   | Description |)

[//]: # (    |--------|--------|--------------|)

[//]: # (    | array  | Array* | Pointer to the array whose capacity will be increased. Must not be `NULL`. |)

[//]: # (    | size   | size_t | New desired number of elements. Must be greater than the current capacity &#40;in elements&#41;. |)

[//]: # (    )
[//]: # (    **Example:**)

[//]: # (    )
[//]: # (    ```c)

[//]: # (    Array arr = ee_array_new&#40;8, sizeof&#40;int&#41;, NULL&#41;;)

[//]: # (    )
[//]: # (    // Grow the array to hold at least 32 elements)

[//]: # (    ee_array_reserve&#40;&arr, 32&#41;;)

[//]: # (    ```)

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_grow&#40;Array\* array&#41;")

[//]: # ()
[//]: # (    **Description**)

[//]: # (    )
[//]: # (    Dynamically increases the array’s capacity by 50% &#40;1.5× the current size&#41;.  )

[//]: # (    This function is typically used internally when inserting new elements into a full array.  )

[//]: # (    It preserves all existing data and uses the array’s allocator to reallocate the buffer.)

[//]: # (    )
[//]: # (    **Parameters:**)

[//]: # (    )
[//]: # (    | Name  | Type   | Description |)

[//]: # (    |--------|--------|--------------|)

[//]: # (    | array  | Array* | Pointer to the array to grow. Must not be `NULL`, and its buffer must already be allocated. |)

[//]: # (    )
[//]: # (    **Example:**)

[//]: # (    )
[//]: # (    ```c)

[//]: # (    Array arr = ee_array_new&#40;8, sizeof&#40;int&#41;, NULL&#41;;)

[//]: # (    while &#40;ee_array_full&#40;&arr&#41;&#41; {)

[//]: # (        ee_array_grow&#40;&arr&#41;; // Increase capacity by 1.5x)

[//]: # (    })

[//]: # (    ```)

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_push&#40;Array\* array, const u8\* val&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_push_zero&#40;Array\* array&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_push_nothing&#40;Array\* array&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE u8\* ee_array_top&#40;const Array\* array&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE u8\* ee_array_at&#40;const Array\* array, size_t i&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_pop&#40;Array\* array, u8\* out_val&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_set&#40;Array\* array, size_t i, const u8\* val&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE size_t ee_array_find_b&#40;const Array\* array, const u8\* target, size_t low, size_t high&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE size_t ee_array_find_pred_b&#40;const Array\* array, const u8\* target, BinCmp predicate, size_t low, size_t high&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE size_t ee_array_find_pred&#40;const Array\* array, const u8\* target, BinCmp predicate&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE size_t ee_array_min_pred_b&#40;const Array\* array, BinCmp predicate, size_t low, size_t high&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE size_t ee_array_max_pred_b&#40;const Array\* array, BinCmp predicate, size_t low, size_t high&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE size_t ee_array_min_pred&#40;const Array\* array, BinCmp predicate&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE size_t ee_array_max_pred&#40;const Array\* array, BinCmp predicate&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_insert&#40;Array\* array, size_t i, const u8\* val&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE size_t ee_array_find&#40;const Array\* array, const u8\* target&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_erase&#40;Array\* array, size_t i&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_swap&#40;Array\* array, size_t i, size_t j&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_insertsort&#40;Array\* array, BinCmp cmp, i64 low, i64 high&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_quicksort&#40;Array\* array, BinCmp cmp, i64 low, i64 high&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_heapsort&#40;Array\* array, BinCmp cmp, i64 low, i64 high&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_introsort&#40;Array\* array, BinCmp cmp, i64 low, i64 high, i32 max_depth&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_sort&#40;Array\* array, BinCmp cmp, ArraySortType type&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_fill&#40;Array\* array, const u8\* val, size_t a, size_t b&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE Array ee_array_copy&#40;Array\* array, Allocator\* allocator&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_reverse&#40;Array\* array&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_swap_n_pop&#40;Array\* array, size_t i, u8\* out_val&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE void ee_array_reset&#40;Array\* array&#41;")

[//]: # ()
[//]: # (??? "EE_INLINE u8\* ee_array_emplace&#40;Array\* array&#41;")
