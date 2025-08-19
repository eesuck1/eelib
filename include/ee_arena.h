#pragma once

#ifndef EE_ARENA_H
#define EE_ARENA_H

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


#define EE_ARENA_MARKS_DEPTH    (64)

typedef struct Arena
{
	uint8_t* buffer;

	size_t size;
	size_t offset;
	size_t marks[EE_ARENA_MARKS_DEPTH];
	size_t mark;
} Arena;

EE_INLINE Arena ee_arena_new(size_t size)
{
	Arena out = { 0 };

	out.buffer = (uint8_t*)malloc(size);
	out.size = size;
	out.offset = 0;
	out.mark = 0;

	memset(out.marks, 0, sizeof(out.marks));

	return out;
}

EE_INLINE void* ee_arena_alloc(Arena* arena, size_t count, size_t size, size_t align)
{
	const size_t total = size * count;
	const size_t offset = (arena->offset + align - 1) & ~(align - 1);

	EE_ASSERT(offset + total < arena->size,
		"Arena overflow. Asked (%zu) bytes but current capacity is (%zu)", total, arena->size - offset);

	void* out = (void*)(arena->buffer + offset);
	arena->offset = offset + total;

	return out;
}

EE_INLINE void ee_arena_mark(Arena* arena)
{
	EE_ASSERT(arena->mark < EE_ARENA_MARKS_DEPTH, "Arena marks stack overflow.");

	arena->marks[arena->mark++] = arena->offset;
}

EE_INLINE void ee_arena_rewind(Arena* arena)
{
	EE_ASSERT(arena->mark > 0, "Arena marks stack underflow.");

	arena->offset = arena->marks[--arena->mark];
}

EE_INLINE void ee_arena_reset(Arena* arena)
{
	arena->offset = 0;
	arena->mark = 0;
}

EE_INLINE void ee_arena_free(Arena* arena)
{
	EE_ASSERT(arena->buffer != NULL, "Invalid arena buffer value (NULL).");

	free(arena->buffer);
}


#endif // EE_ARENA_H
