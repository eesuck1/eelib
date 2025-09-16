#pragma once

#ifndef EE_HEAP_H
#define EE_HEAP_H

#include "ee_vec.h"

#define EE_HEAP_LEFT(x)      (((x) << 1) + 1)
#define EE_HEAP_RIGHT(x)     (((x) << 1) + 2)
#define EE_HEAP_PARENT(x)    (((x) - 1) >> 1)
#define EE_HEAP_DT(x)        (EE_VEC_DT(x))

typedef struct Heap
{	
	Vec items;
	BinCmp cmp;
} Heap;

EE_INLINE Heap ee_heap_new(size_t size, size_t elem_size, BinCmp cmp, Allocator* allocator)
{
	EE_ASSERT(cmp != NULL, "Trying to set NULL comparator");

	Heap out = { 0 };

	out.items = ee_vec_new(size, elem_size, allocator);
	out.cmp = cmp;

	return out;
}

EE_INLINE void ee_heap_free(Heap* heap)
{
	EE_ASSERT(heap != NULL, "Trying to free NULL heap");

	ee_vec_free(&heap->items);
	memset(heap, 0, sizeof(Heap));
}

EE_INLINE void ee_heap_up(Heap* heap, int64_t i)
{
	while (i > 0)
	{
		int64_t parent = EE_HEAP_PARENT(i);

		if (heap->cmp(ee_vec_at(&heap->items, i), ee_vec_at(&heap->items, parent)) < 0)
		{
			ee_vec_swap(&heap->items, i, parent);

			i = parent;
		}
		else
		{
			break;
		}
	}
}

EE_INLINE void ee_heap_down(Heap* heap, int64_t i)
{
	int64_t len = (int64_t)ee_vec_len(&heap->items);
	int64_t left = 0, right = 0, smallest = 0;

	while (left < len)
	{
		left = EE_HEAP_LEFT(i);
		right = EE_HEAP_RIGHT(i);
		smallest = i;

		if (left < len && heap->cmp(ee_vec_at(&heap->items, left), ee_vec_at(&heap->items, smallest)) < 0)
		{
			smallest = left;
		}

		if (right < len && heap->cmp(ee_vec_at(&heap->items, right), ee_vec_at(&heap->items, smallest)) < 0)
		{
			smallest = right;
		}

		if (smallest != i)
		{
			ee_vec_swap(&heap->items, i, smallest);

			i = smallest;
		}
		else
		{
			break;
		}
	}
}

EE_INLINE void ee_heap_push(Heap* heap, uint8_t* val)
{
	EE_ASSERT(heap != NULL, "Trying to push into NULL heap");

	ee_vec_push(&heap->items, val);
	ee_heap_up(heap, ee_vec_len(&heap->items) - 1);
}

EE_INLINE void ee_heap_pop(Heap* heap, uint8_t* out_val)
{
	EE_ASSERT(heap != NULL, "Trying to push into NULL heap");

	Vec* items = &heap->items;
	size_t len = ee_vec_len(items);

	EE_ASSERT(len != 0, "Trying to pop from empty heap");

	if (out_val != NULL)
	{
		memcpy(out_val, ee_vec_at(items, 0), items->elem_size);
	}

	ee_vec_swap(items, 0, len - 1);
	ee_vec_pop(items, NULL);
	ee_heap_down(heap, 0);
}

EE_INLINE int ee_heap_empty(Heap* heap)
{
	EE_ASSERT(heap != NULL, "Trying to check NULL heap");

	return ee_vec_empty(&heap->items);
}

EE_INLINE size_t ee_heap_len(Heap* heap) 
{
	EE_ASSERT(heap != NULL, "Trying to get length of NULL heap");

	return ee_vec_len(&heap->items);
}

EE_INLINE uint8_t* ee_heap_top(Heap* heap)
{
	EE_ASSERT(heap != NULL, "Trying to get top element of NULL heap");

	return ee_vec_at(&heap->items, 0);
}

EE_INLINE uint8_t* ee_heap_at(Heap* heap, size_t i)
{
	EE_ASSERT(heap != NULL, "Trying to get top element of NULL heap");

	return ee_vec_at(&heap->items, i);
}

#endif // EE_Heap_H
