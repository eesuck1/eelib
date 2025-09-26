#ifndef EE_FS_H
#define EE_FS_H

#include "windows.h"
#include "ee_array.h"
#include "ee_string.h"

#define EE_MAX_PATH_LEN    (MAX_PATH)

enum
{
	EE_PATH_DIR  = 1 << 0,
	EE_PATH_FILE = 1 << 1,
};

typedef struct FsReader
{
	Str slab;
	Array offsets;
	Allocator allocator;
} FsReader;

EE_EXTERN_C_START

EE_INLINE FsReader ee_fs_new(size_t size, Allocator* allocator)
{
	FsReader out = { 0 };

	if (allocator == NULL)
	{
		out.allocator.alloc_fn = ee_default_alloc;
		out.allocator.realloc_fn = ee_default_realloc;
		out.allocator.free_fn = ee_default_free;
		out.allocator.context = NULL;
	}
	else
	{
		memcpy(&out.allocator, allocator, sizeof(Allocator));
	}

	out.slab = ee_str_new(size * EE_MAX_PATH_LEN, allocator);
	out.offsets = ee_array_new(size, sizeof(size_t), allocator);

	return out;
}

EE_INLINE s32 ee_is_sep(char val)
{
	return val == '\\' || val == '/';
}

EE_INLINE s32 ee_fs_wildcard(const u8* str, const u8* pattern)
{
	EE_ASSERT(str != NULL, "Trying to match NULL path");
	EE_ASSERT(pattern != NULL, "Trying to match NULL pattern");

	if (pattern[0] == '*' && pattern[1] == '\0')
	{
		return EE_TRUE;
	}

	u8* last_s = NULL;
	u8* last_p = NULL;

	while (*str != '\0')
	{
		char s = *str;
		char p = *pattern;

		if (p == '*')
		{
			last_s = str;
			last_p = ++pattern;

			if (*pattern == '\0')
			{
				return EE_TRUE;
			}

			continue;
		}

		if (p != s && p != '?')
		{
			if (last_p == NULL)
			{
				return EE_FALSE;
			}

			str = ++last_s;
			pattern = last_p;

			continue;
		}

		str++;
		pattern++;
	}
	
	while (*pattern == '*')
	{
		pattern++;
	}

	return *pattern == '\0';
}

EE_INLINE s32 ee_fs_isdir(const u8* dir_path)
{
	EE_ASSERT(dir_path != NULL, "Trying to dereference NULL path");

	return GetFileAttributesA(dir_path) & FILE_ATTRIBUTE_DIRECTORY;
}

EE_INLINE s32 ee_fs_path_exists(const u8* path)
{
	EE_ASSERT(path != NULL, "Trying to dereference NULL path");

	return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

EE_INLINE void ee_fs_format_dir_path(u8* out_path, const u8* dir_path, const u8* mask)
{
	EE_ASSERT(dir_path != NULL, "Trying to dereference NULL path");
	EE_ASSERT(out_path != NULL, "Trying to dereference NULL path");

	const u8* mask_val = mask == NULL ? "" : mask;
	size_t len = strnlen(dir_path, EE_MAX_PATH_LEN);

	if (dir_path[len] != '/' && dir_path[len] != '\\')
	{
		snprintf(out_path, EE_MAX_PATH_LEN, "%s\\%s", dir_path, mask_val);
	}
	else
	{
		snprintf(out_path, EE_MAX_PATH_LEN, "%s%s", dir_path, mask);
	}
}

EE_INLINE void ee_fs_reset(FsReader* fs)
{
	EE_ASSERT(fs != NULL, "Trying to clear NULL fs");

	ee_array_reset(&fs->offsets);
	ee_str_reset(&fs->slab);
}

EE_INLINE void ee_fs_listdir_ex(FsReader* fs, const u8* dir_path, const u8* mask, s32 max_depth, s32 reset_fs)
{
	EE_ASSERT(fs != NULL, "Trying to dereference NULL file reader");
	EE_ASSERT(ee_fs_path_exists(dir_path), "Directory path does not exist (%s)", dir_path);
	EE_ASSERT(ee_fs_isdir(dir_path), "Path is not a directory (%s)", dir_path);

	if (max_depth <= 0 || GetFileAttributesA(dir_path) == INVALID_FILE_ATTRIBUTES ||
		!(GetFileAttributesA(dir_path) & FILE_ATTRIBUTE_DIRECTORY))
	{
		return;
	}

	if (reset_fs)
	{
		ee_fs_reset(fs);
	}

	u8 orig_path[EE_MAX_PATH_LEN] = { 0 };
	u8 base_path[EE_MAX_PATH_LEN] = { 0 };
	u8 full_path[EE_MAX_PATH_LEN] = { 0 };

	WIN32_FIND_DATAA finder = { 0 };
	HANDLE handle = INVALID_HANDLE_VALUE;

	ee_fs_format_dir_path(orig_path, dir_path, "*");
	ee_fs_format_dir_path(base_path, dir_path, NULL);

	size_t base_len = strnlen(base_path, EE_MAX_PATH_LEN);
	handle = FindFirstFileExA(orig_path, FindExInfoBasic, &finder, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);

	EE_ASSERT(handle != INVALID_HANDLE_VALUE, "Unable to list directory (%s)", orig_path);
	
	if (handle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	do
	{
		const u8* name = finder.cFileName;

		if ((name[0] == '.') && (name[1] == '\0' || (name[1] == '.' && name[2] == '\0')))
		{
			continue;
		}

		size_t file_len = strnlen(name, EE_MAX_PATH_LEN);

		memcpy(full_path, base_path, base_len);
		memcpy(&full_path[base_len], name, file_len);
		
		EE_ASSERT(base_len + file_len < EE_MAX_PATH_LEN, "Invalid path length (%zu)", base_len + file_len);
		
		full_path[base_len + file_len] = '\0';

		if (finder.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			ee_fs_listdir_ex(fs, full_path, mask, max_depth - 1, EE_FALSE);
		}
		else
		{
			if (!ee_fs_wildcard(full_path, mask))
			{
				continue;
			}

			size_t offset = fs->slab.top;

			ee_str_push_bytes(&fs->slab, full_path, base_len + file_len + 1);
			ee_array_push(&fs->offsets, EE_ARRAY_DT(offset));
		}
	} while (FindNextFileA(handle, &finder));

	FindClose(handle);
}

EE_INLINE size_t ee_fs_len(FsReader* fs)
{
	EE_ASSERT(fs != NULL, "Trying to dereference NULL reader");

	return ee_array_len(&fs->offsets);
}

EE_INLINE const u8* ee_fs_cstr_at(FsReader* fs, size_t i)
{
	EE_ASSERT(fs != NULL, "Trying to dereference NULL reader");
	EE_ASSERT(i < ee_array_len(&fs->offsets), "Invalid offset index (%zu) for array with length (%zu)", i, ee_array_len(&fs->offsets));

	size_t offset = EE_ARRAY_GET(&fs->offsets, i, size_t);

	return ee_str_at(&fs->slab, offset);
}

EE_EXTERN_C_END

#endif // EE_FS_H
