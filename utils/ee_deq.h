#ifndef EE_DEQ_H
#define EE_DEQ_H

#include "ee_core.h"

typedef struct Deq
{
	size_t head;
	size_t tail;
	size_t cap;
	size_t mask;
	size_t elem_size;
	u8* buffer;
	Allocator allocator;
} Deq;

EE_INLINE Deq ee_deq_new(size_t size, size_t elem_size, Allocator* allocator)
{
	EE_ASSERT(size > 0, "Invalid deqtor size (%zu)", size);
	EE_ASSERT(elem_size > 0, "Invalid deqtor elem_size (%zu)", elem_size);

	Deq out = { 0 };

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

	out.head = 0;
	out.tail = 0;
	out.cap = ee_next_pow_2(elem_size * size);
	out.mask = out.cap - 1;
	out.elem_size = elem_size;
	out.buffer = (u8*)out.allocator.alloc_fn(&out.allocator, out.cap);

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Deq.buffer", out.cap);

	return out;
}

EE_INLINE void ee_deq_free(Deq* deq)
{
	EE_ASSERT(deq != NULL, "Trying to free NULL Array");
	EE_ASSERT(deq->buffer != NULL, "Trying to free NULL Array.buffer");

	deq->allocator.free_fn(&deq->allocator, deq->buffer);

	memset(deq, 0, sizeof(Deq));
}

EE_INLINE s32 ee_deq_size(const Deq* deq)
{
	EE_ASSERT(deq != NULL, "Trying to dereference NULL deq");

	return (deq->head - deq->tail) & deq->mask;
}

EE_INLINE s32 ee_deq_full(const Deq* deq)
{
	EE_ASSERT(deq != NULL, "Trying to dereference NULL deq");
	
	return ee_deq_size(deq) >= (deq->cap - deq->elem_size);
}

EE_INLINE void ee_deq_grow(Deq* deq)
{
	EE_ASSERT(deq != NULL, "Trying to dereference NULL deq");

	size_t new_cap = deq->cap << 1;
	u8* new_buffer = (u8*)deq->allocator.alloc_fn(&deq->allocator, new_cap);

	EE_ASSERT(new_buffer != NULL, "Unable to reallocate (%zu) bytes for Array.buffer", new_cap);

	if (deq->head > deq->tail)
	{
		memcpy(new_buffer, &deq->buffer[deq->tail], deq->head - deq->tail);

		deq->head -= deq->tail;
		deq->tail = 0;
	}
	else
	{
		size_t tail_chunk = deq->cap - deq->tail;

		memcpy(new_buffer, &deq->buffer[deq->tail], tail_chunk);
		memcpy(&new_buffer[tail_chunk], deq->buffer, deq->head);

		deq->head = tail_chunk + deq->head;
		deq->tail = 0;
	}

	deq->cap = new_cap;
	deq->mask = deq->cap - 1;
	deq->allocator.free_fn(&deq->allocator, deq->buffer);
	deq->buffer = new_buffer;
}

EE_INLINE void ee_deq_push_head(Deq* deq, const u8* val)
{
	EE_ASSERT(deq != NULL, "Trying to dereference NULL deq");
	EE_ASSERT(val != NULL, "Trying to dereference NULL value");

	if (ee_deq_full(deq))
	{
		ee_deq_grow(deq);
	}
	
	memcpy(&deq->buffer[deq->head], val, deq->elem_size);
	deq->head = (deq->head + deq->elem_size) & deq->mask;
}

EE_INLINE void ee_deq_pop_head(Deq* deq, u8* out_val)
{
	EE_ASSERT(deq != NULL, "Trying to dereference NULL deq");
	EE_ASSERT(out_val != NULL, "Trying to dereference NULL output value");
	EE_ASSERT(deq->head > deq->tail, "Trying to pop empty deq");

	deq->head = (deq->head - deq->elem_size) & deq->mask;

	if (out_val != NULL)
	{
		memcpy(out_val, &deq->buffer[deq->head], deq->elem_size);
	}
}

EE_INLINE void ee_deq_push_tail(Deq* deq, const u8* val)
{
	EE_ASSERT(deq != NULL, "Trying to dereference NULL deq");
	EE_ASSERT(val != NULL, "Trying to dereference NULL value");

	if (ee_deq_full(deq))
	{
		ee_deq_grow(deq);
	}
	
	deq->tail = (deq->tail + deq->cap - deq->elem_size) & deq->mask;
	memcpy(&deq->buffer[deq->tail], val, deq->elem_size);
}

EE_INLINE void ee_deq_pop_tail(Deq* deq, u8* out_val)
{
	EE_ASSERT(deq != NULL, "Trying to dereference NULL deq");
	EE_ASSERT(out_val != NULL, "Trying to dereference NULL output value");

	size_t size = ee_deq_size(deq);
	
	EE_ASSERT(size > 0, "Trying to pop empty deq");

	if (out_val != NULL)
	{
		memcpy(out_val, &deq->buffer[deq->tail], deq->elem_size);
	}

	deq->tail = (deq->tail + deq->elem_size) & deq->mask;
}

EE_INLINE u8* ee_deq_at_head(Deq* deq)
{
	EE_ASSERT(deq != NULL, "Trying to dereference NULL deq");

	return &deq->buffer[deq->head];
}

EE_INLINE u8* ee_deq_at_tail(Deq* deq)
{
	EE_ASSERT(deq != NULL, "Trying to dereference NULL deq");

	return &deq->buffer[deq->tail];
}

EE_INLINE u8* ee_deq_at(Deq* deq, size_t i)
{
	EE_ASSERT(deq != NULL, "Trying to dereference NULL deq");
	
	size_t i_b = i * deq->elem_size;
	EE_ASSERT((deq->head - deq->tail) & deq->mask > i_b, "Invalid index (%zu) for deq with size (%zu)", i, ((deq->head - deq->tail) & deq->mask) / deq->elem_size);

	return &deq->buffer[(deq->tail + i_b) & deq->mask];
}

#endif // EE_DEQ_H
