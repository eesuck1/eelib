#include "stdio.h"

#include "ee_array.h"
#include "ee_string.h"

void run_hello_world()
{
	Str a = ee_str_from_cstr("string", NULL);
	Str b = ee_str_from_cstr("quite longer text string", NULL);

	//printf("Edit distance: %d\n", ee_str_lev_m64(&a, &b));
	
	size_t i = ee_str_find(&b, &a);

	printf("%zu\n", i);
}
