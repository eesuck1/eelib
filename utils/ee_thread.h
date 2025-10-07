#ifndef EE_THREAD_H
#define EE_THREAD_H

#include "windows.h"

#include "ee_core.h"
#include "ee_deq.h"

#define EE_STDCALL                 __stdcall
#define EE_MAX_WORKERS             (64)
#define EE_M_STACK_BASE_SIZE       (EE_NKB(2))
#define EE_G_STACK_BASE_SIZE       (EE_NKB(2))
#define EE_M_DEQ_BASE_SIZE         (256)
#define EE_INVALID_ID              (-1)
#define EE_W_NUM_SIMD_ROUND_32     ((EE_MAX_WORKERS % (EE_SIMD_BYTES / 4)) == 0)

typedef enum TaskState
{
    EE_TASK_WAITING   = 0,
    EE_TASK_SUSPENDED = 1,
    EE_TASK_RUNNING   = 2,
    EE_TASK_FINISHED  = 3,
} TaskState;

typedef void (*TaskFn)(LPVOID context);

typedef struct Task
{
    LPVOID parent_fiber;
    LPVOID fiber;
    LPVOID context;
    TaskFn fn;
    TaskState state;
} Task;

typedef struct Worker
{
    DWORD  id;
    HANDLE thread;
    HANDLE deq_event;
    LPVOID main_fiber;
    Deq    tasks;
} Worker;

typedef struct Dispatcher
{
    Worker* workers;
    i32 w_count;
    Allocator allocator;
} Dispatcher;

/*

Thoughts

Static deque with head\tail starting from center
No TLS
False sharing in static buffers
Add EE_MT_IMPL for safe include into multiple .C
Cross-Platform (later)

*/

EE_ALIGNAS(EE_SIMD_BYTES) static DWORD _w_id_map[EE_MAX_WORKERS] = { 0 };
EE_ALIGNAS(EE_SIMD_BYTES) static DWORD _w_tls[EE_MAX_WORKERS]    = { 0 };
EE_ALIGNAS(EE_SIMD_BYTES) static DWORD _t_tls[EE_MAX_WORKERS]    = { 0 };

static Dispatcher _disp = { 0 };

EE_INLINE i32 ee_get_cpu_count(void)
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);

    return info.dwNumberOfProcessors;
}

EE_INLINE i32 ee_get_current_id(void)
{
#if !EE_W_NUM_SIMD_ROUND_32
    i32 upper = ee_round_down_pow2(EE_MAX_WORKERS, (EE_SIMD_BYTES / 4));
#else
    i32 upper = EE_MAX_WORKERS;
#endif

    DWORD thread_id = GetCurrentThreadId();
    ee_simd_i pattern = ee_set1_epi32(thread_id);
    i32 i = 0;

    for (; i < upper; i += (EE_SIMD_BYTES / 4))
    {
        ee_simd_i group = ee_load_si((const ee_simd_i*)&_w_id_map[i]);
        ee_simd_i match = ee_cmpeq_epi32(pattern, group);

        i32 mask = ee_movemask_epi8(match);

        if (mask)
        {
            i32 first = ee_first_bit_u32(mask);

            return i + (first >> 2);
        }
    }

#if !EE_W_NUM_SIMD_ROUND_32
    for (; i < EE_MAX_WORKERS; ++i)
    {
        if (_w_id_map[i] == thread_id)
        {
            return i;
        }
    }
#endif

    return EE_INVALID_ID;
}

EE_INLINE LPVOID WINAPI ee_task_trampoline(LPVOID context)
{
    EE_ASSERT(context != NULL, "Trying to dereference NULL context");

    return NULL;
}

EE_INLINE DWORD WINAPI ee_worker_entry(LPVOID context)
{
    Worker* worker = (Worker*)context;
    worker->main_fiber = ConvertThreadToFiber(NULL);

    i32 id = ee_get_current_id();

    EE_PRINTLN("True ID (%d) Thread (%d) Worker TLS (%d) Task TLS (%d)", worker->id, id, _w_tls[id], _t_tls[id]);
    Sleep(1000);

    EE_PRINTLN("True ID (%d) Thread (%d) Worker TLS (%d) Task TLS (%d)", worker->id, id, _w_tls[id], _t_tls[id]);
    Sleep(1000);
}

