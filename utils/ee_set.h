#pragma once

#ifndef EE_SET_H
#define EE_SET_H

#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "immintrin.h"

#include "ee_array.h"

#define EE_NODE_PL_SIZE    (8)
#define EE_NODE_NULL       (-1)

static const u8 EE_RED   = 0x00;
static const u8 EE_BLACK = 0xFF;

typedef struct Node
{
	s64 left;
	s64 right;
	s64 prev;
	u8 data[EE_NODE_PL_SIZE];
} Node;

typedef struct Set
{
	s64 root;
	s64 min;
	s64 max;

	BinCmp cmp;

	Array nodes;
	Array free;
	Array colors;
};

EE_EXTERN_C_START

EE_INLINE Node ee_node_new(s64 prev, u8 data[EE_NODE_PL_SIZE])
{
	Node out = { 0 };

	out.left  = EE_NODE_NULL;
	out.right = EE_NODE_NULL;
	out.prev  = prev;

	if (data == NULL)
		memset(out.data, 0, EE_NODE_PL_SIZE);
	else
		memcpy(out.data, data, EE_NODE_PL_SIZE);

	return out;
}

EE_INLINE Set ee_set_new(size_t size, BinCmp cmp)
{
	EE_ASSERT(cmp != NULL, "Passed NULL comparator");

	Set out = { 0 };

	out.root   = EE_NODE_NULL;
	out.max    = EE_NODE_NULL;
	out.min    = EE_NODE_NULL;
	
	out.cmp    = cmp;

	out.nodes  = ee_array_new(size, sizeof(Node));
	out.free   = ee_array_new(size, sizeof(s64));
	out.colors = ee_array_new(size, sizeof(u8));

	ee_array_fill(&out.colors, EE_ARRAY_DT(EE_RED), 0, size);

	return out;
}

EE_INLINE void ee_set_insert(Set* set, u8 data[EE_NODE_PL_SIZE])
{
	if (set->root == EE_NODE_NULL)
	{
		Node root = ee_node_new(EE_NODE_NULL, data);
		
		set->root = ee_array_len(&set->nodes);

		ee_array_push(&set->nodes, EE_ARRAY_DT(root));
		ee_array_push(&set->colors, EE_ARRAY_DT(EE_BLACK));
	}
}

EE_EXTERN_C_END

#endif // EE_SET_H
