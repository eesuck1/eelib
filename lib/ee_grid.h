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

typedef struct Frame
{
	uint8_t* buffer;

	uint32_t x;
	uint32_t y;
	uint32_t w;
	uint32_t h;
} Frame;

typedef struct Grid
{
	uint8_t* buffer;

	size_t w;
	size_t h;
	size_t elem_size;
} Grid;

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

	memcpy(&grid->buffer[y * grid->w + x], val, grid->elem_size);
}

EE_INLINE uint8_t* ee_grid_get(Grid* grid, size_t x, size_t y)
{
	EE_ASSERT(grid != NULL, "Trying to get from NULL grid");
	EE_ASSERT(x < grid->w && y < grid->h, "Invalid x, y value (%zu, %zu) for grid with size (%zu, %zu)", x, y, grid->w, grid->h);

	return &grid->buffer[y * grid->w + x];
}

#endif // EE_GRID_H
