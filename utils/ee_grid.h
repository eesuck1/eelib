#pragma once

#ifndef EE_GRID_H
#define EE_GRID_H

#include "ee_array.h"
#include "ee_dict.h"
#include "ee_heap.h"

#ifndef EE_INF
#include "math.h"
#define EE_INF                    (INFINITY)
#endif // EE_INF

#define EE_GRID_DT(x)             ((u8*)(&(x)))
#define EE_SQRT_2                 (1.41421356237f)
#define EE_OCTILE_C               (EE_SQRT_2 - 2.0f)
#define EE_SEARCH_EPS             (1e-6f)
#define EE_SEARCH_NEIGHS_COUNT    (8)

typedef struct Grid
{
	u8* buffer;

	i32 w;
	i32 h;
	size_t elem_size;

	Allocator allocator;
} Grid;

typedef struct Frame
{
	Grid* src;

	i32 x;
	i32 y;
	i32 w;
	i32 h;
} Frame;

typedef struct GridPos
{
	i32 x;
	i32 y;
} GridPos;

typedef struct GridNode
{
	GridPos pos;
	f32 cost;
} GridNode;

EE_EXTERN_C_START

typedef f32 (*GridCost)(Grid* grid, i32 x_0, i32 y_0, i32 x_1, i32 y_1);

static const i32 EE_SEARCH_NEIGHS[EE_SEARCH_NEIGHS_COUNT][2] = {
	{ 1, 0 }, { -1, 0 }, {  0, 1 }, {  0, -1 },
	{ 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 }
};

EE_INLINE i32 ee_clip_s32(i32 x, i32 a, i32 b)
{
	EE_ASSERT(a < b, "Invalid bounds (%d, %d)", a, b);

	if (x < a)
		x = a;
	else if (x > b)
		x = b;

	return x;
}

EE_INLINE Grid ee_grid_new(i32 width, i32 height, size_t elem_size, Allocator* allocator)
{
	Grid out = { 0 };

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

	size_t size = (size_t)width * height * elem_size;

	out.buffer = (u8*)out.allocator.alloc_fn(&out.allocator, size);
	out.w = width;
	out.h = height;
	out.elem_size = elem_size;

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Grid.buffer", elem_size * width * height);

	memset(out.buffer, 0, size);

	return out;
}

EE_INLINE void ee_grid_free(Grid* grid)
{
	EE_ASSERT(grid != NULL, "Trying to free NULL grid");

	free(grid->buffer);
	memset(grid, 0, sizeof(Grid));
}

EE_INLINE void ee_grid_set(Grid* grid, i32 x, i32 y, u8* val)
{
	EE_ASSERT(grid != NULL, "Trying to set into NULL grid");
	EE_ASSERT(val != NULL, "Trying to set NULL value");
	EE_ASSERT(x < grid->w && y < grid->h, "Invalid x, y value (%d, %d) for grid with size (%d, %d)", x, y, grid->w, grid->h);

	memcpy(&grid->buffer[((size_t)y * grid->w + x) * grid->elem_size], val, grid->elem_size);
}

EE_INLINE u8* ee_grid_at(Grid* grid, i32 x, i32 y)
{
	EE_ASSERT(grid != NULL, "Trying to get from NULL grid");
	EE_ASSERT(x < grid->w && y < grid->h, "Invalid x, y value (%d, %d) for grid with size (%d, %d)", x, y, grid->w, grid->h);

	return &grid->buffer[((size_t)y * grid->w + x) * grid->elem_size];
}

EE_INLINE Frame ee_grid_frame(Grid* grid, i32 left_x, i32 top_y, i32 width, i32 height)
{
	Frame out = { 0 };
	
	i32 min_x = ee_clip_s32(left_x, 0, grid->w);
	i32 min_y = ee_clip_s32(top_y, 0, grid->h);
	i32 max_x = ee_clip_s32(left_x + width, 0, grid->w);
	i32 max_y = ee_clip_s32(top_y + height, 0, grid->h);

	EE_ASSERT(min_x != max_x || min_y != max_y, "Trying to create an empty frame (%d, %d, %d, %d)", left_x, top_y, width, height);

	out.src = grid;
	out.x = min_x;
	out.y = min_y;
	out.w = max_x - min_x;
	out.h = max_y - min_y;

	return out;
}

