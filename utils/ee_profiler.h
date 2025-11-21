#ifndef EE_PROFILER_H
#define EE_PROFILER_H

#include "ee_dict.h"
#include "ee_array.h"
#include "ee_arena.h"

typedef struct Profiler
{
	const char* name;
	Linked_Array children;
	Dict timers;
} Profiler;

#endif // EE_PROFILER_H