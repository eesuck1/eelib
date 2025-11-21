#ifndef EE_ARENA_H
#define EE_ARENA_H

#include "ee_core.h"

#define EE_NO_REWIND     (0)

typedef struct Arena
{
    size_t* marks;
    u8* buffer;
    u8* base;

    size_t size;
    size_t offset;
    size_t mark;
    size_t marks_depth;

    Allocator allocator;
} Arena;

typedef struct Linked_Arena_Mark
{
    struct Linked_Arena* node;
    size_t offset;
} Linked_Arena_Mark;

typedef struct Linked_Arena
{
    u8* buffer;
    u8* base;

    size_t size;
    size_t offset;
    size_t mark;
    size_t marks_depth;

    Allocator allocator;

    Linked_Arena_Mark* marks;
    struct Linked_Arena* next;
    struct Linked_Arena* tail;
} Linked_Arena;

EE_EXTERN_C_START

EE_INLINE Arena ee_arena_new(size_t size, size_t rewind_depth, const Allocator* allocator)
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

    size_t aligned_payload = ee_round_up_pow2(size, EE_MAX_ALIGN);
    size_t marks_size = rewind_depth * sizeof(size_t);
    size_t marks_align = ee_round_up_pow2(aligned_payload, sizeof(size_t));
    size_t total_size = marks_align + marks_size;

    size_t alloc_size = total_size + (EE_MAX_ALIGN - 1);

    u8* raw = (u8*)out.allocator.alloc_fn(&out.allocator, alloc_size);

    EE_ASSERT(raw != NULL, "Unable to allocate (%zu) bytes for Arena base", alloc_size);

    uintptr_t base_addr = (uintptr_t)raw;
    uintptr_t aligned_addr = (base_addr + (EE_MAX_ALIGN - 1)) & (uintptr_t)EE_ALIGN_MASK;

    u8* aligned = (u8*)aligned_addr;

    out.base = raw;
    out.buffer = aligned;

    out.size = aligned_payload;
    out.offset = 0;
    out.mark = 0;
    out.marks_depth = rewind_depth;

    if (rewind_depth != EE_NO_REWIND)
    {
        out.marks = (size_t*)(aligned + marks_align);
    }
    else
    {
        out.marks = NULL;
    }

    return out;
}

EE_INLINE void ee_arena_clear(Arena* arena)
{
    EE_ASSERT(arena != NULL, "Trying to clean NULL arena");

    memset(arena->buffer, 0, arena->size);
}

EE_INLINE void* ee_arena_alloc(Arena* arena, size_t size)
{
    EE_ASSERT(arena != NULL, "Trying to alloc from NULL arena");

    size_t offset_aligned = ee_round_up_pow2(arena->offset, EE_MAX_ALIGN);

    if (offset_aligned + size > arena->size)
    {
        return NULL;
    }

    void* out = (void*)(arena->buffer + offset_aligned);
    arena->offset = offset_aligned + size;

    return out;
}

EE_INLINE void* ee_arena_alloc_al(Arena* arena, size_t size, size_t align)
{
    EE_ASSERT(arena != NULL, "Trying to alloc from NULL arena");

    size_t offset_aligned;

    if (align > 1)
        offset_aligned = ee_round_up_pow2(arena->offset, align);
    else
        offset_aligned = arena->offset;

    if (offset_aligned + size > arena->size)
    {
        return NULL;
    }

    void* out = (void*)(arena->buffer + offset_aligned);
    arena->offset = offset_aligned + size;

    return out;
}

EE_INLINE void ee_arena_mark(Arena* arena)
{
    EE_ASSERT(arena != NULL, "Trying to mark NULL arena");
    EE_ASSERT(arena->marks_depth != EE_NO_REWIND, "Arena has no marks buffer");
    EE_ASSERT(arena->mark < arena->marks_depth, "Arena marks stack overflow");

    arena->marks[arena->mark++] = arena->offset;
}

EE_INLINE void ee_arena_rewind(Arena* arena)
{
    EE_ASSERT(arena != NULL, "Trying to rewind NULL arena");
    EE_ASSERT(arena->marks_depth != EE_NO_REWIND, "Arena has no marks buffer");
    EE_ASSERT(arena->mark > 0, "Arena marks stack underflow");

    arena->offset = arena->marks[--arena->mark];
}

EE_INLINE void ee_arena_reset(Arena* arena)
{
    EE_ASSERT(arena != NULL, "Trying to reset NULL arena");

    arena->offset = 0;
    arena->mark = 0;
}

EE_INLINE void ee_arena_free(Arena* arena)
{
    EE_ASSERT(arena != NULL, "Trying to free NULL arena");
    EE_ASSERT(arena->base != NULL, "Invalid arena base value (NULL)");

    arena->allocator.free_fn(&arena->allocator, arena->base);
    memset(arena, 0, sizeof(Arena));
}

EE_INLINE void* eev_arena_alloc_fn(Allocator* allocator, size_t size)
{
    EE_ASSERT(allocator != NULL, "Trying to alloc with NULL allocator");
    EE_ASSERT(allocator->context != NULL, "Trying to alloc with NULL allocator context");

    Arena* arena = (Arena*)allocator->context;

    return ee_arena_alloc(arena, size);
}

