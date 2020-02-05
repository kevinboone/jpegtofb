/*==========================================================================
boilerplate
file.h
Copyright (c)2020 Kevin Boone
Distributed under the terms of the GPL v3.0
==========================================================================*/

#pragma once

#include <stdint.h>
#include "list.h" 
#include "buffer.h" 
#include "string.h" 

// File search constants, for file_expand_directory() and
//   path_expand_directory()

// Default file search is files and directories, no prepended path
#define FE_DEFAULT            (FE_FILES | FE_DIRS) 

// Prepend relative or absolute path to all names
#define FE_PREPEND_PATH       0x0001
#define FE_PREPEND_FULL_PATH  0x0002

// Include hidden files in listing
#define FE_HIDDEN             0x0004

// Types of file to include. FE_ALL includes sockets, pipes, etc
#define FE_FILES              0x0010
#define FE_DIRS               0x0020
#define FE_ALL                0x0040

BEGIN_DECLS

int     file_readline (FILE *f, char **buffer);
int64_t file_get_size (const char *filename);
time_t  file_get_mtime (const char *filename);
BOOL    file_exists (const char *filename);
BOOL    file_is_regular (const char *filename);
BOOL    file_is_directory (const char *filename);
BOOL    file_expand_directory (const char *path, int flags, 
          List **names);
BOOL    file_read_to_buffer (const char *filename, Buffer **buffer);
BOOL    file_write_from_buffer (const char *filename, const Buffer *buffer);
BOOL    file_write_from_string (const char *filename, const String *string);
BOOL    file_name_matches_pattern (const char *name, const char *pattern);
BOOL    file_name_matches_pattern_case (const char *name, 
            const char *pattern, BOOL insensitive);
char   *file_glob_to_regex (const char *glob);

END_DECLS
