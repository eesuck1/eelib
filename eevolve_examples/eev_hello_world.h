#include "stdio.h"

#include "ee_array.h"
#include "ee_string.h"

void run_hello_world()
{
	Str f = ee_str_from_cstr("test test test", NULL);
	Str t = ee_str_from_cstr("test", NULL);

	printf("Count %zu\n", ee_str_find_b(&f, &t, 1, 14));
}