EE_INLINE void* eev_arena_realloc_fn(Allocator* allocator, void* buffer, size_t old_size, size_t new_size)
{
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

EE_INLINE Linked_Arena ee_linked_arena_new(size_t size, size_t rewind_depth, const Allocator* allocator)
{
    Linked_Arena out = { 0 };

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

    size_t aligned_payload = ee_round_up_pow2(size, EE_MAX_ALIGN);
    size_t marks_size = rewind_depth * sizeof(Linked_Arena_Mark);
    size_t marks_align = ee_round_up_pow2(aligned_payload, sizeof(Linked_Arena_Mark));
    size_t total_size = marks_align + marks_size;

    size_t alloc_size = total_size + (EE_MAX_ALIGN - 1);

    u8* raw = (u8*)out.allocator.alloc_fn(&out.allocator, alloc_size);

    EE_ASSERT(raw != NULL, "Unable to allocate (%zu) bytes for Arena base", alloc_size);

    uintptr_t base_addr = (uintptr_t)raw;
    uintptr_t aligned_addr = (base_addr + (EE_MAX_ALIGN - 1)) & (uintptr_t)EE_ALIGN_MASK;

    u8* aligned = (u8*)aligned_addr;

    out.base = raw;
    out.buffer = aligned;

    out.size = aligned_payload;
    out.offset = 0;
    out.mark = 0;
    out.marks_depth = rewind_depth;
    
    out.next = NULL;
    out.tail = NULL;

    if (rewind_depth != EE_NO_REWIND)
    {
        out.marks = (Linked_Arena_Mark*)(aligned + marks_align);
    }
    else
    {
        out.marks = NULL;
    }

    return out;
}

EE_INLINE Linked_Arena* ee_linked_arena_alloc_new(size_t size, size_t rewind_depth, const Allocator* allocator)
{
    Linked_Arena out = { 0 };

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

    size_t aligned_payload = ee_round_up_pow2(size, EE_MAX_ALIGN);
    size_t marks_size = rewind_depth * sizeof(Linked_Arena_Mark);
    size_t marks_align = ee_round_up_pow2(aligned_payload, sizeof(Linked_Arena_Mark));
    size_t total_size = marks_align + marks_size;

    size_t alloc_size = total_size + (EE_MAX_ALIGN - 1);

    u8* raw = (u8*)out.allocator.alloc_fn(&out.allocator, alloc_size);

    EE_ASSERT(raw != NULL, "Unable to allocate (%zu) bytes for Arena base", alloc_size);

    uintptr_t base_addr = (uintptr_t)raw;
    uintptr_t aligned_addr = (base_addr + (EE_MAX_ALIGN - 1)) & (uintptr_t)EE_ALIGN_MASK;

    u8* aligned = (u8*)aligned_addr;

    out.base = raw;
    out.buffer = aligned;

    out.size = aligned_payload;
    out.offset = 0;
    out.mark = 0;
    out.marks_depth = rewind_depth;
    
    out.next = NULL;
    out.tail = NULL;

    if (rewind_depth != EE_NO_REWIND)
    {
        out.marks = (Linked_Arena_Mark*)(aligned + marks_align);
    }
    else
    {
        out.marks = NULL;
    }

    Linked_Arena* out_ptr = (Linked_Arena*)out.allocator.alloc_fn(&out.allocator, sizeof(*out_ptr));

    EE_ASSERT(out_ptr != NULL, "Unable to allocate (%zu) bytes", sizeof(*out_ptr));
    memcpy(out_ptr, &out, sizeof(*out_ptr));

    return out_ptr;
}


EE_INLINE void ee_linked_arena_clear(Linked_Arena* arena)
{
    EE_ASSERT(arena != NULL, "Trying to clean NULL arena");

    do
    {
        memset(arena->buffer, 0, arena->size);
        arena = arena->next;
    } while (arena != NULL);
}

EE_INLINE void* ee_linked_arena_alloc(Linked_Arena* arena, size_t size)
{
    EE_ASSERT(arena != NULL, "Trying to alloc from NULL arena");

    Linked_Arena* current = arena->tail == NULL ? arena : arena->tail;
    size_t offset_aligned = ee_round_up_pow2(current->offset, EE_MAX_ALIGN);

    while (offset_aligned + size > current->size)
    {
        if (current->next == NULL)
        {
            size_t alloc_size = ee_max_u64(size, arena->size);
            current->next = ee_linked_arena_alloc_new(alloc_size, EE_NO_REWIND, &arena->allocator);
        }

        current = current->next;
        arena->tail = current;
        offset_aligned = ee_round_up_pow2(current->offset, EE_MAX_ALIGN);
    }

    void* out = (void*)(current->buffer + offset_aligned);
    current->offset = offset_aligned + size;

    return out;
}

EE_INLINE void* ee_linked_arena_alloc_al(Linked_Arena* arena, size_t size, size_t align)
{
    EE_ASSERT(arena != NULL, "Trying to alloc from NULL arena");

    Linked_Arena* current = arena->tail == NULL ? arena : arena->tail;
    size_t offset_aligned;

    if (align > 1)
        offset_aligned = ee_round_up_pow2(current->offset, align);
    else
        offset_aligned = current->offset;

    while (offset_aligned + size > current->size)
    {
        if (current->next == NULL)
        {
            size_t alloc_size = ee_max_u64(size, arena->size);
            current->next = ee_linked_arena_alloc_new(alloc_size, EE_NO_REWIND, &arena->allocator);
        }

        current = current->next;
        arena->tail = current;

        if (align > 1)
            offset_aligned = ee_round_up_pow2(current->offset, align);
        else
            offset_aligned = current->offset;
    }

    void* out = (void*)(current->buffer + offset_aligned);
    current->offset = offset_aligned + size;

    return out;
}

EE_INLINE void ee_linked_arena_mark(Linked_Arena* arena)
{
    EE_ASSERT(arena != NULL, "Trying to mark NULL arena");
    EE_ASSERT(arena->marks_depth != EE_NO_REWIND, "Arena has no marks buffer");
    EE_ASSERT(arena->mark < arena->marks_depth, "Arena marks stack overflow");

    Linked_Arena* current = arena->tail == NULL ? arena : arena->tail;
    Linked_Arena_Mark mark = { 0 };

    mark.node = current;
    mark.offset = current->offset;

    arena->marks[arena->mark++] = mark;
}

EE_INLINE void ee_linked_arena_rewind(Linked_Arena* arena)
{
    EE_ASSERT(arena != NULL, "Trying to rewind NULL arena");
    EE_ASSERT(arena->marks_depth != EE_NO_REWIND, "Arena has no marks buffer");
    EE_ASSERT(arena->mark > 0, "Arena marks stack underflow");

    Linked_Arena_Mark mark = arena->marks[--arena->mark];

    arena->tail = mark.node;
    arena->tail->offset = mark.offset;

    Linked_Arena* tail = arena->tail->next;

    while (tail != NULL)
    {
        tail->offset = 0;
        tail = tail->next;
    }
}

EE_INLINE void ee_linked_arena_reset(Linked_Arena* arena)
{
    EE_ASSERT(arena != NULL, "Trying to reset NULL arena");

    arena->mark = 0;
    arena->tail = arena;

    do
    {
        arena->offset = 0;
        arena = arena->next;
    } while (arena != NULL);
}

EE_INLINE void ee_linked_arena_free(Linked_Arena* head)
{
    EE_ASSERT(head != NULL, "Trying to free NULL arena");
    EE_ASSERT(head->base != NULL, "Invalid arena base value (NULL)");

    Linked_Arena* node = head->next;

    while (node) 
    {
        Linked_Arena* next = node->next;
    
        head->allocator.free_fn(&head->allocator, node->base);
        head->allocator.free_fn(&head->allocator, node);
        
        node = next;
    }

    head->allocator.free_fn(&head->allocator, head->base);
    memset(head, 0, sizeof(*head));
}

EE_INLINE void* ee_linked_arena_alloc_fn(Allocator* allocator, size_t size)
{
    EE_ASSERT(allocator != NULL, "Trying to alloc with NULL allocator");
    EE_ASSERT(allocator->context != NULL, "Trying to alloc with NULL allocator context");

    Linked_Arena* arena = (Linked_Arena*)allocator->context;

    return ee_linked_arena_alloc(arena, size);
}

EE_INLINE void* ee_linked_arena_realloc_fn(Allocator* allocator, void* buffer, size_t old_size, size_t new_size)
{
    EE_ASSERT(allocator != NULL, "Trying to alloc with NULL allocator");
    EE_ASSERT(allocator->context != NULL, "Trying to alloc with NULL allocator context");

    Linked_Arena* head = (Linked_Arena*)allocator->context;

    if (buffer == NULL)
    {
        return ee_linked_arena_alloc(head, new_size);
    }

    
    if (new_size <= old_size)
    {
        return buffer;
    }

    Linked_Arena* current = head->tail == NULL ? head : head->tail;

    u8* buffer_end = (u8*)buffer + old_size;
    u8* arena_top = current->buffer + current->offset;

    if (buffer_end == arena_top)
    {
        size_t diff = new_size - old_size;

        if (current->offset + diff <= current->size)
        {
            current->offset += diff;
            return buffer;
        }
    }

    void* new_buffer = ee_linked_arena_alloc(head, new_size);

    if (new_buffer != NULL)
    {
        memcpy(new_buffer, buffer, old_size);
    }

    return new_buffer;
}

EE_INLINE void  ee_linked_arena_free_fn(Allocator* allocator, void* buffer)
{
    (void)allocator;
    (void)buffer;
}

EE_INLINE Allocator ee_linked_arena_allocator(Linked_Arena* arena)
{
    Allocator out = { 0 };

    out.alloc_fn = ee_linked_arena_alloc_fn;
    out.realloc_fn = ee_linked_arena_realloc_fn;
    out.free_fn = ee_linked_arena_free_fn;
    out.context = arena;

    return out;
}

EE_EXTERN_C_END

#endif // EE_ARENA_H
