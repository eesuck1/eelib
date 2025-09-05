#pragma once

#ifndef EE_SET_H
#define EE_SET_H

#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "immintrin.h"

#include "ee_vec.h"

#define EE_NODE_PL_SIZE    (8)
#define EE_NODE_NULL       (-1)

static const uint8_t EE_RED   = 0x00;
static const uint8_t EE_BLACK = 0xFF;

typedef struct Node
{
	int64_t left;
	int64_t right;
	int64_t prev;
	uint8_t data[EE_NODE_PL_SIZE];
} Node;

typedef struct Set
{
	int64_t root;
	int64_t min;
	int64_t max;

	BinCmp cmp;

	Vec nodes;
	Vec free;
	Vec colors;
};

EE_INLINE Node ee_node_new(int64_t prev, uint8_t data[EE_NODE_PL_SIZE])
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

	out.nodes  = ee_vec_new(size, sizeof(Node));
	out.free   = ee_vec_new(size, sizeof(int64_t));
	out.colors = ee_vec_new(size, sizeof(uint8_t));

	ee_vec_fill(&out.colors, EE_VEC_DT(EE_RED), 0, size);

	return out;
}

EE_INLINE void ee_set_insert(Set* set, uint8_t data[EE_NODE_PL_SIZE])
{
	if (set->root == EE_NODE_NULL)
	{
		Node root = ee_node_new(EE_NODE_NULL, data);
		
		set->root = ee_vec_len(&set->nodes);

		ee_vec_push(&set->nodes, EE_VEC_DT(root));
		ee_vec_push(&set->colors, EE_VEC_DT(EE_BLACK));
	}
}

#endif // EE_SET_H
