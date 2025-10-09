#ifndef EE_DICT_EXAMPLE_H
#define EE_DICT_EXAMPLE_H

#include "ee_dict.h"

// 16-byte key structure
typedef struct
{
	u64 low;
	u64 high;
} Key;

// Basic example, shows how to create a table, insert and remove a value and check if the key is present
void run_dict_example_hello_world(void)
{
	// Creating a hash-table with starting size of 128 values, 16-byte key and 4-byte value
	// Default heap Allocator, default ee_hash function and default key comparison function
	// In order to specify those arguments use either 'ee_dict_new' function directly 
	// or 'ee_dict_new_m' macro with extra parameters
	// using 'ee_dict_def_m' macro

	Dict dict = ee_dict_def_m(128, Key, f32);

	// Same as:
	// Dict dict = ee_dict_new(128, sizeof(Key), sizeof(f32), alignof(Key), alignof(f32), NULL, NULL, NULL);
	
	Key key = { 1, 2 };
	Key key_missing = { 3, 4 };
	f32 val = 3.0f;

	// Inserting (key -> value) pair into table using 'EE_RECAST_U8' macro
	// #define EE_RECAST_U8(x)    ((u8*)(&(x))) - takes the address of passed variable and casts it address to u8*
	// The key and value would be copied so it's okay to modify them afterwards
	// The function returns an EE_TRUE value on success

	i32 set_res = ee_dict_set(&dict, EE_RECAST_U8(key), EE_RECAST_U8(val));

	// Practically it will fail only if Dict ran out of memory (either entire RAM or some pull if custom Allocator is used)
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

// Iterator example, shows how to create an iterator over the table, and access it's keys and values
void run_dict_iter_example(void)
{
	// Creating a hash-table with starting size of 128 values, 4-byte key and 4-byte value
	// Default heap Allocator, default ee_hash function and default key comparison function
	// In order to specify those arguments use either 'ee_dict_new' function directly 
	// or 'ee_dict_new_m' macro with extra parameters
	// using 'ee_dict_def_m' macro

	Dict dict = ee_dict_def_m(128, u32, f32);

	// Same as:
	// Dict dict = ee_dict_new(128, sizeof(u32), sizeof(f32), alignof(u32), alignof(f32), NULL, NULL, NULL);

	// Inserting 'pairs_count' random values wuth random keys
	i32 pairs_count = 8;
	
	for (i32 i = 0; i < pairs_count; ++i)
	{
		u32 key = (u32)i;
		f32 val = (f32)i * i;

		EE_PRINTLN("[%d]: (%u, %.1f) inserted", i, key, val);

		// EE_RECAST_U8 is a helper macro to cast any type pointer to u8* for ee_dict API
		i32 set_res = ee_dict_set(&dict, EE_RECAST_U8(key), EE_RECAST_U8(val));

		// Practically it will fail only if Dict ran out of memory (either entire RAM or some pull if custom Allocator is used)
		EE_ASSERT(set_res == EE_TRUE, "Failed to insert (%f) into hash table", val);
	}

	EE_PRINTLN();

	// Create iterator over dict
	DictIter iter = ee_dict_iter_new(&dict);

	u32 current_key = 0;
	f32 current_val = 0.0f;
	i32 count = 0;

	// 'ee_dict_iter_next' returns EE_FALSE when next (key, value) pair can not be obtained
	while (ee_dict_iter_next(&iter, EE_RECAST_U8(current_key), EE_RECAST_U8(current_val)))
	{
		EE_PRINTLN("(%u, %.1f) obtained via iterator", current_key, current_val);
		count++;

		// Do something with key and value
		// Note that the values are not obtained in the order in which they were inserted
	}

	EE_ASSERT(count == pairs_count, "Invalid iteration result");
	EE_PRINTLN("\nFirst loop over dict completed successfully\n");

	// Reset the iterator to repeat
	ee_dict_iter_reset(&iter);

	u32* key_ptr = NULL;
	f32* val_ptr = NULL;

	// Iterating again
	while (ee_dict_iter_next_ptr(&iter, EE_RECAST_U8(key_ptr), EE_RECAST_U8(val_ptr)))
	{
		EE_ASSERT(key_ptr != NULL && val_ptr != NULL, "Invalid iterator result");
		EE_PRINTLN("(%u, %.1f) obtained via pointer iterator", *key_ptr, *val_ptr);
		count++;

		// Do something with key and value pointers
	}

	EE_ASSERT(count == 2 * pairs_count, "Invalid iteration result");
	EE_PRINTLN("\nSecond loop over dict completed successfully");

	// Clear the table
	ee_dict_free(&dict);
}

//
// Custom functions example, shows how to implement and specify custom functions for key comparison, hashing and custom memory Allocator
//

i32 key_eq_fn(const u8* a_ptr, const u8* b_ptr, size_t len)
{
	// In this example length of the keys are known
	EE_UNUSED(len);

	Key* a = (Key*)a_ptr;
	Key* b = (Key*)b_ptr;

	// Comparison function only needs to known wheter the values are equal on not
	// unlike 'memcmp' which retuns -1, 1 and 0
	return (a->high == b->high) && (a->low == b->low);
}

u64 key_hash_fn(const u8* key_ptr, size_t len)
{
	// In this example length of the key are known
	EE_UNUSED(len);

	Key* key = (Key*)key_ptr;

	// Output hash
	u64 hash = key->low;

	// Basic mixer
	hash ^= key->high + 0x9e3779b97f4a7c15ull + (hash << 6) + (hash >> 2);

	return hash;
}

void* custom_alloc_fn(Allocator* self, size_t size)
{
	// Performing some important work
	EE_PRINTLN("Basic custom alloc function called, allocated size (%zu) bytes", size);

	return ee_default_alloc(self, size);
}

void run_dict_custom_fn_example(void)
{
	// Creating a hash-table with starting size of 128 values, 4-byte key and 4-byte value
	// Custom heap Allocator with very important work, custom mixer hash function and custom key comparison function

	Allocator allocator = { 0 };

	// You should specify how to allocate, reallocate and free your memory
	// If you want left some behaviour as default use ee_default_* functions
	// Also you can pass user context

	allocator.alloc_fn = custom_alloc_fn;
	allocator.realloc_fn = ee_default_realloc;
	allocator.free_fn = ee_default_free;
	allocator.context = NULL;

	Dict dict = ee_dict_new_m(128, Key, f32, &allocator, key_hash_fn, key_eq_fn);

	// Same as:
	// Dict dict = ee_dict_new(128, sizeof(u32), sizeof(f32), alignof(u32), alignof(f32), &allocator, key_hash_fn, key_eq_fn);

	// Inserting 1048576 values, MB is the constant (1 << 20) for bytes 
	// but i also like to use it such cases
	u32 pairs_count = EE_MB;

	for (u32 i = 0; i < pairs_count; ++i)
	{
		Key key = { 0 };
		f32 val = (f32)i;

		// Generating very simple key
		// 'ee_random' have convinient 'ee_rand_u64' function for such cases
		key.high = (u64)i;
		key.low  = (u64)i;

		// Insert random key and value
		i32 set_res = ee_dict_set(&dict, EE_RECAST_U8(key), EE_RECAST_U8(val));

		// Practically it will fail only if Dict ran out of memory (either entire RAM or some pull if custom Allocator is used)
		EE_ASSERT(set_res == EE_TRUE, "Failed to insert (%f) into hash table", val);

		// Get same value that just was inserted
		u8* val_ptr = ee_dict_at(&dict, EE_RECAST_U8(key));

		// Never fails
		EE_ASSERT(val_ptr != NULL && val == *(f32*)val_ptr, "Invalid insertion");
	}

	EE_ASSERT(ee_dict_count(&dict) == pairs_count, "Invalid insertation");
	EE_PRINTLN("\nTotal inserted: (%zu)", ee_dict_count(&dict));

	ee_dict_free(&dict);
}

#endif // EE_DICT_EXAMPLE_H
