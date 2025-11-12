# **ee_array.h**

`ee_array.h` provides a **type-generic dynamic array** (vector) implementation.

It defines the `Array` structure, which manages a contiguous, resizable memory buffer. This module is designed to be type-agnostic; it operates on raw bytes by tracking the `elem_size` (element size). All element manipulation (push, pop, set, at) is done via `memcpy` or direct byte-pointer access.

## Defines

This header provides several helper macros for constants and type-safe element access.

### Constants

| Macro              | Description                                                                                                                                                                                             |
|:-------------------|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `EE_ARRAY_INVALID` | A sentinel value (`(size_t)-1`) returned by search functions (e.g., [`ee_array_find()`](#ee_array_find)) when no matching element is found.                                                             |
| `EE_ARRAY_SORT_TH` | The size threshold (typically `16` elements) for [`ee_array_introsort()`](#ee_array_introsort). Sub-arrays smaller than or equal to this size will be sorted using the faster Insertion Sort algorithm. |

### Helper Macros

| Macro                               | Description                                                                                                                                                                                                                          |
|:------------------------------------|:-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `EE_ARRAY_RECAST(v_ptr, i, dtype)`  | **(Recommended)** Gets a correctly typed pointer (`dtype*`) to the element at index `i` from an array pointer `v_ptr`. This is the safest way to access elements. <br> **Example:** `int* val = EE_ARRAY_RECAST(&my_array, 0, int);` |
| `EE_ARRAY_PTR_GET(v_ptr, i, d_ptr)` | Copies the element at index `i` from an array pointer `v_ptr` *into* the destination pointer `d_ptr`. <br> **Example:** `int val; EE_ARRAY_PTR_GET(&my_array, 0, &val);`                                                             |
| `EE_ARRAY_GET(v, i, d)`             | Copies the element at index `i` from an array struct `v` *into* the destination variable `d`. <br> **Example:** `int val; EE_ARRAY_GET(my_array, 0, val);`                                                                           |

## Structures

Structure `struct Array` defines the core container for a resizable, contiguous array.
All array operations depend on these fields to manage allocation and indexing.

```c
typedef struct Array
{
    size_t top; 
    size_t cap; 
    size_t elem_size; 
    u8* buffer; 
    Allocator allocator; 
} Array;
```

??? "Structure members"
    
    !!! warning "Important" 
        Note that `top` and `cap` store the size in bytes, not element count. Use `ee_array_len()` to get the number of elements.

    | Members     | Type        | Description                                                                             |
    |:------------|:------------|:----------------------------------------------------------------------------------------|
    | `top`       | `size_t`    | The current size (length) of the array in bytes. (`top == ee_array_len() * elem_size`). |
    | `cap`       | `size_t`    | The total allocated capacity of the `buffer` in bytes.                                  |
    | `elem_size` | `size_t`    | The size of a single element in bytes (e.g., `sizeof(int)`).                            |
    | `buffer`    | `u8*`       | Pointer to the contiguous memory block holding the elements.                            |
    | `allocator` | `Allocator` | The underlying allocator used for `buffer` (e.g., default malloc/free).                 |

## Enumerations (enum)

<span id="arraysorttype"></span>

`enum ArraySortType` lists available sorting algorithms for internal operations.

```c
typedef enum ArraySortType
{
    EE_SORT_DEFAULT = 0,
    EE_SORT_INSERT  = 1,
    EE_SORT_QUICK   = 2,
    EE_SORT_HEAP    = 3,
    EE_SORT_INTRO   = 4,
} ArraySortType;
```

??? "Enum values"

    | Value           | Description                                                                                                              |
    |-----------------|--------------------------------------------------------------------------------------------------------------------------|
    | `EE_SORT_DEFAULT` | Defaults to Introspective Sort (`EE_SORT_INTRO`).                                                                        |
    | `EE_SORT_INSERT`  | Insertion Sort. Efficient for small or nearly-sorted arrays.                                                             |
    | `EE_SORT_QUICK`   | Quicksort. Fast on average, but with O(n^2^) worst-case.                                                                 |
    | `EE_SORT_HEAP`    | Heapsort. Guaranteed O(n log n) performance.                                                                             |
    | `EE_SORT_INTRO`   | Introspective Sort. A hybrid that starts with Quicksort and switches to Heapsort to prevent worst-case O(n^2^) behavior. |


## Functions (Lifecycle)

??? "EE_INLINE Array ee_array_new(size_t size, size_t elem_size, const Allocator* allocator)"

    <span id="ee_array_new"></span>

    **Syntax**
    
    ```c
    Array ee_array_new(size_t size, size_t elem_size, const Allocator* allocator);
    ```
    
    **Description**
    
    Initializes and allocates memory for a new `Array` object.
    
    This function allocates an initial buffer large enough to hold `size` elements. The `Array.top` is initialized to `0`, and `Array.cap` is set to `size * elem_size`.

    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `size` | `size_t` | The initial **element capacity** (number of elements, not bytes). Must be greater than 0. |
    | `elem_size` | `size_t` | The size of a single element. Use `sizeof(your_type)`. Must be greater than 0. |
    | `allocator` | `const Allocator*` | Optional. A pointer to a custom [`Allocator`](../Core/core.md#allocator). Pass `NULL` to use the default `ee_lib` allocator. |
    
    **Returns**
    
    An initialized `Array` structure.
    
    **Example**
    
    ```c
    // Create an array to hold 16 integers
    Array int_array = ee_array_new(16, sizeof(int), NULL);
    
    // ... use the array ...
    
    ee_array_free(&int_array);
    ```
    
    **See Also**
    
    * [`ee_array_free()`](#ee_array_free)

??? "EE_INLINE Array ee_array_copy(Array\* array, Allocator\* allocator)"

    <span id="ee_array_copy"></span>

    **Syntax**
    
    ```c
    Array ee_array_copy(Array* array, Allocator* allocator);
    ```
    
    **Description**
    
    Creates a **deep copy** of an existing array.
    
    A new `Array` struct is created, and a new memory buffer is allocated for it using the specified `allocator` (or the default if `NULL`).

    !!! note "Note"
        This function copies the **entire capacity (`cap`)** of the source array's buffer, not just the currently used portion (`top`). The `top` value of the new array is set to match the source array's `top`.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the *source* `Array` to be copied. |
    | `allocator` | `Allocator*` | **Optional.** The allocator to be used for the *new* array's buffer. Pass `NULL` to use the default allocator. |
    
    **Returns**
    
    A new `Array` struct containing a deep copy of the source array's data.
    
    **Example**
    
    ```c
    // Create an original array
    Array arr_a = ee_array_new(4, sizeof(int), NULL);
    int x = 5;
    ee_array_push(&arr_a, (u8*)&x); // arr_a has [5]
    
    // Create a copy
    Array arr_b = ee_array_copy(&arr_a, NULL);
    
    // Now, arr_a and arr_b are independent.
    // Let's modify arr_b.
    int y = 10;
    ee_array_push(&arr_b, (u8*)&y); // arr_b has [5, 10]
    
    // arr_a is unchanged (still has [5])
    
    // They have different memory buffers
    // EE_ASSERT(arr_a.buffer != arr_b.buffer);
    
    // Clean up both arrays
    ee_array_free(&arr_a);
    ee_array_free(&arr_b);
    ```
    
    **See Also**
    
    * [`ee_array_new()`](#ee_array_new)
    * [`ee_array_free()`](#ee_array_free)

??? "EE_INLINE void ee_array_free(Array\* array)"

    <span id="ee_array_free"></span>

    **Syntax**
    
    ```c
    void ee_array_free(Array* array);
    ```
    
    **Description**
    
    Frees the internal buffer of the array using its allocator and zeroes the `Array` struct. After this call, the array is no longer valid and must not be used.

    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to be freed and destroyed. |
    
    **See Also**
    
    * [`ee_array_new()`](#ee_array_new)

## Functions (Capacity & Size)

??? "EE_INLINE int ee_array_full(const Array\* array)"

    <span id="ee_array_full"></span>

    **Syntax**
    
    ```c
    int ee_array_full(const Array* array);
    ```
    
    **Description**
    
    Checks if the array's used size in bytes (`top`) has reached its total capacity (`cap`).
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `const Array*` | Pointer to the `Array` to check for fullness. |
    
    **Returns**
    
    `1` (true) if the array is full (`array->top >= array->cap`), `0` (false) otherwise.
    
    **See Also**
    
    * [`ee_array_grow()`](#ee_array_grow)
    * [`ee_array_push()`](#ee_array_push)

??? "EE_INLINE int ee_array_empty(const Array\* array)"

    <span id="ee_array_empty"></span>

    **Syntax**
    
    ```c
    int ee_array_empty(const Array* array);
    ```
    
    **Description**
    
    Checks if the array contains any elements.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `const Array*` | Pointer to the `Array` to check for emptiness. |
    
    **Returns**
    
    `1` (true) if the array is empty (`array->top == 0`), `0` (false) otherwise

??? "EE_INLINE size_t ee_array_len(const Array\* array)"

    <span id="ee_array_len"></span>

    **Syntax**
    
    ```c
    size_t ee_array_len(const Array* array);
    ```
    
    **Description**
    
    Gets the number of **elements** currently stored in the array.

    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `const Array*` | Pointer to the `Array` to query for its element count. |
    
    **Returns**
    
    The element count (i.e., `array->top / array->elem_size`).
    
    **See Also**
    
    * [`ee_array_size()`](#ee_array_size) (for byte size)

??? "EE_INLINE size_t ee_array_size(const Array\* array)"

    <span id="ee_array_size"></span>

    **Syntax**
    
    ```c
    size_t ee_array_size(const Array* array);
    ```
    
    **Description**
    
    Gets the total number of **bytes** currently used by the stored elements. This is equivalent to `array->top`.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `const Array*` | Pointer to the `Array` to query for its used byte size. |
    
    **Returns**
    
    The used size in bytes (i.e., `array->top`).

    **See Also**
    
    * [`ee_array_len()`](#ee_array_len) (for element count)

??? "EE_INLINE void ee_array_reserve(Array\* array, size_t size)"

    <span id="ee_array_reserve"></span>

    **Syntax**
    
    ```c
    void ee_array_reserve(Array* array, size_t size);
    ```
    
    **Description**
    
    Reallocates the array's buffer to ensure it can hold at least `size` elements.
    
    This function asserts if the new `size` (in elements) is not larger than the current capacity.

    !!! note "Note"
        Based on the code's assertion (`size * array->elem_size > array->cap`), this function can only be used to *increase* the capacity. It cannot be used to shrink the array.

    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to be resized. |
    | `size` | `size_t` | The desired new minimum **element capacity** (not bytes). |

??? "EE_INLINE void ee_array_grow(Array\* array)"

    <span id="ee_array_grow"></span>

    **Syntax**
    
    ```c
    void ee_array_grow(Array* array);
    ```
    
    **Description**
    
    Grows the array's capacity by a factor of 1.5x (i.e., `new_cap = cap + (cap >> 1)`).
    
    This function is called automatically by `ee_array_push` and other modifiers when the array is full.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to be grown. |
    
    **See Also**
    
    * [`ee_array_push()`](#ee_array_push)
    * [`ee_array_full()`](#ee_array_full)

## Functions (Element Access)

??? "EE_INLINE u8\* ee_array_at(const Array\* array, size_t i)"

    <span id="ee_array_at"></span>

    **Syntax**
    
    ```c
    u8* ee_array_at(const Array* array, size_t i);
    ```
    
    **Description**
    
    Gets a direct, raw pointer to the element at the specified **element index** `i`.
    
    !!! warning "Important"
        The returned pointer is a `u8*`. It is the user's responsibility to cast this pointer to the correct type. Use the `EE_ARRAY_RECAST` macro for a safer and cleaner way to do this.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `const Array*` | Pointer to the `Array` to access. |
    | `i` | `size_t` | The **element index** (not byte offset) to access. Asserts if out of bounds. |
    
    **Returns**
    
    A `u8*` pointer to the first byte of the element at index `i`.
    
    **Example**
    
    ```c
    // Using the macro (Recommended)
    int* my_int_ptr = EE_ARRAY_RECAST(&int_array, 2, int);
    *my_int_ptr = 100; // Modify the value in-place
    
    // Using raw access (Manual casting)
    int* raw_ptr = (int*)ee_array_at(&int_array, 2);
    printf("Value: %d\n", *raw_ptr); // Prints 100
    ```
    
    **See Also**
    
    * [`ee_array_set()`](#ee_array_set)
    * [`ee_array_top()`](#ee_array_top)

??? "EE_INLINE u8\* ee_array_top(const Array\* array)"

    <span id="ee_array_top"></span>

    **Syntax**
    
    ```c
    u8* ee_array_top(const Array* array);
    ```
    
    **Description**
    
    Gets a direct, raw pointer to the **last** valid element in the array.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `const Array*` | Pointer to the `Array` to get the top element from. |
    
    **Returns**
    
    A `u8*` pointer to the last element, or `NULL` if the array is empty.
    
    **Example**
    
    ```c
    // Assumes 'int_array' is not empty
    int* last_val = (int*)ee_array_top(&int_array);
    if (last_val) 
    {
        printf("Last element: %d\n", *last_val);
    }
    ```
    
    **See Also**
    
    * [`ee_array_at()`](#ee_array_at)
    * [`ee_array_pop()`](#ee_array_pop)

## Functions (Modifiers)

??? "EE_INLINE void ee_array_push(Array\* array, const u8\* val)"
    
    <span id="ee_array_push"></span>

    **Syntax**
    
    ```c
    void ee_array_push(Array* array, const u8* val);
    ```
    
    **Description**
    
    Appends a new element to the end of the array by copying the data from `val`.
    
    If the array is full (`ee_array_full()` is `true`), this function will automatically call `ee_array_grow()` to expand the capacity before pushing the new element.

    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to push onto. |
    | `val` | `const u8*` | A pointer to the element data to be copied *into* the array. |
    
    **Example**
    
    ```c
    // Assumes 'int_array' is an Array with elem_size = sizeof(int)
    int x = 123;
    ee_array_push(&int_array, (const u8*)&x);
    
    // Assumes 'struct_array' is an Array with elem_size = sizeof(MyStruct)
    MyStruct s = { .a = 1, .b = 2 };
    ee_array_push(&struct_array, (const u8*)&s);
    ```
    
    **See Also**
    
    * [`ee_array_grow()`](#ee_array_grow)
    * [`ee_array_push_zero()`](#ee_array_push_zero)
    * [`ee_array_emplace()`](#ee_array_emplace)

??? "EE_INLINE void ee_array_push_zero(Array\* array)"

    <span id="ee_array_push_zero"></span>

    **Syntax**
    
    ```c
    void ee_array_push_zero(Array* array);
    ```
    
    **Description**
    
    Appends a new, **zero-initialized** element to the end of the array.
    
    Calls `ee_array_grow()` if the array is full. This is useful for reserving an element and ensuring it has a clean default state.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to push onto. |
    
    **See Also**
    
    * [`ee_array_push()`](#ee_array_push)
    * [`ee_array_emplace()`](#ee_array_emplace)

??? "EE_INLINE void ee_array_push_nothing(Array\* array)"

    <span id="ee_array_push_nothing"></span>

    **Syntax**
    
    ```c
    void ee_array_push_nothing(Array* array);
    ```
    
    **Description**
    
    Advances the array's `top` pointer by one `elem_size`, effectively reserving space for a new element without initializing its memory.
    
    Calls `ee_array_grow()` if the array is full.
    
    !!! note "Note"
        This function is generally discouraged. Prefer [`ee_array_emplace()`](#ee_array_emplace), which does the same thing but also returns a pointer to the newly reserved space.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to advance its `top` pointer. |
    
    **See Also**
    
    * [`ee_array_emplace()`](#ee_array_emplace)
    * [`ee_array_push()`](#ee_array_push)

??? "EE_INLINE u8\* ee_array_emplace(Array\* array)"

    <span id="ee_array_emplace"></span>

    **Syntax**
    
    ```c
    u8* ee_array_emplace(Array* array);
    ```
    
    **Description**
    
    Appends space for one new element and returns a direct pointer to that new slot.
    
    This is the **preferred** way to add a new element if you want to initialize it in-place without a separate `memcpy` (which [`ee_array_push()`](#ee_array_push) does).
    
    Calls `ee_array_grow()` if the array is full.
    
    !!! note "Note"
        This function returns a pointer **to** the *beginning* of the new element's slot; the memory at that location is **uninitialized**.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to emplace into. |
    
    **Returns**
    
    A `u8*` pointer to the newly reserved (but uninitialized) element slot.
    
    **Example**
    
    ```c
    // Emplace a new int and initialize it
    int* new_int = (int*)ee_array_emplace(&int_array);
    *new_int = 42;
    
    // Emplace a new struct and initialize it
    MyStruct* new_s = (MyStruct*)ee_array_emplace(&struct_array);
    new_s->name = "Example";
    new_s->id = 123;
    
    // Array now contains the fully initialized struct
    ```
    
    **See Also**
    
    * [`ee_array_push()`](#ee_array_push) (alternative that takes a pointer to data)
    * [`ee_array_push_nothing()`](#ee_array_push_nothing) (discouraged alternative)

??? "EE_INLINE void ee_array_pop(Array\* array, u8\* out_val)"

    <span id="ee_array_pop"></span>

    **Syntax**
    
    ```c
    void ee_array_pop(Array* array, u8* out_val);
    ```
    
    **Description**
    
    Removes the last element from the array by decrementing the `top` pointer.
    
    If `out_val` is not `NULL`, the element's data is copied into `out_val` *before* it is "removed" (i.e., before `top` is moved).
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to pop from. Asserts if empty. |
    | `out_val` | `u8*` | **Optional.** A pointer to a destination buffer to copy the popped element into. Pass `NULL` to discard the element. |
    
    **Example**
    
    ```c
    int last_val;
    // Pop the last int and store it in last_val
    ee_array_pop(&int_array, (u8*)&last_val);
    
    // Pop the last element and discard it
    ee_array_pop(&int_array, NULL);
    ```
    
    **See Also**
    
    * [`ee_array_push()`](#ee_array_push)
    * [`ee_array_top()`](#ee_array_top)

??? "EE_INLINE void ee_array_set(Array\* array, size_t i, const u8\* val)"

    <span id="ee_array_set"></span>

    **Syntax**
    
    ```c
    void ee_array_set(Array* array, size_t i, const u8* val);
    ```
    
    **Description**
    
    Overwrites the element at the specified index `i` by copying new data from `val`.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to modify. |
    | `i` | `size_t` | The **element index** to modify. Asserts if out of bounds. |
    | `val` | `const u8*` | A pointer to the new element data to be copied *into* the array. |
    
    **Example**
    
    ```c
    int new_val = 777;
    // Set the element at index 3 to 777
    ee_array_set(&int_array, 3, (EE_RECAST_U8)new_val);
    ```
    
    **See Also**
    
    * [`ee_array_at()`](#ee_array_at)

??? "EE_INLINE void ee_array_insert(Array\* array, size_t i, const u8\* val)"

    <span id="ee_array_insert"></span>

    **Syntax**
    
    ```c
    void ee_array_insert(Array* array, size_t i, const u8* val);
    ```
    
    **Description**
    
    Inserts an element at index `i`, shifting all subsequent elements one position to the right.
    
    If the array is full, it will be grown via `ee_array_grow()` *before* the insertion.
    
    !!! warning "Performance"
        This is an O(n) operation (slow for large arrays) because it uses `memmove` to shift all elements after index `i`.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to insert into. |
    | `i` | `size_t` | The **element index** at which to insert. Asserts if `i > ee_array_len()`. |
    | `val` | `const u8*` | A pointer to the new element data to be copied *into* the array. |
    
    **See Also**
    
    * [`ee_array_push()`](#ee_array_push) (for O(1) insertion at the end)
    * [`ee_array_erase()`](#ee_array_erase)

??? "EE_INLINE void ee_array_erase(Array\* array, size_t i)"

    <span id="ee_array_erase"></span>

    **Syntax**
    
    ```c
    void ee_array_erase(Array* array, size_t i);
    ```

    **Description**
    
    Removes the element at index `i`, shifting all subsequent elements one position to the left to fill the gap.
    
    !!! warning "Important"
        This is an O(n) operation (slow for large arrays) because it uses `memmove` to shift all elements after index `i`.
        
    !!! info "Fast Unordered Erase"
        If you need to remove an element and **do not** care about preserving the order of the array, use [`ee_array_swap_n_pop()`](#ee_array_swap_n_pop) instead. It runs in O(1) time.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to erase from. |
    | `i` | `size_t` | The **element index** to remove. Asserts if out of bounds. |
    
    **See Also**
    
    * [`ee_array_swap_n_pop()`](#ee_array_swap_n_pop) (Fast O(1) alternative)
    * [`ee_array_pop()`](#ee_array_pop)
    * [`ee_array_clear()`](#ee_array_clear)

??? "EE_INLINE void ee_array_swap(Array\* array, size_t i, size_t j)"

    <span id="ee_array_swap"></span>

    **Syntax**
    
    ```c
    void ee_array_swap(Array* array, size_t i, size_t j);
    ```
    
    **Description**
    
    Swaps the elements at index `i` and index `j` in-place.
    
    This function uses `EE_ALLOCA` to allocate a temporary buffer on the stack for the swap.

    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to modify. |
    * `i` | `size_t` | The **element index** of the first element. |
    | `j` | `size_t` | The **element index** of the second element. |
    
    **See Also**
    
    * [`ee_array_sort()`](#ee_array_sort)

??? "EE_INLINE void ee_array_swap_n_pop(Array\* array, size_t i, u8\* out_val)"

    <span id="ee_array_swap_n_pop"></span>

    **Syntax**
    
    ```c
    void ee_array_swap_n_pop(Array* array, size_t i, u8* out_val);
    ```
    
    **Description**
    
    Removes the element at index `i` by overwriting it with the **last** element in the array, and then decrementing the `top` pointer.
    
    This is an extremely fast **O(1)** removal operation.

    !!! warning "Important"
        This function **will break the order** of your array if the removed element is not the last element. Use [`ee_array_erase()`](#ee_array_erase) if you need to preserve the order of other elements (which is a slow O(n) operation).
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to modify. |
    | `i` | `size_t` | The **element index** to remove. Asserts if out of bounds. |
    | `out_val` | `u8*` | **Optional.** A pointer to a destination buffer to copy the *original* element at index `i` before it is overwritten. Pass `NULL` to discard. |
    
    **Example**
    
    ```c
    // Array content: [10, 20, 30, 40, 50]
    // We want to remove index 1 (value 20)
    
    int removed_val;
    ee_array_swap_n_pop(&int_array, 1, (u8*)&removed_val);
    
    // removed_val is now 20
    // Array content is now: [10, 50, 30, 40]
    // (Note that 50 moved to index 1, and 20 is gone)
    ```

    **See Also**

    * [`ee_array_erase()`](#ee_array_erase) (Slow, order-preserving alternative)
    * [`ee_array_pop()`](#ee_array_pop)

??? "EE_INLINE void ee_array_fill(Array\* array, const u8\* val, size_t a, size_t b)"

    <span id="ee_array_fill"></span>

    **Syntax**
    
    ```c
    void ee_array_fill(Array* array, const u8* val, size_t a, size_t b);
    ```
    
    **Description**
    
    Fills a range of the array `[a, b)` with copies of the element data from `val`.
    
    This function iterates from the **element index** `a` up to (but not including) `b`, copying the data from `val` into each slot.

    !!! note "Note"
        This function operates within the array's *capacity* (`cap`). If the fill range `b` extends beyond the current array size `top`, `top` will be updated to `b * elem_size`. This allows `ee_array_fill` to be used to initialize portions of the array's reserved capacity.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to be filled. |
    | `val` | `const u8*` | A pointer to the element data that will be copied *into* each slot in the range. |
    | `a` | `size_t` | The starting **element index** (inclusive) of the range to fill. |
    | `b` | `size_t` | The ending **element index** (exclusive) of the range to fill. Asserts if `a >= b` or range is outside the *capacity*. |
    
    **Example**
    
    ```c
    // Create an array with capacity for 10 ints, but size 0
    Array int_array = ee_array_new(10, sizeof(int), NULL);
    
    int val_a = 7;
    // Fill the first 5 elements with '7'.
    // This also sets ee_array_len() to 5.
    ee_array_fill(&int_array, (u8*)&val_a, 0, 5);
    
    // ee_array_len(&int_array) is now 5
    
    int val_b = -1;
    // Overwrite elements at index 2 and 3 with '-1'
    ee_array_fill(&int_array, (u8*)&val_b, 2, 4);
    
    // Array content: [7, 7, -1, -1, 7]
    ```
    
    **See Also**
    
    * [`ee_array_set()`](#ee_array_set)
    * [`ee_array_clear()`](#ee_array_clear)

??? "EE_INLINE void ee_array_reverse(Array\* array)"

    <span id="ee_array_reverse"></span>

    **Syntax**
    
    ```c
    void ee_array_reverse(Array* array);
    ```
    
    **Description**
    
    Reverses the order of all elements in the array in-place.
    
    It works by iterating from the start to the middle of the array (`len / 2`) and swapping each element `i` with its counterpart `len - 1 - i`.

    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to be reversed. |
    
    **Example**
    
    ```c
    // Array content: [10, 20, 30, 40]
    ee_array_reverse(&int_array);
    // Array content is now: [40, 30, 20, 10]
    ```
    
    **See Also**
    
    * [`ee_array_swap()`](#ee_array_swap)

??? "EE_INLINE void ee_array_clear(Array\* array)"

    <span id="ee_array_clear"></span>

    **Syntax**

    ```c
    void ee_array_clear(Array* array);
    ```

    **Description**

    Removes all elements from the array by setting its `top` (byte count) to `0`.

    !!! warning "Important"
        This function does **not** free or zero the underlying memory buffer. It only resets the element count, allowing the buffer to be reused.

    **Parameters**

    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to be cleared. |

    **See Also**

    * [`ee_array_reset()`](#ee_array_reset)
    * [`ee_array_free()`](#ee_array_free)

??? "EE_INLINE void ee_array_reset(Array\* array)"

    <span id="ee_array_reset"></span>

    **Syntax**
    
    ```c
    void ee_array_reset(Array* array);
    ```
    
    **Description**
    
    Resets the array's element count to zero by setting `top = 0`. This is identical in function to [`ee_array_clear()`](#ee_array_clear).
    
    !!! warning "Important"
        This function does **not** free or zero the underlying memory buffer. It only resets the element count, allowing the buffer to be reused.

    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to be reset. |
    
    **See Also**
    
    * [`ee_array_clear()`](#ee_array_clear)
    * [`ee_array_free()`](#ee_array_free)

## Functions (Search)

??? "EE_INLINE size_t ee_array_find(const Array\* array, const u8\* target)"

    <span id="ee_array_find"></span>

    **Syntax**
    
    ```c
    size_t ee_array_find(const Array* array, const u8* target);
    ```
    
    **Description**
    
    Finds the first occurrence of `target` in the **entire** array.
    
    This is a convenience wrapper for [`ee_array_find_b(array, target, 0, ee_array_len(array))`](#ee_array_find_b). It benefits from the same SIMD optimizations as `ee_array_find_b`.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `const Array*` | Pointer to the `Array` to be searched. |
    | `target` | `const u8*` | A pointer to the element data to search for. |
    
    **Returns**
    
    The **index** of the first matching element, or [`EE_ARRAY_INVALID`](#defines) if not found.
    
    **See Also**
    
    * [`ee_array_find_b()`](#ee_array_find_b)
    * [`ee_array_find_pred()`](#ee_array_find_pred)

??? "EE_INLINE size_t ee_array_find_b(const Array\* array, const u8\* target, size_t low, size_t high)"

    <span id="ee_array_find_b"></span>

    **Syntax**
    
    ```c
    size_t ee_array_find_b(const Array* array, const u8* target, size_t low, size_t high);
    ```
    
    **Description**
    
    Finds the first occurrence of `target` within the **element index** range `[low, high)`.
    
    This function is heavily optimized using SIMD instructions for element sizes of 1, 2, 4, and 8 bytes. It performs the search in three phases:
    
    1.  **Head:** Linearly checks unaligned elements from `low` up to the first SIMD-aligned boundary.
    2.  **Body:** Scans the main, aligned portion of the array in large (e.g., 16/32-byte) SIMD chunks.
    3.  **Tail:** Linearly checks any remaining elements after the last aligned chunk up to `high`.
    
    For all other `elem_size` values, it falls back to a standard `memcmp` loop.

    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `const Array*` | Pointer to the `Array` to be searched. |
    | `target` | `const u8*` | A pointer to the element data to search for. |
    | `low` | `size_t` | The starting **element index** (inclusive). |
    | `high` | `size_t` | The ending **element index** (exclusive). Asserts if `low >= high`. |
    
    **Returns**
    
    The **index** of the first matching element, or [`EE_ARRAY_INVALID`](#defines) if not found.
    
    **See Also**
    
    * [`ee_array_find()`](#ee_array_find) (wrapper for this function)
    * [`ee_array_find_pred_b()`](#ee_array_find_pred_b)

??? "EE_INLINE size_t ee_array_find_pred(const Array\* array, const u8\* target, BinCmp predicate)"

    <span id="ee_array_find_pred"></span>

    **Syntax**
    
    ```c
    size_t ee_array_find_pred(const Array* array, const u8* target, BinCmp predicate);
    ```
    
    **Description**
    
    Finds the first element in the **entire** array that matches `target` according to a custom `predicate` function.
    
    This is a convenience wrapper for [`ee_array_find_pred_b(array, target, predicate, 0, ee_array_len(array))`](#ee_array_find_pred_b).
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `const Array*` | Pointer to the `Array` to be searched. |
    | `target` | `const u8*` | A pointer to the data to compare against. This is passed as the **first** argument to `predicate`. |
    | `predicate` | `BinCmp` | A function pointer (`int (*)(const void*, const void*)`). The function should return `0` for a match. |
    
    **Returns**
    
    The **index** of the first matching element, or [`EE_ARRAY_INVALID`](#defines) if not found.

??? "EE_INLINE size_t ee_array_find_pred_b(const Array\* array, const u8\* target, BinCmp predicate, size_t low, size_t high)"

    <span id="ee_array_find_pred_b"></span>

    **Syntax**

    ```c
    size_t ee_array_find_pred_b(const Array* array, const u8* target, BinCmp predicate, size_t low, size_t high);
    ```

    **Description**

    Finds the first element in the index range `[low, high)` that matches `target` according to a custom `predicate` function.

    The search stops at the first element where `predicate(target, element)` returns `0`.

    **Parameters**

    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `const Array*` | Pointer to the `Array` to be searched. |
    | `target` | `const u8*` | A pointer to the data to compare against. This is passed as the **first** argument to `predicate`. |
    | `predicate` | `BinCmp` | A function pointer (`int (*)(const void*, const void*)`). The function should return `0` for a match. |
    | `low` | `size_t` | The starting **element index** (inclusive). |
    | `high` | `size_t` | The ending **element index** (exclusive). Asserts if `low >= high`. |

    **Returns**

    The **index** of the first matching element, or [`EE_ARRAY_INVALID`](#defines) if not found.

    **See Also**

    * [`ee_array_find_pred()`](#ee_array_find_pred)
    * [`ee_array_find_b()`](#ee_array_find_b)

??? "EE_INLINE size_t ee_array_min_pred(const Array\* array, BinCmp predicate)"

    <span id="ee_array_min_pred"></span>

    **Syntax**
    
    ```c
    size_t ee_array_min_pred(const Array* array, BinCmp predicate);
    ```
    
    **Description**
    
    Finds the index of the **minimum** element in the **entire** array, according to a `predicate`.
    
    This is a convenience wrapper for [`ee_array_min_pred_b(array, predicate, 0, ee_array_len(array))`](#ee_array_min_pred_b).
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `const Array*` | Pointer to the `Array` to be searched. |
    | `predicate` | `BinCmp` | The comparison function. It should return `< 0` if the first argument is "less than" the second. |
    
    **Returns**
    
    The **index** of the minimum element.
    
    **See Also**
    
    * [`ee_array_min_pred_b()`](#ee_array_min_pred_b)
    * [`ee_array_max_pred()`](#ee_array_max_pred)

??? "EE_INLINE size_t ee_array_min_pred_b(const Array\* array, BinCmp predicate, size_t low, size_t high)"

    <span id="ee_array_min_pred_b"></span>

    **Syntax**
    
    ```c
    size_t ee_array_min_pred_b(const Array* array, BinCmp predicate, size_t low, size_t high);
    ```
    
    **Description**
    
    Finds the index of the **minimum** element within the index range `[low, high)`.
    
    The comparison is performed by the `predicate` function. An element `i` is considered the new minimum if `predicate(element_i, current_min)` returns `< 0`.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `const Array*` | Pointer to the `Array` to be searched. |
    | `predicate` | `BinCmp` | The comparison function. It should return `< 0` if the first argument is "less than" the second. |
    | `low` | `size_t` | The starting **element index** (inclusive). |
    | `high` | `size_t` | The ending **element index** (exclusive). Asserts if `low >= high`. |
    
    **Returns**
    
    The **index** of the minimum element found in the range. (Returns `low` if the range contains only one element).
    
    **See Also**
    
    * [`ee_array_max_pred_b()`](#ee_array_max_pred_b)
    * [`ee_array_sort()`](#ee_array_sort)

??? "EE_INLINE size_t ee_array_max_pred(const Array\* array, BinCmp predicate)"

    <span id="ee_array_max_pred"></span>

    **Syntax**
    
    ```c
    size_t ee_array_max_pred(const Array* array, BinCmp predicate);
    ```
    
    **Description**
    
    Finds the index of the **maximum** element in the **entire** array, according to a `predicate`.
    
    This is a convenience wrapper for [`ee_array_max_pred_b(array, predicate, 0, ee_array_len(array))`](#ee_array_max_pred_b).
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `const Array*` | Pointer to the `Array` to be searched. |
    | `predicate` | `BinCmp` | The comparison function. It should return `> 0` if the first argument is "greater than" the second. |
    
    **Returns**
    
    The **index** of the maximum element.
    
    **See Also**
    
    * [`ee_array_max_pred_b()`](#ee_array_max_pred_b)
    * [`ee_array_min_pred()`](#ee_array_min_pred)

??? "EE_INLINE size_t ee_array_max_pred_b(const Array\* array, BinCmp predicate, size_t low, size_t high)"

    <span id="ee_array_max_pred_b"></span>

    **Syntax**

    ```c
    size_t ee_array_max_pred_b(const Array* array, BinCmp predicate, size_t low, size_t high);
    ```

    **Description**

    Finds the index of the **maximum** element within the index range `[low, high)`.

    The comparison is performed by the `predicate` function. An element `i` is considered the new maximum if `predicate(element_i, current_max)` returns `> 0`.

    **Parameters**

    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `const Array*` | Pointer to the `Array` to be searched. |
    | `predicate` | `BinCmp` | The comparison function. It should return `> 0` if the first argument is "greater than" the second. |
    | `low` | `size_t` | The starting **element index** (inclusive). |
    | `high` | `size_t` | The ending **element index** (exclusive). Asserts if `low >= high`. |

    **Returns**

    The **index** of the maximum element found in the range. (Returns `low` if the range contains only one element).

    **See Also**

    * [`ee_array_min_pred_b()`](#ee_array_min_pred_b)
    * [`ee_array_sort()`](#ee_array_sort)

## Functions (Sorting)

??? "EE_INLINE void ee_array_sort(Array\* array, BinCmp cmp, ArraySortType type)"

    <span id="ee_array_sort"></span>

    **Syntax**
    
    ```c
    void ee_array_sort(Array* array, BinCmp cmp, ArraySortType type);
    ```
    
    **Description**
    
    Sorts the **entire** array in-place using the specified algorithm and comparison function.
    
    This is the main, user-facing sort function. It acts as a wrapper that calls the appropriate low-level sorting implementation (e.g., [`ee_array_introsort()`](#ee_array_introsort)), handling the conversion from element indices to the byte offsets required by the internal functions.

    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to be sorted. |
    | `cmp` | `BinCmp` | The comparison function `int (*)(const void* a, const void* b)`. It must return: <br>• **`< 0`** if `a < b` <br>• **`0`** if `a == b` <br>• **`> 0`** if `a > b` |
    | `type` | `ArraySortType` | The sorting algorithm to use. [`EE_SORT_DEFAULT`](#arraysorttype) (Introsort) is highly recommended for most cases. |
    
    **Example**
    
    ```c
    // 1. Define an Array of integers
    Array int_array = ee_array_new(10, sizeof(int), NULL);
    // ... (push some integers into the array) ...

    // 2. Define a comparison function (BinCmp) for integers
    int compare_ints(const void* a, const void* b) 
    {
        int val_a = *(const int*)a;
        int val_b = *(const int*)b;
        
        // Return (a > b) - (a < b) for a simple, fast comparison
        return (val_a > val_b) - (val_a < val_b);
        
        /* // A more verbose but equivalent way:
        if (val_a < val_b) return -1;
        if (val_a > val_b) return 1;
        return 0;
        */
    }
    
    // 3. Call the sort function
    ee_array_sort(&int_array, compare_ints, EE_SORT_DEFAULT);
    
    // ... int_array is now sorted ...
    ```
    
    **See Also**
    
    * [`ArraySortType`](#arraysorttype) (enum)
    * [`ee_array_introsort()`](#ee_array_introsort)
    * [`ee_array_heapsort()`](#ee_array_heapsort)
    * [`ee_array_quicksort()`](#ee_array_quicksort)
    * [`ee_array_insertsort()`](#ee_array_insertsort)

## Internal Sort Implementations

These functions are low-level helpers called by [`ee_array_sort()`](#ee_array_sort).

!!! note "Note"
    For most use cases, calling the main [`ee_array_sort()`](#ee_array_sort) function is **highly recommended**, as it operates on element indices.

    The functions in this section are low-level helpers. They expect their `low` and `high` range parameters as **byte offsets**, which can be less intuitive and error-prone if used directly.

??? "EE_INLINE void ee_array_insertsort(Array\* array, BinCmp cmp, i64 low, i64 high)"

    <span id="ee_array_insertsort"></span>

    **Syntax**
    
    ```c
    void ee_array_insertsort(Array* array, BinCmp cmp, i64 low, i64 high);
    ```
    
    **Description**
    
    Sorts a sub-section of the array using the **Insertion Sort** algorithm.
    
    Insertion sort iterates through the array, taking one element at a time and inserting it into its correct position within the already-sorted portion of the array.
    
    This algorithm is O(n^2^) but is very efficient for small arrays (see `EE_ARRAY_SORT_TH`) or for arrays that are already mostly sorted. It is used as the final step in `ee_array_introsort`.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to be sorted. |
    | `cmp` | `BinCmp` | The comparison function. Must return `< 0` if `a < b`, `0` if `a == b`, or `> 0` if `a > b`. |
    | `low` | `i64` | The **byte offset** of the *first* element in the range (inclusive). |
    | `high` | `i64` | The **byte offset** of the *last* element in the range (inclusive). |
    
    **See Also**
    
    * [`ee_array_sort()`](#ee_array_sort)
    * [`ee_array_introsort()`](#ee_array_introsort)

??? "EE_INLINE void ee_array_quicksort(Array\* array, BinCmp cmp, i64 low, i64 high)"

    <span id="ee_array_quicksort"></span>

    **Syntax**
    
    ```c
    void ee_array_quicksort(Array* array, BinCmp cmp, i64 low, i64 high);
    ```
    
    **Description**
    
    Sorts a sub-section of the array using the **Quicksort** algorithm.
    
    Quicksort is a divide-and-conquer algorithm. This implementation works as follows:
    1.  Selects the middle element as the "pivot".
    2.  Partitions the array (using a Hoare-like scheme) so that all elements smaller than the pivot are to its left, and all elements larger are to its right.
    3.  Recursively calls itself on the two sub-arrays (partitions).
    
    This algorithm has an average-case performance of O(n log n), but a worst-case performance of O(n^2^), which `ee_array_introsort` is designed to prevent.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to be sorted. |
    | `cmp` | `BinCmp` | The comparison function. Must return `< 0` if `a < b`, `0` if `a == b`, or `> 0` if `a > b`. |
    | `low` | `i64` | The **byte offset** of the *first* element in the range (inclusive). |
    | `high` | `i64` | The **byte offset** of the *last* element in the range (inclusive). |
    
    **See Also**
    
    * [`ee_array_sort()`](#ee_array_sort)
    * [`ee_array_introsort()`](#ee_array_introsort)

??? "EE_INLINE void ee_array_heapsort(Array\* array, BinCmp cmp, i64 low, i64 high)"

    <span id="ee_array_heapsort"></span>

    **Syntax**
    
    ```c
    void ee_array_heapsort(Array* array, BinCmp cmp, i64 low, i64 high);
    ```
    
    **Description**
    
    Sorts a sub-section of the array using the **Heapsort** algorithm.
    
    Heapsort is an in-place, comparison-based algorithm that works in two phases:
    1.  **Build Max-Heap:** It first rearranges the array into a "max-heap", which is a binary tree structure (represented linearly in the array) where every parent node is larger than or equal to its children.
        
    2.  **Sortdown:** It repeatedly swaps the root element (the largest item) with the last element in the heap, reduces the heap's size by one, and "sifts down" the new root to maintain the max-heap property.
        
    
    This algorithm has a guaranteed worst-case and average-case performance of O(n log n). It is used by `ee_array_introsort` to avoid the O(n^2^) worst-case of Quicksort when the recursion depth becomes too large.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to be sorted. |
    | `cmp` | `BinCmp` | The comparison function. Must return `< 0` if `a < b`, `0` if `a == b`, or `> 0` if `a > b`. |
    | `low` | `i64` | The **byte offset** of the *first* element in the range (inclusive). |
    | `high` | `i64` | The **byte offset** of the *last* element in the range (inclusive). |
    
    **See Also**
    
    * [`ee_array_sort()`](#ee_array_sort)
    * [`ee_array_introsort()`](#ee_array_introsort)

??? "EE_INLINE void ee_array_introsort(Array\* array, BinCmp cmp, i64 low, i64 high, i32 max_depth)"

    <span id="ee_array_introsort"></span>

    **Syntax**
    
    ```c
    void ee_array_introsort(Array* array, BinCmp cmp, i64 low, i64 high, i32 max_depth);
    ```
    
    **Description**
    
    Sorts a sub-section of the array using the **Introspective Sort** (Introsort) algorithm.
    
    Introsort is a hybrid algorithm that combines the speed of Quicksort with the guaranteed worst-case O(n log n) performance of Heapsort.
    
    This implementation's behavior depends on the partition size and recursion depth:
    
    1.  **Quicksort (Default):** It begins by performing a standard Quicksort partition.
    2.  **Heapsort (Fallback):** If the recursion `max_depth` (used to detect bad pivot choices) reaches zero, it switches to [`ee_array_heapsort()`](#ee_array_heapsort) to prevent Quicksort's O(n^2^) worst-case.
    3.  **Insertion Sort (Base Case):** If a partition's size (`len`) becomes smaller than or equal to [`EE_ARRAY_SORT_TH`](#defines), it switches to [`ee_array_insertsort()`](#ee_array_insertsort), which is more efficient for small arrays.
    
    This hybrid approach provides a fast average-case sort while completely avoiding quadratic performance. It is the default algorithm used by [`ee_array_sort()`](#ee_array_sort).
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to be sorted. |
    | `cmp` | `BinCmp` | The comparison function. Must return `< 0` if `a < b`, `0` if `a == b`, or `> 0` if `a > b`. |
    | `low` | `i64` | The **byte offset** of the *first* element in the range (inclusive). |
    | `high` | `i64` | The **byte offset** of the *last* element in the range (inclusive). |
    | `max_depth` | `i32` | The recursion depth limit. When this hits `0`, the algorithm switches to Heapsort. |
    
    **See Also**
    
    * [`ee_array_sort()`](#ee_array_sort)
    * [`ee_array_quicksort()`](#ee_array_quicksort)
    * [`ee_array_heapsort()`](#ee_array_heapsort)
    * [`ee_array_insertsort()`](#ee_array_insertsort)
