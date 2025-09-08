#pragma once

#ifndef EE_GRID_H
#define EE_GRID_H

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
#define EE_TRUE      (1)
#endif // EE_TRUE

#ifndef EE_FALSE
#define EE_FALSE     (0)
#endif // EE_FALSE

#define EE_GRID_DT(x)    ((uint8_t*)(&(x)))

typedef struct Grid
{
	uint8_t* buffer;

	size_t w;
	size_t h;
	size_t elem_size;
} Grid;

typedef struct Frame
{
	Grid* src;

	uint32_t x;
	uint32_t y;
	uint32_t w;
	uint32_t h;
} Frame;

EE_INLINE int64_t ee_clip_s64(int64_t x, int64_t a, int64_t b)
{
	EE_ASSERT(a < b, "Invalid bounds (%lld, %lld)", a, b);

	if (x < a)
		x = a;
	else if (x > b)
		x = b;

	return x;
}

EE_INLINE Grid ee_grid_new(size_t width, size_t height, size_t elem_size)
{
	Grid out = { 0 };

	out.buffer = (uint8_t*)calloc(width * height, elem_size);
	out.w = width;
	out.h = height;
	out.elem_size = elem_size;

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Grid.buffer", elem_size * width * height);

	return out;
}

EE_INLINE void ee_grid_free(Grid* grid)
{
	EE_ASSERT(grid != NULL, "Trying to free NULL grid");

	free(grid->buffer);
	memset(grid, 0, sizeof(Grid));
}

EE_INLINE void ee_grid_set(Grid* grid, size_t x, size_t y, uint8_t* val)
{
	EE_ASSERT(grid != NULL, "Trying to set into NULL grid");
	EE_ASSERT(val != NULL, "Trying to set NULL value");
	EE_ASSERT(x < grid->w && y < grid->h, "Invalid x, y value (%zu, %zu) for grid with size (%zu, %zu)", x, y, grid->w, grid->h);

	memcpy(&grid->buffer[(y * grid->w + x) * grid->elem_size], val, grid->elem_size);
}

EE_INLINE uint8_t* ee_grid_at(Grid* grid, size_t x, size_t y)
{
	EE_ASSERT(grid != NULL, "Trying to get from NULL grid");
	EE_ASSERT(x < grid->w && y < grid->h, "Invalid x, y value (%zu, %zu) for grid with size (%zu, %zu)", x, y, grid->w, grid->h);

	return &grid->buffer[(y * grid->w + x) * grid->elem_size];
}

EE_INLINE Frame ee_grid_frame(Grid* grid, int64_t left_x, int64_t top_y, int64_t width, int64_t height)
{
	Frame out = { 0 };
	
	int64_t min_x = ee_clip_s64(left_x, 0, grid->w);
	int64_t min_y = ee_clip_s64(top_y, 0, grid->h);
	int64_t max_x = ee_clip_s64(left_x + width, 0, grid->w);
	int64_t max_y = ee_clip_s64(top_y + height, 0, grid->h);

	EE_ASSERT(min_x != max_x || min_y != max_y, "Trying to create an empty frame (%lld, %lld, %lld, %lld)", left_x, top_y, width, height);

	out.src = grid;
	out.x = min_x;
	out.y = min_y;
	out.w = max_x - min_x;
	out.h = max_y - min_y;

	return out;
}

EE_INLINE void ee_frame_set(Frame frame, size_t x, size_t y, uint8_t* val)
{
	EE_ASSERT(frame.w != 0 && frame.h != 0, "Trying to set into empty frame (%ud, %ud, %ud, %ud)", frame.x, frame.y, frame.w, frame.h);
	EE_ASSERT(x < frame.w && y < frame.h, "Invalid frame coordinates (%zu, %zu) for frame size (%u, %u)", x, y, frame.w, frame.h);

	uint8_t* dest = &frame.src->buffer[((y + frame.y) * frame.src->w + x + frame.x) * frame.src->elem_size];
	memcpy(dest, val, frame.src->elem_size);
}

EE_INLINE uint8_t* ee_frame_at(Frame frame, size_t x, size_t y)
{
	EE_ASSERT(frame.w != 0 && frame.h != 0, "Trying to get from empty frame (%ud, %ud, %ud, %ud)", frame.x, frame.y, frame.w, frame.h);
	EE_ASSERT(x < frame.w && y < frame.h, "Invalid frame coordinates (%zu, %zu) for frame size (%u, %u)", x, y, frame.w, frame.h);

	return &frame.src->buffer[((y + frame.y) * frame.src->w + x + frame.x) * frame.src->elem_size];
}

#endif // EE_GRID_H
