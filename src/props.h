/*============================================================================
  boilerplate 
  propos.h 
  Copyright (c)2020 Kevin Boone, GPL v3.0
============================================================================*/

#pragma once

#include "defs.h"
#include "path.h"

struct _Props;
typedef struct _Props Props;

BEGIN_DECLS

Props      *props_create (void);
void        props_destroy (Props *self);
void        props_put (Props *self, const char *name, const char *value);
const char *props_get (const Props *self, const char *key);
BOOL        props_read_from_file (Props *self, const char *filename);
void        props_delete (Props *self, const char *name);
void        props_put_boolean (Props *self, const char *key, BOOL value);
BOOL        props_get_boolean (const Props *self, const char *key, BOOL deflt);
void        props_put_integer (Props *self, const char *key, int value);
void        props_put_int64 (Props *self, const char *key, int64_t value);
int         props_get_integer (const Props *self, const char *key, int deflt);
int64_t     props_get_int64 (const Props *self, const char *key, 
              int64_t deflt);
void        props_dump (const Props *self);
BOOL        props_read_from_path (Props *self, const Path *path);

END_DECLS

