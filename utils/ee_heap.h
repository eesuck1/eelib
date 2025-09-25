#pragma once

#ifndef EE_HEAP_H
#define EE_HEAP_H

#include "ee_array.h"

#define EE_HEAP_LEFT(x)      (((x) << 1) + 1)
#define EE_HEAP_RIGHT(x)     (((x) << 1) + 2)
#define EE_HEAP_PARENT(x)    (((x) - 1) >> 1)
#define EE_HEAP_DT(x)        (EE_ARRAY_DT(x))

typedef struct Heap
{	
	Array items;
	BinCmp cmp;
} Heap;

EE_EXTERN_C_START

EE_INLINE Heap ee_heap_new(size_t size, size_t elem_size, BinCmp cmp, Allocator* allocator)
{
	EE_ASSERT(cmp != NULL, "Trying to set NULL comparator");

	Heap out = { 0 };

	out.items = ee_array_new(size, elem_size, allocator);
	out.cmp = cmp;

	return out;
}

EE_INLINE void ee_heap_free(Heap* heap)
{
	EE_ASSERT(heap != NULL, "Trying to free NULL heap");

	ee_array_free(&heap->items);
	memset(heap, 0, sizeof(Heap));
}

EE_INLINE void ee_heap_up(Heap* heap, s64 i)
{
	while (i > 0)
	{
		s64 parent = EE_HEAP_PARENT(i);

		if (heap->cmp(ee_array_at(&heap->items, i), ee_array_at(&heap->items, parent)) < 0)
		{
			ee_array_swap(&heap->items, i, parent);

			i = parent;
		}
		else
		{
			break;
		}
	}
}

EE_INLINE void ee_heap_down(Heap* heap, s64 i)
{
	s64 len = (s64)ee_array_len(&heap->items);
	s64 left = 0, right = 0, smallest = 0;

	while (left < len)
	{
		left = EE_HEAP_LEFT(i);
		right = EE_HEAP_RIGHT(i);
		smallest = i;

		if (left < len && heap->cmp(ee_array_at(&heap->items, left), ee_array_at(&heap->items, smallest)) < 0)
		{
			smallest = left;
		}

		if (right < len && heap->cmp(ee_array_at(&heap->items, right), ee_array_at(&heap->items, smallest)) < 0)
		{
			smallest = right;
		}

		if (smallest != i)
		{
			ee_array_swap(&heap->items, i, smallest);

			i = smallest;
		}
		else
		{
			break;
		}
	}
}

EE_INLINE void ee_heap_push(Heap* heap, u8* val)
{
	EE_ASSERT(heap != NULL, "Trying to push into NULL heap");

	ee_array_push(&heap->items, val);
	ee_heap_up(heap, ee_array_len(&heap->items) - 1);
}

EE_INLINE void ee_heap_pop(Heap* heap, u8* out_val)
{
	EE_ASSERT(heap != NULL, "Trying to push into NULL heap");

	Array* items = &heap->items;
	size_t len = ee_array_len(items);

	EE_ASSERT(len != 0, "Trying to pop from empty heap");

	if (out_val != NULL)
	{
		memcpy(out_val, ee_array_at(items, 0), items->elem_size);
	}

	ee_array_swap(items, 0, len - 1);
	ee_array_pop(items, NULL);
	ee_heap_down(heap, 0);
}

EE_INLINE int ee_heap_empty(Heap* heap)
{
	EE_ASSERT(heap != NULL, "Trying to check NULL heap");

	return ee_array_empty(&heap->items);
}

EE_INLINE size_t ee_heap_len(Heap* heap) 
{
	EE_ASSERT(heap != NULL, "Trying to get length of NULL heap");

	return ee_array_len(&heap->items);
}

EE_INLINE u8* ee_heap_top(Heap* heap)
{
	EE_ASSERT(heap != NULL, "Trying to get top element of NULL heap");

	return ee_array_at(&heap->items, 0);
}

EE_INLINE u8* ee_heap_at(Heap* heap, size_t i)
{
	EE_ASSERT(heap != NULL, "Trying to get top element of NULL heap");

	return ee_array_at(&heap->items, i);
}

EE_EXTERN_C_END

#endif // EE_Heap_H
