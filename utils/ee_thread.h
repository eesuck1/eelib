#ifndef EE_THREAD_H
#define EE_THREAD_H

#include "windows.h"

#include "ee_deq.h"

#define EE_STDCALL                 __stdcall
#define EE_MAX_WORKERS             (64)
#define EE_G_STACK_BASE_SIZE       (EE_NKB(2))
#define EE_M_STACK_BASE_SIZE       (EE_NKB(2))
#define EE_M_DEQ_BASE_SIZE         (256)
#define EE_INVALID_ID              (-1)
#define EE_W_NUM_SIMD_ROUND_32     ((EE_MAX_WORKERS % (EE_SIMD_BYTES / 4)) == 0)
#define EE_W_DEQ_TIMEOUT           (10)
#define EE_MT_STOP_TIMEOUT         (10)

typedef enum TaskState
{
    EE_TASK_INITS = 0,
    EE_TASK_WAITING,
    EE_TASK_SUSPENDED,
    EE_TASK_RUNNING,
    EE_TASK_FINISHED,
} TaskState;

typedef enum WorkerState
{
    EE_WORKER_INITS,
    EE_WORKER_BUSY,
    EE_WORKER_FREE,
} WorkerState;

typedef void (*TaskFn)(LPVOID context);

typedef struct Task
{
    LPVOID fiber;
    LPVOID context;
    TaskFn fn;
    TaskState state;
    size_t stack_size;
} Task;

typedef struct Worker
{
    DWORD   id;
    HANDLE  thread;
    HANDLE  deq_event;
    HANDLE  free_event;
    LPVOID  main_fiber;
    Deq     tasks;
    Task*   curr_task;
    WorkerState state;
} Worker;

typedef struct Dispatcher
{
    Worker* workers;
    i32 w_count;
    i32 w_running;
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

EE_ALIGNAS(EE_SIMD_BYTES) static DWORD   _id_map[EE_MAX_WORKERS]  = { 0 };

static CRITICAL_SECTION _deq_cs = { 0 };
static Dispatcher       _disp   = { 0 };

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
        ee_simd_i group = ee_load_si((const ee_simd_i*)&_id_map[i]);
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
        if (_id_map[i] == thread_id)
        {
            return i;
        }
    }
#endif

    return EE_INVALID_ID;
}

EE_INLINE i32 ee_disp_is_init(void)
{
    static const Dispatcher stab = { 0 };

    return memcmp(&_disp, &stab, sizeof(Dispatcher)) != 0;
}


EE_INLINE LPVOID WINAPI ee_task_trampoline(LPVOID context)
{
    EE_ASSERT(context != NULL, "Trying to dereference NULL context");

    i32 curr_id = ee_get_current_id();
    Task* task = _disp.workers[curr_id].curr_task;

    task->state = EE_TASK_RUNNING;
    task->fn(task->context);
    task->state = EE_TASK_FINISHED;

    EE_PRINTLN("Task Finished!");

    SwitchToFiber(_disp.workers[curr_id].main_fiber);

    return NULL;
}

EE_INLINE i32 ee_process_task(Worker* worker_ptr, Task* task_ptr)
{
    i32 task_set = EE_FALSE;

    switch (task_ptr->state)
    {
    case EE_TASK_FINISHED:
    {
        if (task_ptr->fiber != NULL)
        {
            DeleteFiber(task_ptr->fiber);
            task_ptr->fiber = NULL;
        }
    } break;
    
    case EE_TASK_SUSPENDED:
    case EE_TASK_WAITING:
    {
        worker_ptr->curr_task = task_ptr;
        task_set = EE_TRUE;
    } break;

    case EE_TASK_INITS:
    {
        while (task_ptr->state == EE_TASK_INITS); // TODO prevent deadlock

        ee_deq_push_head(&worker_ptr->tasks, (u8*)task_ptr);
    } break;
    case EE_TASK_RUNNING:
    {
        ee_deq_push_head(&worker_ptr->tasks, (u8*)task_ptr);
    } break;

    default:
    {
        EE_ASSERT(0, "Invalid state (%d)", task_ptr->state);
    }
    }

    return task_set;
}

