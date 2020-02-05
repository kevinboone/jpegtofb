/*============================================================================
  boilerplate 
  path.h
  Copyright (c)2017 Kevin Boone, GPL v3.0
============================================================================*/

#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include "defs.h"
#include "file.h"
#include "buffer.h"
#include "string.h"

struct _Path;
typedef struct _Path Path;

BEGIN_DECLS

Path        *path_create_empty (void);
Path        *path_create (const char *s);
Path        *path_create_home (void);
const UTF32 *path_cstr (const Path *self);
void         path_destroy (Path *self);
int          path_length (const Path *self);
void         path_append (Path *self, const char *name);
UTF8        *path_to_utf8 (const Path *self);
BOOL         path_expand_directory (const Path *path, 
                    int flags, List **names);
BOOL         path_ends_with_separator (const Path *self);
BOOL         path_ends_with_fwd_slash (const Path *self);
BOOL         path_create_directory (const Path *self);
Path        *path_clone (const Path *p);
void         path_remove_filename (Path *self);
void         path_remove_directory (Path *self);
UTF8        *path_get_filename_utf8 (const Path *path);
BOOL         path_read_to_buffer (const Path *path, Buffer **buffer);
FILE        *path_fopen (const Path *self, const char *mode);
BOOL         path_write_from_buffer (const Path *self, const Buffer *buffer);
BOOL         path_write_from_string (const Path *self, const String *s);
BOOL         path_is_directory (const Path *self);
BOOL         path_is_regular (const Path *self);
BOOL         path_stat (const Path *self, struct stat *sb);

END_DECLS



