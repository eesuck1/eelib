//#define EE_NO_ASSERT

#include "stdio.h"
#include "stdint.h"
#include "stdalign.h"
#include "time.h"
#include "windows.h"
#include "assert.h"

#include "ee_string.h"
#include "ee_dict.h"
#include "ee_array.h"
#include "ee_grid.h"
#include "ee_heap.h"

#define DICT_SIZE    (1024)
#define ARRAY_SIZE     (1024 * 1024)
#define REPETITIONS  (50)

typedef struct { uint64_t a, b; } s128;
typedef struct { uint64_t a, b, c, d; } s256;

static void measure_array()
{
    Array v = ee_array_new(ARRAY_SIZE + ARRAY_SIZE / 2, sizeof(size_t), NULL);
    LARGE_INTEGER freq, start, end;
    double elapsed;

    QueryPerformanceFrequency(&freq);

    for (size_t i = 0; i < ARRAY_SIZE; ++i)
    {
        ee_array_push(&v, EE_ARRAY_DT(i));
    }

    volatile size_t sum_1 = 0;
    volatile size_t sum_2 = 0;

    QueryPerformanceCounter(&start);
    for (size_t i = 0; i < ARRAY_SIZE; ++i)
    {
        sum_1 += *(size_t*)ee_array_at(&v, i);
    }
    QueryPerformanceCounter(&end);

    elapsed = (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;
    printf("Array at: %f [ns/op]\n", elapsed * 1e9 / (double)ARRAY_SIZE);

    size_t* buffer = (size_t*)v.buffer;

    QueryPerformanceCounter(&start);
    for (size_t i = 0; i < ARRAY_SIZE; ++i)
    {
        sum_2 += buffer[i];
    }
    QueryPerformanceCounter(&end);

    elapsed = (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;
    printf("Plain view: %f [ns/op]\n", elapsed * 1e9 / (double)ARRAY_SIZE);
    printf("Right: %d\n", sum_1 == sum_2);

    size_t sum_3 = sum_1 + sum_2;
}

static void validate_arrayfind()
{
    Array v = ee_array_new(10, sizeof(size_t), NULL);

    for (size_t i = 0; i < ARRAY_SIZE; ++i)
    {
        ee_array_push(&v, EE_ARRAY_DT(i));
    }

    size_t all_good = EE_TRUE;

    for (size_t i = 0; i < ARRAY_SIZE; ++i)
    {
        size_t good = ee_array_find(&v, EE_ARRAY_DT(i)) == i;

        if (!good)
        {
            printf("Wrong value: %zu\n", i);
        }

        all_good = all_good && good;
    }

    printf("All Good: %zu\n", all_good);
}

static void validate_arrayfind_128()
{
    Array v = ee_array_new(10, sizeof(s128), NULL);

    for (size_t i = 0; i < ARRAY_SIZE; ++i)
    {
        s128 val = { i, i };

        ee_array_push(&v, EE_ARRAY_DT(val));
    }

    size_t all_good = EE_TRUE;

    for (size_t i = 0; i < ARRAY_SIZE; ++i)
    {
        s128 val = { i, i };
        size_t good = ee_array_find(&v, EE_ARRAY_DT(val)) == i;

        if (!good)
        {
            printf("Wrong value: %zu\n", i);
        }

        all_good = all_good && good;
    }

    printf("All Good: %zu\n", all_good);
}

static void validate_arrayfind_256()
{
    Array v = ee_array_new(10, sizeof(s256), NULL);

    for (size_t i = 0; i < ARRAY_SIZE; ++i)
    {
        s256 val = { i, i };

        ee_array_push(&v, EE_ARRAY_DT(val));
    }

    size_t all_good = EE_TRUE;

    for (size_t i = 0; i < ARRAY_SIZE; ++i)
    {
        s256 val = { i, i };
        size_t good = ee_array_find(&v, EE_ARRAY_DT(val)) == i;

        if (!good)
        {
            printf("Wrong value: %zu\n", i);
        }

        all_good = all_good && good;
    }

    printf("All Good: %zu\n", all_good);
}

static void benchmark_array_iteration() 
{
    Array array = ee_array_new(ARRAY_SIZE, sizeof(size_t), NULL);

    for (size_t i = 0; i < ARRAY_SIZE; ++i) 
    {
        size_t val = i;
        ee_array_push(&array, (uint8_t*)&val);
    }

    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);

    double total_time = 0.0;

    for (size_t r = 0; r < REPETITIONS; ++r) 
    {
        volatile size_t sum = 0;
        QueryPerformanceCounter(&start);

        for (size_t i = 0; i < ARRAY_SIZE; ++i) 
        {
            size_t* ptr = (size_t*)ee_array_at(&array, i);
            sum += *ptr;
        }

        QueryPerformanceCounter(&end);
        double elapsed = (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;
        total_time += elapsed;
    }

    double avg_time_s = total_time / REPETITIONS;
    double ns_per_elem = avg_time_s * 1e9 / ARRAY_SIZE;

    printf("-- ee_array iteration benchmark --\n");
    printf("Average ns per element: %f\n", ns_per_elem);
    printf("Elements per second: %f\n", 1e9 / ns_per_elem);

    ee_array_free(&array);
}

static int cmp_s128(const void* p1, const void* p2)
{
    const s128* a = (const s128*)p1;
    const s128* b = (const s128*)p2;
    return (a->a > b->a) - (a->a < b->a);
}

static int cmp_s256(const void* p1, const void* p2)
{
    const s256* a = (const s256*)p1;
    const s256* b = (const s256*)p2;
    return (a->a > b->a) - (a->a < b->a);
}

static void benchmark_array_sort_s128_s256(size_t array_size, size_t repetitions)
{
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);

    double total_arraysort_128 = 0.0;
    double total_qsort_128 = 0.0;
    double total_arraysort_256 = 0.0;
    double total_qsort_256 = 0.0;

    for (size_t r = 0; r < repetitions; ++r)
    {
        Array v128 = ee_array_new(array_size, sizeof(s128), NULL);
        for (size_t i = 0; i < array_size; ++i)
        {
            s128 val = { (uint64_t)rand(), (uint64_t)rand() };
            ee_array_push(&v128, &val);
        }

        QueryPerformanceCounter(&start);
        ee_array_sort(&v128, cmp_s128, EE_DEFAULT);
        QueryPerformanceCounter(&end);
        total_arraysort_128 += (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
        ee_array_free(&v128);

        s128* arr128 = (s128*)malloc(array_size * sizeof(s128));
        for (size_t i = 0; i < array_size; ++i)
        {
            arr128[i].a = rand();
            arr128[i].b = rand();
        }

        QueryPerformanceCounter(&start);
        qsort(arr128, array_size, sizeof(s128), cmp_s128);
        QueryPerformanceCounter(&end);
        total_qsort_128 += (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
        free(arr128);
        
        Array v256 = ee_array_new(array_size, sizeof(s256), NULL);
        for (size_t i = 0; i < array_size; ++i)
        {
            s256 val = { (uint64_t)rand(), (uint64_t)rand(), (uint64_t)rand(), (uint64_t)rand() };
            ee_array_push(&v256, &val);
        }

        QueryPerformanceCounter(&start);
        ee_array_sort(&v256, cmp_s256, EE_DEFAULT);
        QueryPerformanceCounter(&end);
        total_arraysort_256 += (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
        ee_array_free(&v256);
        
        s256* arr256 = (s256*)malloc(array_size * sizeof(s256));
        for (size_t i = 0; i < array_size; ++i)
        {
            arr256[i].a = rand();
            arr256[i].b = rand();
            arr256[i].c = rand();
            arr256[i].d = rand();
        }

        QueryPerformanceCounter(&start);
        qsort(arr256, array_size, sizeof(s256), cmp_s256);
        QueryPerformanceCounter(&end);
        total_qsort_256 += (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
        free(arr256);
    }

    printf("-- Sorting Benchmark (array_size=%zu, repetitions=%zu) --\n", array_size, repetitions);
    printf("s128 ee_array_sort avg time: %f ms\n", total_arraysort_128 / repetitions * 1000.0);
    printf("s128 qsort      avg time: %f ms\n", total_qsort_128 / repetitions * 1000.0);
    printf("s256 ee_array_sort avg time: %f ms\n", total_arraysort_256 / repetitions * 1000.0);
    printf("s256 qsort      avg time: %f ms\n", total_qsort_256 / repetitions * 1000.0);
}

static void grid_test()
{
    int w = 13;
    int h = 9;

    Grid grid = ee_grid_new(w, h, sizeof(int), NULL);

    for (int x = 0; x < w; ++x)
    {
        for (int y = 0; y < h; ++y)
        {
            int v = x + y;

            ee_grid_set(&grid, x, y, EE_GRID_DT(v));
        }
    }

    Frame frame = ee_grid_frame(&grid, 2, 2, 3, 3);

    for (int x = 0; x < w; ++x)
    {
        for (int y = 0; y < h; ++y)
        {
            int v = *(int*)ee_grid_at(&grid, x, y);

            printf("%d ", v);

            if (v < 10)
                printf(" ");
        }

        printf("\n");
    }

    printf("\n\n");

    for (int x = 0; x < frame.w; ++x)
    {
        for (int y = 0; y < frame.h; ++y)
        {
            int v = *(int*)ee_frame_at(frame, x, y);

            printf("%d ", v);

            if (v < 10)
                printf(" ");
        }

        printf("\n");
    }

    printf("\n\n");
}


static int cmp_s32(const void* a, const void* b)
{
    int va = *(int*)a;
    int vb = *(int*)b;

    return va - vb;
}

static void heap_test()
{
    Heap h = ee_heap_new(10, sizeof(int), cmp_s32, NULL);

    printf("Inserting elements:\n");
    for (int i = 0; i < 10; ++i) {
        int v = rand() % 100;
        printf("%d ", v);
        ee_heap_push(&h, EE_HEAP_DT(v));
    }
    printf("\n\n");

    printf("Popping elements in order:\n");
    while (!ee_heap_empty(&h)) {
        int min;
        ee_heap_pop(&h, (uint8_t*)&min);
        printf("%d ", min);
    }
    printf("\n");

    ee_heap_free(&h);
}

static float cost_with_obstacles(Grid* grid, int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    int* cell = (int*)ee_grid_at(grid, x1, y1);
    if (*cell != 0)
        return EE_INF;

    int dx = x1 - x0;
    int dy = y1 - y0;

    if (dx != 0 && dy != 0)
        return 1.41421356237f;
    return 1.0f;
}

static void test_grid_wall_with_gap()
{
    const int w = 25;
    const int h = 25;

    Grid grid = ee_grid_new(w, h, sizeof(int), NULL);

    int zero = 0;
    int one = 1;

    for (int x = 0; x < w; ++x)
        for (int y = 0; y < h; ++y)
            ee_grid_set(&grid, x, y, EE_GRID_DT(zero));

    int wall_x = w / 2;
    int gap_y = h / 2;

    for (int y = 0; y < h; ++y)
    {
        if (y == gap_y) continue;
        ee_grid_set(&grid, wall_x, y, EE_GRID_DT(one));
    }

    int32_t start_x = 1;
    int32_t start_y = 1;
    int32_t goal_x = 15;
    int32_t goal_y = 1;

    Array path = ee_grid_search(&grid, start_x, start_y, goal_x, goal_y, cost_with_obstacles);

    char** display = (char**)malloc(h * sizeof(char*));
    for (int y = 0; y < h; ++y)
    {
        display[y] = (char*)malloc(w * sizeof(char));
        for (int x = 0; x < w; ++x)
        {
            int* cell = (int*)ee_grid_at(&grid, x, y);
            display[y][x] = (*cell != 0) ? '#' : '.';
        }
    }

    for (size_t i = 0; i < ee_array_len(&path); ++i)
    {
        GridNode* node = (GridNode*)ee_array_at(&path, i);
        display[node->pos.y][node->pos.x] = '*';
    }

    display[start_y][start_x] = 'O';
    display[goal_y][goal_x] = 'X';

    printf("FULL PATH MAP:\n");
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
            printf("%c ", display[y][x]);
        printf("\n");
    }

    float max_cost = 10.0f;
    size_t sub_len = ee_grid_subpath(&path, max_cost);

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
            display[y][x] = (*((int*)ee_grid_at(&grid, x, y)) != 0) ? '#' : '.';
    }

    for (size_t i = 0; i <= sub_len; ++i)
    {
        GridNode* node = (GridNode*)ee_array_at(&path, i);
        display[node->pos.y][node->pos.x] = '*';
    }

    display[start_y][start_x] = 'O';

    printf("\nSUBPATH (cost <= %.1f):\n", max_cost);
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
            printf("%c ", display[y][x]);
        printf("\n");
    }

    for (int y = 0; y < h; ++y)
        free(display[y]);
    free(display);

    ee_array_free(&path);
    ee_grid_free(&grid);
}

EE_INLINE void* my_realloc(Allocator* self, void* buffer, size_t old_size, size_t new_size)
{
    printf("Reallocated from %zu to %zu\n", old_size, new_size);

    return realloc(buffer, new_size);
}

//int main()
//{
//    test_grid_wall_with_gap();
//
//    Allocator al = { 0 };
//
//    al.alloc_fn = ee_default_alloc;
//    al.realloc_fn = my_realloc;
//    al.free_fn = ee_default_free;
//
//    Array array = ee_array_new(8, 4, &al);
//
//    for (int i = 0; i < 1 << 20; ++i)
//    {
//        ee_array_push(&array, EE_ARRAY_DT(i));
//    }
//
//    return 0;
//}