EE_INLINE void ee_frame_set(Frame frame, i32 x, i32 y, u8* val)
{
	EE_ASSERT(frame.w != 0 && frame.h != 0, "Trying to set into empty frame (%d, %d, %d, %d)", frame.x, frame.y, frame.w, frame.h);
	EE_ASSERT(x >= 0 && y >= 0 && x < frame.w && y < frame.h, "Invalid frame coordinates (%d, %d) for frame size (%d, %d)", x, y, frame.w, frame.h);

	u8* dest = &frame.src->buffer[(((size_t)y + frame.y) * frame.src->w + x + frame.x) * frame.src->elem_size];
	memcpy(dest, val, frame.src->elem_size);
}

EE_INLINE u8* ee_frame_at(Frame frame, i32 x, i32 y)
{
	EE_ASSERT(frame.w != 0 && frame.h != 0, "Trying to get from empty frame (%d, %d, %d, %d)", frame.x, frame.y, frame.w, frame.h);
	EE_ASSERT(x >= 0 && y >= 0 && x < frame.w && y < frame.h, "Invalid frame coordinates (%d, %d) for frame size (%d, %d)", x, y, frame.w, frame.h);

	return &frame.src->buffer[((y + frame.y) * frame.src->w + x + frame.x) * frame.src->elem_size];
}

EE_INLINE int ee_grid_cost_cmp(const void* a_ptr, const void* b_ptr)
{
	const GridNode* a = (const GridNode*)a_ptr;
	const GridNode* b = (const GridNode*)b_ptr;

	if (a->cost < b->cost)
	{
		return -1;
	}
	
	if (a->cost > b->cost)
	{
		return 1;
	}

	return 0;
}

EE_INLINE f32 ee_octile(i32 x_0, i32 y_0, i32 x_1, i32 y_1)
{
	i32 dx = abs(x_1 - x_0);
	i32 dy = abs(y_1 - y_0);

	f32 min_d = dx < dy ? (f32)dx : (f32)dy;
	f32 sum = (f32)(dx + dy);

	return sum + EE_OCTILE_C * min_d;
}

EE_INLINE Array ee_grid_search(Grid* grid, i32 x_0, i32 y_0, i32 x_1, i32 y_1, GridCost step_cost_fn)
{
	f32 dist = ee_octile(x_0, y_0, x_1, y_1);
	size_t start_size = (size_t)(dist * dist * 2.0f + 16.0f);

	Array out_path = ee_array_new((size_t)(dist * 2.0f), sizeof(GridNode), &grid->allocator);
	Heap open_set = ee_heap_new(start_size, sizeof(GridNode), ee_grid_cost_cmp, &grid->allocator);

	Dict score = ee_dict_new_m(start_size, GridPos, f64, &grid->allocator, NULL);
	Dict parent = ee_dict_new_m(start_size, GridPos, GridPos, &grid->allocator, NULL);
	Dict closed = ee_dict_new_m(start_size, GridPos, u64, &grid->allocator, NULL);

	GridPos start_pos = { x_0, y_0 };
	GridNode start_node = { start_pos, dist };

	ee_heap_push(&open_set, EE_HEAP_DT(start_node));
	ee_dict_set(&score, EE_RECAST_U8(start_pos), EE_CONST_ZERO_F64);

	GridNode current = { 0 };

	while (!ee_heap_empty(&open_set))
	{
		ee_heap_pop(&open_set, EE_HEAP_DT(current));

		if (current.pos.x == x_1 && current.pos.y == y_1)
		{
			GridPos pos = current.pos;

			while (ee_dict_contains(&parent, EE_RECAST_U8(pos)))
			{
				f64* score_ptr = (f64*)ee_dict_at(&score, EE_RECAST_U8(pos));
				f64 out_score = score_ptr == NULL ? EE_INF : *score_ptr;

				GridNode out_node = { pos, (f32)out_score };

				ee_array_push(&out_path, EE_RECAST_U8(out_node));
				pos = *(GridPos*)ee_dict_at(&parent, EE_RECAST_U8(pos));
			}

			f64* score_ptr = (f64*)ee_dict_at(&score, EE_RECAST_U8(start_pos));
			f64 out_score = score_ptr == NULL ? EE_INF : *score_ptr;

			GridNode out_node = { start_pos, (f32)out_score };

			ee_array_push(&out_path, EE_RECAST_U8(out_node));
			ee_array_reverse(&out_path);

			break;
		}

		if (ee_dict_contains(&closed, EE_RECAST_U8(current.pos)))
		{
			continue;
		}

		f64* current_score_ptr = (f64*)ee_dict_at(&score, EE_RECAST_U8(current.pos));
		f32 current_score = EE_INF;

		if (current_score_ptr != NULL)
		{
			current_score = (f32)(*current_score_ptr);
		}

		f32 expected_cost = current_score + ee_octile(current.pos.x, current.pos.y, x_1, y_1);
		f32 cost_diff = expected_cost - current.cost;
		
		if (cost_diff > EE_SEARCH_EPS || cost_diff < -EE_SEARCH_EPS)
		{
			continue;
		}

		ee_dict_set(&closed, EE_RECAST_U8(current.pos), EE_CONST_ONE);

		for (int i = 0; i < EE_SEARCH_NEIGHS_COUNT; ++i)
		{
			i32 neigh_x = current.pos.x + EE_SEARCH_NEIGHS[i][0];
			i32 neigh_y = current.pos.y + EE_SEARCH_NEIGHS[i][1];

			if (neigh_x < 0 || neigh_y < 0 || neigh_x >= grid->w || neigh_y >= grid->h)
			{
				continue;
			}

			GridPos neigh_pos = { neigh_x, neigh_y };

			if (ee_dict_contains(&closed, EE_RECAST_U8(neigh_pos)))
			{
				continue;
			}

			f32 step_cost = step_cost_fn(grid, current.pos.x, current.pos.y, neigh_x, neigh_y);
			f32 tent_cost = current_score + step_cost;

			f64* neigh_score_ptr = (f64*)ee_dict_at(&score, EE_RECAST_U8(neigh_pos));
			f32 neigh_score = EE_INF;

			if (neigh_score_ptr != NULL)
			{
				neigh_score = (f32)(*neigh_score_ptr);
			}

			if (tent_cost < neigh_score)
			{
				f64 neigh_score_f64 = tent_cost;
				f32 f_score = tent_cost + ee_octile(neigh_x, neigh_y, x_1, y_1);
				
				GridNode next_node = { neigh_pos , f_score };

				ee_dict_set(&score, EE_RECAST_U8(neigh_pos), EE_RECAST_U8(neigh_score_f64));
				ee_dict_set(&parent, EE_RECAST_U8(neigh_pos), EE_RECAST_U8(current.pos));
				ee_heap_push(&open_set, EE_HEAP_DT(next_node));
			}
		}
	}

	ee_dict_free(&score);
	ee_dict_free(&parent);
	ee_dict_free(&closed);
	ee_heap_free(&open_set);

	return out_path;
}