EE_INLINE DWORD WINAPI ee_worker_entry(LPVOID context)
{
    Worker* curr_w = (Worker*)context;

    curr_w->main_fiber = ConvertThreadToFiber(NULL);
    curr_w->state = EE_WORKER_FREE;

    EE_ASSERT(curr_w->id == ee_get_current_id(), "Invalid (Thread ID -> ID) mapping");

    while (_disp.w_running)
    {
        WaitForSingleObject(curr_w->deq_event, EE_W_DEQ_TIMEOUT);

        Worker* maxl_w = NULL;

        i32 task_set = EE_FALSE;
        i32 w_max_id = -1;
        i32 w_min_id = -1;

        if (!ee_deq_empty(&curr_w->tasks))
        {
            EnterCriticalSection(&_deq_cs);
            Task* task_ptr = (Task*)ee_deq_at_tail(&curr_w->tasks);
            ee_deq_pop_tail(&curr_w->tasks, NULL);
            
            task_set = ee_process_task(curr_w, task_ptr);
            LeaveCriticalSection(&_deq_cs);
        }
        else
        {
            w_max_id = ee_mt_max_load_worker();
            w_min_id = ee_mt_min_load_worker();

            maxl_w   = &_disp.workers[w_max_id];

            size_t steal_count = ee_deq_len(&maxl_w->tasks) >> 1;

            if (w_min_id == 0 || steal_count == 0)
            {
                continue;
            }

            EE_ASSERT(maxl_w != NULL, "Trying to dereference NULL max load worker");

            EnterCriticalSection(&_deq_cs);
            for (size_t i = 0; i < steal_count; ++i)
            {
                Task* task_ptr = (Task*)ee_deq_at_tail(&maxl_w->tasks);
                ee_deq_pop_tail(&maxl_w->tasks, NULL);

                if (!task_set)
                {
                    task_set = ee_process_task(curr_w, task_ptr);
                }
                else
                {
                    ee_deq_push_head(&curr_w->tasks, (u8*)task_ptr);
                }
            }
            LeaveCriticalSection(&_deq_cs);
        }

        if (task_set)
        {
            EE_ASSERT(curr_w->curr_task != NULL, "Trying to derefernce NULL worker's curr_task");

            if (curr_w->curr_task->fiber == NULL)
            {
                curr_w->curr_task->fiber = CreateFiber(curr_w->curr_task->stack_size, ee_task_trampoline, curr_w->curr_task);
                EE_ASSERT(curr_w->curr_task->fiber != NULL, "Unable to create fiber in worker");
            }

            curr_w->state = EE_WORKER_BUSY;

            SwitchToFiber(curr_w->curr_task->fiber);
            SetEvent(curr_w->deq_event);
        }
        else
        {
            curr_w->state = EE_WORKER_FREE;
        }
    }
}

EE_INLINE void ee_disp_init(size_t max_tasks, Allocator* allocator)
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

    _disp.w_count   = ee_get_cpu_count();
    _disp.w_running = EE_TRUE;
    _disp.workers   = _disp.allocator.alloc_fn(&_disp.allocator, _disp.w_count * sizeof(*_disp.workers));

    EE_ASSERT(_disp.workers != NULL, "Unable to allocate (%zu) bytes for workers buffer", _disp.w_count * sizeof(*_disp.workers));

    InitializeCriticalSection(&_deq_cs);

    for (i32 i = 0; i < _disp.w_count; ++i)
    {
        Worker* worker = &_disp.workers[i];

        DWORD  thread_id  = -1;
        HANDLE m_thread   = CreateThread(NULL, EE_M_STACK_BASE_SIZE, ee_worker_entry, worker, CREATE_SUSPENDED, &thread_id);
        HANDLE deq_event  = CreateEventA(NULL, FALSE, FALSE, NULL);
        HANDLE free_event = CreateEventA(NULL, TRUE, TRUE, NULL);

        EE_ASSERT(m_thread != NULL, "Unable to create thread for worker (%d)", i);
        EE_ASSERT(deq_event != NULL, "Unable to create event for worker (%d)", i);

        worker->id = i;
        worker->thread = m_thread;
        worker->deq_event = deq_event;
        worker->free_event = free_event;
        worker->main_fiber = NULL;
        worker->tasks = ee_deq_new(max_tasks, sizeof(Task) , &_disp.allocator);
        worker->curr_task = NULL;
        worker->state = EE_WORKER_INITS;

        _id_map[i] = thread_id;

        ee_deq_clear(&worker->tasks, 0);
        ResumeThread(m_thread);
    }
}

