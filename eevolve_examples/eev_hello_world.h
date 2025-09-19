#include "stdio.h"

#include "ee_array.h"
#include "ee_string.h"

void run_hello_world()
{
	Str f = ee_str_from_cstr("test", NULL);

	ee_str_to_file(&f, "test_file.txt", "w");
}