EE_INLINE size_t ee_grid_subpath(Array* path, f32 max_cost)
{
	EE_ASSERT(path != NULL, "Trying to find subpath in NULL path");
	EE_ASSERT(path->buffer != NULL, "Trying to find subpath in path with NULL buffer (could be that path is not found)");

	size_t lo = 0;
	size_t hi = ee_array_len(path);
	size_t out = 0;

	while (lo < hi)
	{
		size_t mid = (lo + hi) >> 1;
		GridNode* node = (GridNode*)ee_array_at(path, mid);

		if (node->cost <= max_cost)
		{
			out = mid;
			lo = mid + 1;
		}
		else
		{
			hi = mid;
		}
	}

	return out;
}

EE_INLINE Grid ee_grid_from_frame(Frame frame)
{
	EE_ASSERT(frame.src != NULL, "Trying to copy from NULL src frame");

	Grid out = { 0 };

	out.buffer = (u8*)malloc((size_t)frame.h * frame.w * frame.src->elem_size);
	out.elem_size = frame.src->elem_size;
	out.w = frame.w;
	out.h = frame.h;

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Grid.buffer", frame.h * frame.src->elem_size * frame.w);

	for (i32 y = 0; y < frame.h; ++y)
	{
		memcpy(&out.buffer[y * frame.w * frame.src->elem_size], ee_frame_at(frame, 0, y), frame.w * frame.src->elem_size);
	}

	return out;
}

EE_INLINE void ee_frame_fill(Frame frame, u8* val)
{
	EE_ASSERT(val != NULL, "Trying to fill frame with NULL value");
	EE_ASSERT(frame.src != NULL, "Trying to fill NULL src frame");

	for (i32 x = 0; x < frame.w; ++x)
	{
		ee_frame_set(frame, x, 0, val);
	}

	for (i32 y = 1; y < frame.h; ++y)
	{
		memcpy(ee_frame_at(frame, 0, y), ee_frame_at(frame, 0, 0), frame.w * frame.src->elem_size);
	}
}

EE_INLINE void ee_grid_set_zero(Grid* grid, i32 x, i32 y)
{
	memset(ee_grid_at(grid, x, y), 0, grid->elem_size);
}

EE_EXTERN_C_END

#endif // EE_GRID_H