EE_INLINE void ee_disp_free()
{
    EE_ASSERT(ee_disp_is_init(), "Trying to access uninitialized dispatcher");

    for (i32 i = 0; i < _disp.w_count; ++i)
    {
        Worker* w = &_disp.workers[i];

        if (w->thread != NULL)
        {
            TerminateThread(w->thread, 0);
            CloseHandle(w->thread);
            w->thread = NULL;
        }

        if (w->deq_event != NULL)
        {
            CloseHandle(w->deq_event);
            w->deq_event = NULL;
        }

        ee_deq_free(&w->tasks);
    }

    _disp.allocator.free_fn(&_disp.allocator, _disp.workers);
    _disp.workers = NULL;
    _disp.w_count = 0;

    DeleteCriticalSection(&_deq_cs);

    memset(&_disp, 0, sizeof(Dispatcher));
}

EE_INLINE i32 ee_mt_min_load_worker()
{
    EE_ASSERT(ee_disp_is_init(), "Trying to access uninitialized dispatcher");
    
    size_t min_val = ee_deq_size(&_disp.workers[0].tasks);
    i32 out = 0;

    for (i32 i = 1; i < _disp.w_count; ++i)
    {
        size_t val = ee_deq_size(&_disp.workers[i].tasks);

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
    
    size_t max_val = ee_deq_size(&_disp.workers[0].tasks);
    i32 out = 0;

    for (i32 i = 1; i < _disp.w_count; ++i)
    {
        size_t val = ee_deq_size(&_disp.workers[i].tasks);
        
        if (val > max_val) 
        { 
            max_val = val; 
            out = i; 
        }
    }

    return out;
}

EE_INLINE i32 ee_mt_go(TaskFn fn, LPVOID context, size_t stack_size)
{
    EE_ASSERT(ee_disp_is_init(), "Trying to access uninitialized dispatcher");
    
    if (stack_size < EE_G_STACK_BASE_SIZE)
    {
        stack_size = EE_G_STACK_BASE_SIZE;
    }

    i32 w_id = ee_mt_min_load_worker();

    EnterCriticalSection(&_deq_cs);
    Task* task = (Task*)ee_deq_emplace_head(&_disp.workers[w_id].tasks);

    task->state = EE_TASK_INITS;
    task->fn = fn;
    task->context = context;
    task->fiber = NULL;
    task->stack_size = stack_size;
    task->state = EE_TASK_WAITING;

    LeaveCriticalSection(&_deq_cs);
    SetEvent(_disp.workers[w_id].deq_event);

    EE_PRINTLN("Inserted Task for worker (%d)", w_id);

    return EE_TRUE;
}

EE_INLINE void ee_mt_yield(void)
{
    EE_ASSERT(ee_disp_is_init(), "Trying to access uninitialized dispatcher");

    i32 curr_id = ee_get_current_id();
    Task* task = _disp.workers[curr_id].curr_task;

    task->state = EE_TASK_SUSPENDED;

    EnterCriticalSection(&_deq_cs);
    ee_deq_push_head(&_disp.workers[curr_id].tasks, (u8*)task);
    LeaveCriticalSection(&_deq_cs);

    SwitchToFiber(_disp.workers[curr_id].main_fiber);
}

EE_INLINE void ee_mt_wait_all(void)
{
    EE_ASSERT(ee_disp_is_init(), "Trying to access uninitialized dispatcher");
    
    while (EE_TRUE)
    {
        for (i32 i = 0; i < _disp.w_count; ++i)
        {
            WaitForSingleObject(_disp.workers[i].thread, EE_MT_STOP_TIMEOUT);
        }
    }
}

#endif // EE_THREAD_H
