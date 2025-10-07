#ifndef EE_DICT_EXAMPLE_H
#define EE_DICT_EXAMPLE_H

#include "ee_dict.h"

// 16-byte key structure
typedef struct
{
	u64 low;
	u64 high;
} Key;

void run_dict_example_hello_world(void)
{
	// Creating a hash-table with starting size of 128 values, key size 16-byte and 4-byte value
	// Default heap Allocator and default ee_hash function
	// using 'ee_dict_new_m' macro

	Dict dict = ee_dict_new_m(128, Key, f32, NULL, NULL);

	// Same as:
	// Dict dict = ee_dict_new(128, sizeof(Key), sizeof(f32), alignof(Key), alignof(f32), NULL, NULL);
	
	Key key = { 1, 2 };
	Key key_missing = { 3, 4 };
	f32 val = 3.0f;

	// Inserting (key -> value) pair into table using 'EE_RECAST_U8' macro
	// #define EE_RECAST_U8(x)    ((u8*)(&(x))) - takes the address of passed variable and casts it address to u8*
	// The key and value would be copied so it's okay to modify them afterwards
	// The function returns an EE_TRUE value on success

	i32 set_res = ee_dict_set(&dict, EE_RECAST_U8(key), EE_RECAST_U8(val));

	// Practically it will fail only if Dict ran out of memory
	EE_ASSERT(set_res == EE_TRUE, "Failed to insert (%f) into hash table", val);
	EE_PRINTLN("Inserted (%f) successfully", val);

	// Check if the value is contained within table
	i32 key_there = ee_dict_contains(&dict, EE_RECAST_U8(key));
	i32 key_not_there = ee_dict_contains(&dict, EE_RECAST_U8(key_missing));

	EE_ASSERT(key_there == EE_TRUE, "Invalid contains result, should be true");
	EE_ASSERT(key_not_there == EE_FALSE, "Invalid contains result, should be false");

	// Search function returns a raw u8 pointer to internal table memory location when value is stored
	// There are two options to obtain an actual value:
	//
	//   1) Using reinterpreted cast to desired type: *(f32*)val_at
	// 
	//   2) Copy memory from table buffer into local variable 
	//      memcpy(&dest, val_at, dict.val_len);
	// 
	// Returns NULL if key is not in table

	u8* val_at = ee_dict_at(&dict, EE_RECAST_U8(key));
	u8* val_missing = ee_dict_at(&dict, EE_RECAST_U8(key_missing));

	EE_ASSERT(val_at != NULL, "Invalid searching result, should not be NULL");
	EE_ASSERT(val_missing == NULL, "Invalid searching result, should be NULL");

	f32 val_search = *(f32*)val_at;

	EE_ASSERT(val == val_search, "Wrong value!");
	EE_PRINTLN("Found (%f) successfully", val);

	// Remove the value from the table
	i32 del_res = ee_dict_remove(&dict, EE_RECAST_U8(key));

	EE_ASSERT(del_res == EE_TRUE, "Failed to remove (%f) into hash table", val);
	EE_PRINTLN("Removed (%f) successfully", val);

	// Clear the table
	ee_dict_free(&dict);
}

#endif // EE_DICT_EXAMPLE_H
