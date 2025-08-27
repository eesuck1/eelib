#pragma once

#ifndef EE_ISTACK_H
#define EE_ISTACK_H

#include "stdlib.h"
#include "string.h"
#include "stdint.h"

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
#else  // EE_NO_ASSERT

#define EE_ASSERT(cond, fmt, ...)    ((void)0)

#endif // EE_NO_ASSERT

#ifndef EE_INLINE
#define EE_INLINE    static inline
#endif // EE_INLINE

#define EE_KEY_SIZE        (16)
#define EE_VALUE_SIZE      (24)
#define EE_DEFAULT_SEED    (263611987)

#ifndef EE_TRUE
#define EE_TRUE            (1)
#endif // EE_TRUE

#ifndef EE_FALSE
#define EE_FALSE           (0)
#endif // EE_FALSE

typedef struct IStack
{
	int32_t* buffer;
	size_t size;
	size_t top;
} IStack;

EE_INLINE IStack ee_istack_new(size_t size)
{
	IStack out = { 0 };

	out.buffer = (int32_t*)malloc(sizeof(int32_t) * size);
	out.size = size;
	out.top = 0;

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for IStack.buffer", sizeof(int32_t) * size);

	return out;
}

EE_INLINE void ee_istack_push(IStack* stack, int32_t value)
{
	EE_ASSERT(stack->top < stack->size, "Can't push to full stack");

	if (stack->top > stack->size)
	{
		return;
	}

	stack->buffer[stack->top++] = value;
}

EE_INLINE int32_t ee_istack_pop(IStack* stack)
{
	EE_ASSERT(stack->top > 0, "Can't pop from empty stack");

	if (stack->top <= 0)
	{
		return 0;
	}

	return stack->buffer[--stack->top];
}

EE_INLINE int32_t* ee_istack_top(IStack* stack)
{
	return &stack->buffer[stack->top - 1];
}

EE_INLINE int32_t ee_istack_peek(IStack* stack)
{
	return stack->buffer[stack->top - 1];
}

EE_INLINE int32_t ee_istack_is_full(IStack* stack)
{
	return stack->top >= stack->size;
}

EE_INLINE int32_t ee_istack_is_empty(IStack* stack)
{
	return stack->top == 0;
}

#endif // EE_ISTACK_H
