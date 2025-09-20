#include "stdio.h"

#include "ee_array.h"
#include "ee_string.h"

void run_hello_world()
{
	Str test_0 = ee_str_from_cstr("asd", NULL);
	Str test_1 = ee_str_from_cstr("afd", NULL);

	ee_str_lev_m64();
}
