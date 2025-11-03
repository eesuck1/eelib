# **ee_array**

`ee_array.h` provides a **type-generic dynamic array** (vector) implementation.

It defines the `Array` structure, which manages a contiguous, resizable memory buffer. This module is designed to be type-agnostic; it operates on raw bytes by tracking the `elem_size` (element size). All element manipulation (push, pop, set, at) is done via `memcpy` or direct byte-pointer access.

### **Structures**:

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

    | Members    | Type      | Description                                                                             |
    |-----------|-----------|-----------------------------------------------------------------------------------------|
    | top       | size_t    | The current size (length) of the array in bytes. (`top == ee_array_len() * elem_size`). |
    | cap       | size_t    | The total allocated capacity of the `buffer` in bytes.                                  |
    | elem_size | size_t    | The size of a single element in bytes (e.g., `sizeof(int)`).                            |
    | buffer    | u8\*      | Pointer to the contiguous memory block holding the elements.                            |
    | allocator | Allocator | The underlying allocator used for `buffer` (e.g., default malloc/free).                 |

### **Enumerations (enum)**:

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
    | EE_SORT_DEFAULT | Defaults to Introspective Sort (`EE_SORT_INTRO`).                                                                        |
    | EE_SORT_INSERT  | Insertion Sort. Efficient for small or nearly-sorted arrays.                                                             |
    | EE_SORT_QUICK   | Quicksort. Fast on average, but with O(n^2^) worst-case.                                                                 |
    | EE_SORT_HEAP    | Heapsort. Guaranteed O(n log n) performance.                                                                             |
    | EE_SORT_INTRO   | Introspective Sort. A hybrid that starts with Quicksort and switches to Heapsort to prevent worst-case O(n^2^) behavior. |


### **Functions**:


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
    
    !!! note "Advanced Use"
        This function is generally discouraged. Prefer [`ee_array_emplace()`](#ee_array_emplace), which does the same thing but also returns a pointer to the newly reserved space.
    
    **Parameters**
    
    | Name | Type | Description |
    | :--- | :--- | :--- |
    | `array` | `Array*` | Pointer to the `Array` to advance its `top` pointer. |
    
    **See Also**
    
    * [`ee_array_emplace()`](#ee_array_emplace)
    * [`ee_array_push()`](#ee_array_push)

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
    
    * `ee_array_at()`
    * `ee_array_pop()`

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
    
    * `ee_array_set()`
    * `ee_array_top()`

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
    ee_array_set(&int_array, 3, (const u8*)&new_val);
    ```
    
    **See Also**
    
    * [`ee_array_at()`](#ee_array_at)

??? "EE_INLINE size_t ee_array_find_b(const Array\* array, const u8\* target, size_t low, size_t high)"



??? "EE_INLINE size_t ee_array_find_pred_b(const Array\* array, const u8\* target, BinCmp predicate, size_t low, size_t high)"



??? "EE_INLINE size_t ee_array_find_pred(const Array\* array, const u8\* target, BinCmp predicate)"



??? "EE_INLINE size_t ee_array_min_pred_b(const Array\* array, BinCmp predicate, size_t low, size_t high)"



??? "EE_INLINE size_t ee_array_max_pred_b(const Array\* array, BinCmp predicate, size_t low, size_t high)"



??? "EE_INLINE size_t ee_array_min_pred(const Array\* array, BinCmp predicate)"



??? "EE_INLINE size_t ee_array_max_pred(const Array\* array, BinCmp predicate)"



??? "EE_INLINE void ee_array_insert(Array\* array, size_t i, const u8\* val)"



??? "EE_INLINE size_t ee_array_find(const Array\* array, const u8\* target)"



??? "EE_INLINE void ee_array_erase(Array\* array, size_t i)"



??? "EE_INLINE void ee_array_swap(Array\* array, size_t i, size_t j)"



??? "EE_INLINE void ee_array_insertsort(Array\* array, BinCmp cmp, i64 low, i64 high)"



??? "EE_INLINE void ee_array_quicksort(Array\* array, BinCmp cmp, i64 low, i64 high)"



??? "EE_INLINE void ee_array_heapsort(Array\* array, BinCmp cmp, i64 low, i64 high)"



??? "EE_INLINE void ee_array_introsort(Array\* array, BinCmp cmp, i64 low, i64 high, i32 max_depth)"



??? "EE_INLINE void ee_array_sort(Array\* array, BinCmp cmp, ArraySortType type)"



??? "EE_INLINE void ee_array_fill(Array\* array, const u8\* val, size_t a, size_t b)"



??? "EE_INLINE Array ee_array_copy(Array\* array, Allocator\* allocator)"

    
    <span id="ee_array_copy"></span>


    **Syntax**

    
    ```c

    Array ee_array_copy(Array* array, Allocator* allocator);

    ```

    
    **Description**

    
    Creates a deep copy of an existing array.

    
    A new buffer is allocated using the specified `allocator` (or default if `NULL`), and the *entire capacity* (`cap`) of the source array is copied, regardless of its current `top`.

    
    **Parameters**

    
    | Name | Type | Description |

    | :--- | :--- | :--- |

    | `array` | `Array*` | A pointer to the source array to copy. |

    | `allocator` | `Allocator*` | Optional. The allocator to use for the *new* array. Pass `NULL` for default. |

    
    **Returns**

    
    A new `Array` struct containing a copy of the source array's data.


??? "EE_INLINE void ee_array_reverse(Array\* array)"



??? "EE_INLINE void ee_array_swap_n_pop(Array\* array, size_t i, u8\* out_val)"



??? "EE_INLINE void ee_array_reset(Array\* array)"



??? "EE_INLINE u8\* ee_array_emplace(Array\* array)"
