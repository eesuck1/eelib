#pragma once

#ifndef EE_SET_H
#define EE_SET_H

#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "immintrin.h"

#ifndef EE_NO_ASSERT
#ifndef EE_ASSERT
#include "stdio.h"

#define EE_ASSERT(cond, fmt, ...) do {                                    \
		if (!(cond)) {                                                        \
			fprintf(stderr, "[%s][%d][%s] ", __FILE__, __LINE__, __func__);   \
			fprintf(stderr, fmt "\n", ##__VA_ARGS__);                         \
			exit(1);                                                          \
		}                                                                     \
	} while (0)
#endif // EE_ASSERT
#else
#define EE_ASSERT(cond, fmt, ...)    ((void)0)
#endif // EE_NO_ASSERT

#ifndef EE_INLINE
#define EE_INLINE    static inline
#endif // EE_INLINE

#ifndef EE_TRUE
#define EE_TRUE     (1)
#endif // EE_TRUE

#ifndef EE_FALSE
#define EE_FALSE    (0)
#endif // EE_FALSE

#define EE_NODE_PL_SIZE    (8)

typedef enum NodeColor
{
	EE_RED   = 1,
	EE_BLACK = 2,
} NodeColor;

typedef struct Node
{
	size_t left;
	size_t right;
	size_t prev;

	NodeColor color;
	uint8_t data[EE_NODE_PL_SIZE];
} Node;

typedef struct Set
{
	Node* root;
	Node* nodes;
};

#endif // EE_SET_H
