#pragma once

#ifndef EE_ARENA_H
#define EE_ARENA_H

#include "ee_core.h"

// TODO(eesuck): better max_align
#define EE_MAX_ALIGN     (16)
#define EE_ALIGN_MASK    (~(EE_MAX_ALIGN - 1))
#define EE_NO_REWIND     (0)

typedef struct Arena
{
	size_t* marks;
	u8* buffer;

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

	u8* buffer = (u8*)out.allocator.alloc_fn(&out.allocator, total_size);

	out.buffer = buffer;
	
	if (rewind_depth != EE_NO_REWIND)
	{
		out.marks = (size_t*)&out.buffer[aligned_size];
	}

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

EE_INLINE void* eev_arena_alloc_fn(Allocator* allocator, size_t size)
{
	EE_ASSERT(allocator != NULL, "Trying to alloc with NULL allocator");
	EE_ASSERT(allocator->context != NULL, "Trying to alloc with NULL allocator context");

	Arena* arena = (Arena*)allocator->context;

	return  ee_arena_alloc(arena, size);
}

EE_INLINE void* eev_arena_realloc_fn(Allocator* allocator, void* buffer, size_t old_size, size_t new_size)
{
	// TODO(eesuck): possibly could be done if block is on top of the arena

	(void)allocator;
	(void)buffer;
	(void)old_size;
	(void)new_size;

	return NULL;
}

EE_INLINE void eev_arena_free_fn(Allocator* allocator, void* buffer)
{
	(void)allocator;
	(void)buffer;

	return;
}

EE_INLINE Allocator ee_arena_allocator(Arena* arena)
{
	Allocator out = { 0 };

	out.alloc_fn = eev_arena_alloc_fn;
	out.realloc_fn = eev_arena_realloc_fn;
	out.free_fn = eev_arena_free_fn;
	out.context = arena;

	return out;
}

#endif // EE_ARENA_H
