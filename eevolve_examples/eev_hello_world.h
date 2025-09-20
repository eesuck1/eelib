#include "stdio.h"

#include "ee_array.h"
#include "ee_string.h"

void run_hello_world()
{
	Str f = ee_str_from_cstr("ababab", NULL);
	Str t = ee_str_from_cstr("abab", NULL);

	printf("Count %zu\n", ee_str_count(&f, &t));
}
