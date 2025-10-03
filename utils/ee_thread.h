#ifndef EE_THREAD_H
#define EE_THREAD_H

#include "ee_core_mt.h"
#include "windows.h"

typedef struct Task
{
	void (*task_fn)(void* context);
	void* context;
} Task;

typedef struct Dispatcher
{
	HANDLE* os_threads;

	s32* thr_load_min;
	s32* thr_load_max;
	s32  thr_count;

	Allocator allocator;
} Dispatcher;

EE_INLINE s32 ee_get_cpu_count()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);

	return info.dwNumberOfProcessors;
}

EE_INLINE Dispatcher ee_disp_new(Allocator* allocator)
{
	Dispatcher out = { 0 };

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

	s32 cpu_count = ee_get_cpu_count();
	s32 upper = (cpu_count + EE_SIMD_BYTES - 1) & ~(EE_SIMD_BYTES - 1);

	//out.pool = out.allocator.alloc_fn(&out.allocator, out.pool_cap * sizeof(HANDLE));

	return out;
}

#endif // EE_THREAD_H
