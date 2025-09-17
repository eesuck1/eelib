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

#if __STDC_VERSION__ >= 201112L
#include "stdalign.h"
#define EE_MAX_ALIGN    (alignof(max_align_t))
#else
#define EE_MAX_ALIGN    (16)
#endif

#define EE_ALIGN_MASK           (~(EE_MAX_ALIGN - 1))

#ifndef EE_ALLOCATOR
#define EE_ALLOCATOR

typedef struct Allocator
{
	void* (*alloc_fn)(struct Allocator* self, size_t size);
	void* (*realloc_fn)(struct Allocator* self, void* buffer, size_t old_size, size_t new_size);
	void  (*free_fn)(struct Allocator* self, void* buffer);
	void* context;
} Allocator;

EE_INLINE void* ee_default_alloc(Allocator* allocator, size_t size)
{
	(void)allocator;

	return malloc(size);
}

EE_INLINE void* ee_default_realloc(Allocator* allocator, void* buffer, size_t old_size, size_t new_size)
{
	(void)allocator;
	(void)old_size;

	return realloc(buffer, new_size);
}

EE_INLINE void ee_default_free(Allocator* allocator, void* buffer)
{
	(void)allocator;

	free(buffer);
}

#endif // EE_ALLOCATOR

typedef struct Arena
{
	size_t* marks;
	uint8_t* buffer;

	size_t size;
	size_t offset;
	size_t mark;
	size_t marks_depth;

	Allocator allocator;
} Arena;

EE_INLINE Arena ee_arena_new(size_t size, size_t rewind_depth, Allocator* allocator)
{
	Arena out = { 0 };

	if (allocator == NULL)
	{
		out.allocator.alloc_fn = ee_default_alloc;
		out.allocator.realloc_fn = ee_default_realloc;
		out.allocator.free_fn = ee_default_free;
		out.allocator.context = NULL;
	}
	else
	{
		memcpy(&out.allocator, allocator, sizeof(Allocator));
	}

	EE_ASSERT(out.allocator.alloc_fn != NULL, "Trying to set NULL alloc callback");
	EE_ASSERT(out.allocator.realloc_fn != NULL, "Trying to set NULL realloc callback");
	EE_ASSERT(out.allocator.free_fn != NULL, "Trying to set NULL free callback");

	size_t marks_size = rewind_depth * sizeof(size_t);
	size_t aligned_size = (size + EE_MAX_ALIGN - 1) & EE_ALIGN_MASK;
	size_t total_size = aligned_size + marks_size;

	uint8_t* buffer = (uint8_t*)out.allocator.alloc_fn(&out.allocator, total_size);

	out.buffer = buffer;
	out.marks = (size_t*)&out.buffer[aligned_size];

	out.size = aligned_size;
	out.offset = 0;
	out.mark = 0;
	out.marks_depth = rewind_depth;

	EE_ASSERT(buffer != NULL, "Unable to allocate (%zu) bytes for Arena.buffer", total_size);
	
	return out;
}

EE_INLINE void ee_arena_clear(Arena* arena)
{
	EE_ASSERT(arena != NULL, "Trying to clean NULL arena");

	memset(arena->buffer, 0, arena->size);
}

EE_INLINE void* ee_arena_alloc(Arena* arena, size_t size)
{
	const size_t total = size;
	const size_t offset = (arena->offset + EE_MAX_ALIGN - 1) & EE_ALIGN_MASK;

	if (offset + total > arena->size)
	{
		return NULL;
	}

	EE_ASSERT(offset + total <= arena->size,
		"Arena overflow. Asked (%zu) bytes but current capacity is (%zu)", total, arena->size - offset);

	void* out = (void*)(arena->buffer + offset);
	arena->offset = offset + total;

	return out;
}

EE_INLINE void ee_arena_mark(Arena* arena)
{
	EE_ASSERT(arena->mark < arena->marks_depth, "Arena marks stack overflow.");

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

	arena->allocator.free_fn(&arena->allocator, arena->buffer);
	memset(arena, 0, sizeof(Arena));
}


#endif // EE_ARENA_H
