#ifndef EE_PROFILER_H
#define EE_PROFILER_H

#ifdef _WIN32
#include "windows.h"

#define EE_PROF_GET_TICKS(tick_ptr)                           (QueryPerformanceCounter(tick_ptr))
#define EE_PROF_GET_FREQ(freq_ptr)                            (QueryPerformanceFrequency(freq_ptr)) 
#define EE_PROF_TICKS_TO_SEC(ticks_start, ticks_end, freq)    ((f64)((ticks_end).QuadPart - (ticks_start).QuadPart) / (f64)(freq).QuadPart)

#else

#endif

#endif // EE_PROFILER_H