# **ee_array**

The `ee_array.h` defines a generic, dynamically resizable array implementation with customizable memory management.  
It provides low-level operations for allocation, resizing, element access, and sorting, built around an explicit memory model using a user-defined or default `Allocator`.

### **Structures**:

Structure `struct Array` defines the core container for a resizable, contiguous array.
All array operations depend on these fields to manage allocation and indexing.

```c
typedef struct Array 
{
    size_t top;          // Current used memory offset
    size_t cap;          // Total allocated capacity in bytes
    size_t elem_size;    // Size of each element
    u8* buffer;          // Pointer to data buffer
    Allocator allocator; // Memory allocator for dynamic operations
} Array;
```

Structure `enum ArraySortType` enumerates available sorting algorithms for internal operations.

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

### **Functions**:

??? "EE_INLINE Array ee_array_new(size_t size, size_t elem_size, const Allocator* allocator)"

    **Description:**
    
    Creates a new `Array` structure with the specified number of elements and element size.  
    If no allocator is provided, the default allocator is used.
    
    **Parameters:**
    
    | Name      | Type             | Description                               |
    |-----------|------------------|-------------------------------------------|
    | size      | size_t           | Number of elements to allocate initially. |
    | elem_size | size_t           | Size of each element in bytes.            |
    | allocator | const Allocator* | Custom allocator (can be `NULL`).         |
    
    **Example:**
    
    ```c
    Array arr = ee_array_new(32, sizeof(int), NULL);
    ```

??? "EE_INLINE void ee_array_free(Array* array)"

    **Description:**
    
    Frees the memory used by the array and resets its internal structure.  
    After this call, the array should not be used until reinitialized.
    
    **Parameters:**
    
    | Name  | Type   | Description                  |
    |--------|--------|------------------------------|
    | array  | Array* | Pointer to the array object. |
    
    **Example:**
    
    ```c
    ee_array_free(&arr);
    ```

??? "EE_INLINE int ee_array_full(const Array* array)"

    **Description:**
    
    Checks whether the array has reached its maximum capacity.
    
    **Parameters:**
    
    | Name  | Type         | Description              |
    |--------|--------------|--------------------------|
    | array  | const Array* | Pointer to the array.    |
    
    **Example:**
    
    ```c
    if (ee_array_full(&arr)) 
    {
        ee_array_grow(&arr);
    }
    ```

??? "EE_INLINE int ee_array_empty(const Array* array)"

    **Description:**
    
    Checks whether the array is empty.
    
    **Parameters:**
    
    | Name  | Type         | Description           |
    |-------|--------------|-----------------------|
    | array | const Array* | Pointer to the array. |

    **Example:**
    
    ```c
    if (ee_array_empty(&arr)) 
    {
        printf("Array is empty.\n");
    }
    ```

??? "EE_INLINE size_t ee_array_len(const Array* array)"

    **Description:**
    
    Returns the number of elements currently stored in the array.
    
    **Parameters:**
    
    | Name  | Type         | Description            |
    |--------|--------------|------------------------|
    | array  | const Array* | Pointer to the array.  |
    
    **Example:**
    
    ```c
    printf("Array length: %zu\n", ee_array_len(&arr));
    ```

??? "EE_INLINE void ee_array_clear(Array* array)"

    **Description:**
    
    Removes all elements from the array but keeps the allocated capacity.
    
    **Parameters:**
    
    | Name  | Type   | Description                  |
    |-------|--------|------------------------------|
    | array | Array* | Pointer to the array object. |
    
    **Example:**
    
    ```c
    ee_array_clear(&arr);
    ```

??? "EE_INLINE void\* ee_array_push(Array* array)"

    **Description:**
    
    Adds an uninitialized element to the end of the array and returns its pointer.  
    Automatically grows the array if necessary.
    
    **Parameters:**
    
    | Name  | Type   | Description                  |
    |--------|--------|------------------------------|
    | array  | Array* | Pointer to the array object. |

    **Example:**
    
    ```c
    int* value = ee_array_push(&arr);
    *value = 42;
    ```

??? "EE_INLINE void ee_array_pop(Array* array)"

    **Description:**
    
    Removes the last element from the array.  
    Does nothing if the array is empty.
    
    **Parameters:**
    
    | Name  | Type   | Description                  |
    |--------|--------|------------------------------|
    | array  | Array* | Pointer to the array object. |
    
    **Example:**
    
    ```c
    ee_array_pop(&arr);
    ```

