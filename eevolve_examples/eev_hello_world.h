#include "stdio.h"

#include "ee_array.h"
#include "ee_string.h"

void run_hello_world()
{
	Str target = ee_str_from("HELLO WORLD!");

	printf("Hello, World!\n");
}