EE_INLINE i32 ee_disp_is_init(void)
{
    static const Dispatcher stab = { 0 };

    return memcmp(&_disp, &stab, sizeof(Dispatcher)) != 0;
}

EE_INLINE void ee_disp_init(Allocator* allocator)
{
    if (ee_disp_is_init())
    {
        EE_ASSERT(0, "Reinitialization of global dispatcher is forbidden");
        return;
    }

    if (!allocator)
    {
        _disp.allocator.alloc_fn = ee_default_alloc;
        _disp.allocator.realloc_fn = ee_default_realloc;
        _disp.allocator.free_fn = ee_default_free;
        _disp.allocator.context = NULL;
    }
    else
    {
        memcpy(&_disp.allocator, allocator, sizeof(Allocator));
    }

    _disp.w_count = ee_get_cpu_count();
    _disp.workers = _disp.allocator.alloc_fn(&_disp.allocator, _disp.w_count * sizeof(*_disp.workers));

    EE_ASSERT(_disp.workers != NULL, "Unable to allocate (%zu) bytes for workers buffer", _disp.w_count * sizeof(*_disp.workers));

    for (i32 i = 0; i < _disp.w_count; ++i)
    {
        Worker* worker = &_disp.workers[i];

        DWORD thread_id = -1;
        HANDLE m_thread = CreateThread(NULL, EE_M_STACK_BASE_SIZE, ee_worker_entry, worker, CREATE_SUSPENDED, &thread_id);
        HANDLE deq_event = CreateEventA(NULL, FALSE, FALSE, NULL);

        EE_ASSERT(m_thread != NULL, "Unable to create thread for worker (%d)", i);
        EE_ASSERT(deq_event != NULL, "Unable to create event for worker (%d)", i);

        worker->id = i;
        worker->thread = m_thread;
        worker->deq_event = deq_event;
        worker->main_fiber = NULL;
        worker->tasks = ee_deq_new(EE_M_DEQ_BASE_SIZE, sizeof(Task), &_disp.allocator);

        _w_id_map[i] = thread_id;
        _w_tls[i] = TlsAlloc();
        _t_tls[i] = TlsAlloc();

        ResumeThread(m_thread);
    }
}

EE_INLINE void ee_disp_free()
{
    EE_ASSERT(ee_disp_is_init(), "Trying to access uninitialized dispatcher");

    memset(&_disp, 0, sizeof(Dispatcher));
}

EE_INLINE i32 ee_mt_min_load_worker()
{
    EE_ASSERT(ee_disp_is_init(), "Trying to access uninitialized dispatcher");
    
    size_t min_val = ee_deq_len(&_disp.workers[0].tasks);
    i32 out = 0;

    for (i32 i = 1; i < _disp.w_count; ++i)
    {
        size_t val = ee_deq_len(&_disp.workers[i].tasks);

        if (val < min_val) 
        { 
            min_val = val; 
            out = i; 
        }
    }

    return out;
}

EE_INLINE i32 ee_mt_max_load_worker()
{
    EE_ASSERT(ee_disp_is_init(), "Trying to access uninitialized dispatcher");
    
    size_t max_val = ee_deq_len(&_disp.workers[0].tasks);
    i32 out = 0;

    for (i32 i = 1; i < _disp.w_count; ++i)
    {
        size_t val = ee_deq_len(&_disp.workers[i].tasks);
        
        if (val > max_val) 
        { 
            max_val = val; 
            out = i; 
        }
    }

    return out;
}

EE_INLINE i32 ee_mt_go(TaskFn fn, LPVOID context)
{
    EE_ASSERT(ee_disp_is_init(), "Trying to access uninitialized dispatcher");
    
    i32 w_id = ee_mt_min_load_worker();



    return EE_TRUE;
}

EE_INLINE void ee_mt_yield(void)
{
    EE_ASSERT(ee_disp_is_init(), "Trying to access uninitialized dispatcher");
}

EE_INLINE void ee_mt_wait_all()
{
    EE_ASSERT(ee_disp_is_init(), "Trying to access uninitialized dispatcher");
    
    for (i32 i = 0; i < _disp.w_count; ++i)
    {
        WaitForSingleObject(_disp.workers[i].thread, INFINITE);
    }
}

#endif // EE_THREAD_H
