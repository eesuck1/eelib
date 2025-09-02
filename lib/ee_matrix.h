#pragma once

#ifndef EE_MATRIX_H
#define EE_MATRIX_H

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
#define EE_TRUE     (1)
#endif // EE_TRUE

#ifndef EE_FALSE
#define EE_FALSE    (0)
#endif // EE_FALSE

#ifndef EE_MAT_DT
#define EE_MAT_DT(x)    ((uint8_t*)(&(x)))
#endif // EE_MAT_DT

typedef struct Matrix
{
	size_t rows;
	size_t cols;
	size_t elem_size;
	uint8_t* buffer;
} Matrix;

EE_INLINE Matrix ee_mat_new(size_t rows, size_t cols, size_t elem_size)
{
	EE_ASSERT(rows > 0, "Invalid number of rows (%zu)", rows);
	EE_ASSERT(cols > 0, "Invalid number of columns (%zu)", cols);
	EE_ASSERT(elem_size > 0, "Invalid matrix elem_size (%zu)", elem_size);

	Matrix out = { 0 };

	out.rows = rows;
	out.cols = cols;
	out.elem_size = elem_size;
	out.buffer = (uint8_t*)malloc(rows * cols * elem_size);

	EE_ASSERT(out.buffer != NULL, "Unable to allocate (%zu) bytes for Matrix.buffer", rows * cols * elem_size);

	return out;
}

EE_INLINE void ee_mat_free(Matrix* mat)
{
	EE_ASSERT(mat != NULL, "Trying to free NULL Vec");
	EE_ASSERT(mat->buffer != NULL, "Trying to free NULL Matrix.buffer");

	free(mat->buffer);

	memset(mat, 0, sizeof(Matrix));
}

EE_INLINE size_t ee_mat_index(Matrix* mat, size_t row, size_t col)
{
	return (row * mat->cols + col) * mat->elem_size;
}

EE_INLINE Matrix ee_mat_set(Matrix* mat, size_t row, size_t col, uint8_t* item)
{
	EE_ASSERT(mat != NULL, "Trying to access NULL Matrix");

	memcpy(&mat->buffer[ee_mat_index(mat, row, col)], item, mat->elem_size);
}

EE_INLINE uint8_t* ee_mat_at(Matrix* mat, size_t row, size_t col)
{
	EE_ASSERT(mat != NULL, "Trying to access NULL Matrix");

	return &mat->buffer[ee_mat_index(mat, row, col)];
}

#endif // EE_MATRIX_H
