#pragma once

#ifndef EE_GRID_H
#define EE_GRID_H

#include "ee_vec.h"
#include "ee_dict.h"
#include "ee_heap.h"

#ifndef EE_INF
#include "math.h"
#define EE_INF                    (INFINITY)
#endif // EE_INF

#define EE_GRID_DT(x)             ((uint8_t*)(&(x)))
#define EE_SQRT_2                 (1.41421356237f)
#define EE_OCTILE_C               (EE_SQRT_2 - 2.0f)
#define EE_SEARCH_EPS             (1e-6f)
#define EE_SEARCH_NEIGHS_COUNT    (8)

typedef struct Grid
{
	uint8_t* buffer;

	int32_t w;
	int32_t h;
	size_t elem_size;
} Grid;

typedef struct Frame
{
	Grid* src;

	int32_t x;
	int32_t y;
	int32_t w;
	int32_t h;
} Frame;

typedef struct GridPos
{
	int32_t x;
	int32_t y;
} GridPos;

typedef struct GridNode
{
	GridPos pos;
	float cost;
} GridNode;

typedef float (*GridCost)(Grid* grid, int32_t x_0, int32_t y_0, int32_t x_1, int32_t y_1);

static const int32_t EE_SEARCH_NEIGHS[EE_SEARCH_NEIGHS_COUNT][2] = {
	{ 1, 0 }, { -1, 0 }, {  0, 1 }, {  0, -1 },
	{ 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 }
};

EE_INLINE int32_t ee_clip_s32(int32_t x, int32_t a, int32_t b)
{
	EE_ASSERT(a < b, "Invalid bounds (%d, %d)", a, b);

	if (x < a)
		x = a;
	else if (x > b)
		x = b;

	return x;
}