??? "EE_INLINE void\* ee_array_at(const Array* array, size_t index)"

    **Description:**
    
    Returns a pointer to the element at the given index.
    
    **Parameters:**
    
    | Name   | Type         | Description                       |
    |---------|--------------|-----------------------------------|
    | array   | const Array* | Pointer to the array.             |
    | index   | size_t       | Index of the desired element.     |

    **Example:**
    
    ```c
    int* elem = ee_array_at(&arr, 5);
    printf("%d\n", *elem);
    ```

??? "EE_INLINE void ee_array_grow(Array* array)"

    **Description:**
    
    Expands the capacity of the array, typically by doubling it.
    
    **Parameters:**
    
    | Name  | Type   | Description                  |
    |-------|--------|------------------------------|
    | array | Array* | Pointer to the array object. |
    
    **Example:**
    
    ```c
    ee_array_grow(&arr);
    ```

??? "EE_INLINE void ee_array_resize(Array* array, size_t new_size)"

    **Description:**
    
    Changes the logical size of the array.  
    Expands memory if `new_size` exceeds current capacity.
    
    **Parameters:**
    
    | Name     | Type   | Description                  |
    |----------|--------|------------------------------|
    | array    | Array* | Pointer to the array object. |
    | new_size | size_t | New size of the array.       |
    
    **Example:**
    
    ```c
    ee_array_resize(&arr, 128);
    ```

??? "EE_INLINE void ee_array_swap(Array\* a, Array\* b)"

    **Description:**
    
    Swaps the contents of two arrays of the same element size.
    
    **Parameters:**
    
    | Name | Type   | Description                  |
    |------|--------|------------------------------|
    | a    | Array* | Pointer to the first array.  |
    | b    | Array* | Pointer to the second array. |

    **Example:**
    
    ```c
    ee_array_swap(&arr1, &arr2);
    ```

??? "EE_INLINE void\* ee_array_find(const Array\* array, const void\* value, CompareFn cmp)"

    **Description:**
    
    Finds the first element equal to `value` using a custom comparator.
    
    **Parameters:**
    
    | Name  | Type         | Description                               |
    |-------|--------------|-------------------------------------------|
    | array | const Array* | Pointer to the array.                     |
    | value | const void*  | Value to find.                            |
    | cmp   | CompareFn    | Comparator function (returns 0 if equal). |
    
    **Example:**
    
    ```c
    int key = 42;
    int* found = ee_array_find(&arr, &key, cmp_int);
    ```

??? "EE_INLINE void ee_array_erase(Array\* array, size_t index)"

    **Description:**
    
    Removes an element from the array at the specified index.
    
    **Parameters:**
    
    | Name  | Type   | Description                     |
    |-------|--------|---------------------------------|
    | array | Array* | Pointer to the array object.    |
    | index | size_t | Index of the element to remove. |
    
    **Example:**
    
    ```c
    ee_array_erase(&arr, 3);
    ```

??? "EE_INLINE void ee_array_insert(Array\* array, size_t index, const void\* value)"

    **Description:**
    
    Inserts a new element at the specified index, shifting existing elements.
    
    **Parameters:**
    
    | Name  | Type        | Description                     |
    |-------|-------------|---------------------------------|
    | array | Array*      | Pointer to the array object.    |
    | index | size_t      | Position to insert new element. |
    | value | const void* | Pointer to the value to insert. |
    
    **Example:**
    
    ```c
    int value = 7;
    ee_array_insert(&arr, 2, &value);
    ```

??? "EE_INLINE void\* ee_array_min_pred(const Array* array, CompareFn cmp)"

    **Description:**
    
    Finds the element with the minimal value according to a comparator.
    
    **Parameters:**
    
    | Name  | Type         | Description                            |
    |-------|--------------|----------------------------------------|
    | array | const Array* | Pointer to the array.                  |
    | cmp   | CompareFn    | Comparator function used for ordering. |
    
    **Example:**
    
    ```c
    int* min = ee_array_min_pred(&arr, cmp_int);
    ```

??? "EE_INLINE void\* ee_array_max_pred(const Array* array, CompareFn cmp)"

    **Description:**
    
    Finds the element with the maximal value according to a comparator.
    
    **Parameters:**
    
    | Name  | Type         | Description                            |
    |-------|--------------|----------------------------------------|
    | array | const Array* | Pointer to the array.                  |
    | cmp   | CompareFn    | Comparator function used for ordering. |
    
    **Example:**
    
    ```c
    int* max = ee_array_max_pred(&arr, cmp_int);
    ```