EE_INLINE Grid ee_grid_new(int32_t width, int32_t height, size_t elem_size)
{
	Grid out = { 0 };

	out.buffer = (uint8_t*)calloc((size_t)width * height, elem_size);
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

EE_INLINE void ee_grid_set(Grid* grid, int32_t x, int32_t y, uint8_t* val)
{
	EE_ASSERT(grid != NULL, "Trying to set into NULL grid");
	EE_ASSERT(val != NULL, "Trying to set NULL value");
	EE_ASSERT(x < grid->w && y < grid->h, "Invalid x, y value (%d, %d) for grid with size (%d, %d)", x, y, grid->w, grid->h);

	memcpy(&grid->buffer[((size_t)y * grid->w + x) * grid->elem_size], val, grid->elem_size);
}

EE_INLINE uint8_t* ee_grid_at(Grid* grid, int32_t x, int32_t y)
{
	EE_ASSERT(grid != NULL, "Trying to get from NULL grid");
	EE_ASSERT(x < grid->w && y < grid->h, "Invalid x, y value (%d, %d) for grid with size (%d, %d)", x, y, grid->w, grid->h);

	return &grid->buffer[((size_t)y * grid->w + x) * grid->elem_size];
}

EE_INLINE Frame ee_grid_frame(Grid* grid, int32_t left_x, int32_t top_y, int32_t width, int32_t height)
{
	Frame out = { 0 };
	
	int32_t min_x = ee_clip_s32(left_x, 0, grid->w);
	int32_t min_y = ee_clip_s32(top_y, 0, grid->h);
	int32_t max_x = ee_clip_s32(left_x + width, 0, grid->w);
	int32_t max_y = ee_clip_s32(top_y + height, 0, grid->h);

	EE_ASSERT(min_x != max_x || min_y != max_y, "Trying to create an empty frame (%d, %d, %d, %d)", left_x, top_y, width, height);

	out.src = grid;
	out.x = min_x;
	out.y = min_y;
	out.w = max_x - min_x;
	out.h = max_y - min_y;

	return out;
}

EE_INLINE void ee_frame_set(Frame frame, int32_t x, int32_t y, uint8_t* val)
{
	EE_ASSERT(frame.w != 0 && frame.h != 0, "Trying to set into empty frame (%d, %d, %d, %d)", frame.x, frame.y, frame.w, frame.h);
	EE_ASSERT(x < frame.w && y < frame.h, "Invalid frame coordinates (%d, %d) for frame size (%d, %d)", x, y, frame.w, frame.h);

	uint8_t* dest = &frame.src->buffer[(((size_t)y + frame.y) * frame.src->w + x + frame.x) * frame.src->elem_size];
	memcpy(dest, val, frame.src->elem_size);
}

EE_INLINE uint8_t* ee_frame_at(Frame frame, int32_t x, int32_t y)
{
	EE_ASSERT(frame.w != 0 && frame.h != 0, "Trying to get from empty frame (%d, %d, %d, %d)", frame.x, frame.y, frame.w, frame.h);
	EE_ASSERT(x < frame.w && y < frame.h, "Invalid frame coordinates (%d, %d) for frame size (%d, %d)", x, y, frame.w, frame.h);

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

EE_INLINE float ee_octile(int32_t x_0, int32_t y_0, int32_t x_1, int32_t y_1)
{
	int32_t dx = abs(x_1 - x_0);
	int32_t dy = abs(y_1 - y_0);

	float min_d = dx < dy ? (float)dx : (float)dy;
	float sum = (float)(dx + dy);

	return sum + EE_OCTILE_C * min_d;
}

EE_INLINE Vec ee_grid_search(Grid* grid, int32_t x_0, int32_t y_0, int32_t x_1, int32_t y_1, GridCost step_cost_fn)
{
	float dist = ee_octile(x_0, y_0, x_1, y_1);
	size_t start_size = (size_t)(dist * dist * 2.0f + 16.0f);

	Vec out_path = ee_vec_new((size_t)(dist * 2.0f), sizeof(GridNode));
	Heap open_set = ee_heap_new(start_size, sizeof(GridNode), ee_grid_cost_cmp);

	Dict score = ee_dict_new(start_size);
	Dict parent = ee_dict_new(start_size);
	Dict closed = ee_dict_new(start_size);

	GridPos start_pos = { x_0, y_0 };
	GridNode start_node = { start_pos, dist };

	ee_heap_push(&open_set, EE_HEAP_DT(start_node));
	ee_dict_set(&score, EE_DICT_DT(start_pos), EE_CONST_ZERO_F64);

	GridNode current = { 0 };

	while (!ee_heap_empty(&open_set))
	{
		ee_heap_pop(&open_set, EE_HEAP_DT(current));

		if (current.pos.x == x_1 && current.pos.y == y_1)
		{
			GridPos pos = current.pos;

			while (ee_dict_contains(&parent, EE_DICT_DT(pos)))
			{
				double* score_ptr = (double*)ee_dict_at(&score, EE_DICT_DT(pos));
				double out_score = score_ptr == NULL ? EE_INF : *score_ptr;

				GridNode out_node = { pos, (float)out_score };

				ee_vec_push(&out_path, EE_VEC_DT(out_node));
				pos = *(GridPos*)ee_dict_at(&parent, EE_DICT_DT(pos));
			}

			double* score_ptr = (double*)ee_dict_at(&score, EE_DICT_DT(start_pos));
			double out_score = score_ptr == NULL ? EE_INF : *score_ptr;

			GridNode out_node = { start_pos, (float)out_score };

			ee_vec_push(&out_path, EE_VEC_DT(out_node));
			ee_vec_reverse(&out_path);

			break;
		}

		if (ee_dict_contains(&closed, EE_DICT_DT(current.pos)))
		{
			continue;
		}

		double* current_score_ptr = (double*)ee_dict_at(&score, EE_DICT_DT(current.pos));
		float current_score = EE_INF;

		if (current_score_ptr != NULL)
		{
			current_score = (float)(*current_score_ptr);
		}

		float expected_cost = current_score + ee_octile(current.pos.x, current.pos.y, x_1, y_1);
		float cost_diff = expected_cost - current.cost;
		
		if (cost_diff > EE_SEARCH_EPS || cost_diff < -EE_SEARCH_EPS)
		{
			continue;
		}

		ee_dict_set(&closed, EE_DICT_DT(current.pos), EE_CONST_ONE);

		for (int i = 0; i < EE_SEARCH_NEIGHS_COUNT; ++i)
		{
			int32_t neigh_x = current.pos.x + EE_SEARCH_NEIGHS[i][0];
			int32_t neigh_y = current.pos.y + EE_SEARCH_NEIGHS[i][1];

			if (neigh_x < 0 || neigh_y < 0 || neigh_x >= grid->w || neigh_y >= grid->h)
			{
				continue;
			}

			GridPos neigh_pos = { neigh_x, neigh_y };

			if (ee_dict_contains(&closed, EE_DICT_DT(neigh_pos)))
			{
				continue;
			}

			float step_cost = step_cost_fn(grid, current.pos.x, current.pos.y, neigh_x, neigh_y);
			float tent_cost = current_score + step_cost;

			double* neigh_score_ptr = (double*)ee_dict_at(&score, EE_DICT_DT(neigh_pos));
			float neigh_score = EE_INF;

			if (neigh_score_ptr != NULL)
			{
				neigh_score = (float)(*neigh_score_ptr);
			}

			if (tent_cost < neigh_score)
			{
				double neigh_score_f64 = tent_cost;
				float f_score = tent_cost + ee_octile(neigh_x, neigh_y, x_1, y_1);
				
				GridNode next_node = { neigh_pos , f_score };

				ee_dict_set(&score, EE_DICT_DT(neigh_pos), EE_DICT_DT(neigh_score_f64));
				ee_dict_set(&parent, EE_DICT_DT(neigh_pos), EE_DICT_DT(current.pos));
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

EE_INLINE size_t ee_grid_subpath(Vec* path, float max_cost)
{
	EE_ASSERT(path != NULL, "Trying to find subpath in NULL path");
	EE_ASSERT(path->buffer != NULL, "Trying to find subpath in path with NULL buffer (could be that path is not found)");

	size_t lo = 0;
	size_t hi = ee_vec_len(path);
	size_t out = 0;

	while (lo < hi)
	{
		size_t mid = (lo + hi) >> 1;
		GridNode* node = (GridNode*)ee_vec_at(path, mid);

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

	out.buffer = (uint8_t*)malloc((size_t)frame.h * frame.w * frame.src->elem_size);
	out.elem_size = frame.src->elem_size;
	out.w = frame.w;
	out.h = frame.h;

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Grid.buffer", frame.h * frame.src->elem_size * frame.w);

	for (size_t y = 0; y < frame.h; ++y)
	{
		memcpy(&out.buffer[y * frame.w * frame.src->elem_size], ee_frame_at(frame, 0, y), frame.w * frame.src->elem_size);
	}

	return out;
}

EE_INLINE void ee_frame_fill(Frame frame, uint8_t* val)
{
	EE_ASSERT(val != NULL, "Trying to fill frame with NULL value");
	EE_ASSERT(frame.src != NULL, "Trying to fill NULL src frame");

	for (size_t x = 0; x < frame.w; ++x)
	{
		ee_frame_set(frame, x, 0, val);
	}

	for (size_t y = 1; y < frame.h; ++y)
	{
		memcpy(ee_frame_at(frame, 0, y), ee_frame_at(frame, 0, 0), frame.w * frame.src->elem_size);
	}
}

#endif // EE_GRID_H
