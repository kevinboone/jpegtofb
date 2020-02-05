/*============================================================================

  boilerplate 
  buffer.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include <stdint.h>
#include "defs.h"

struct _Buffer;
typedef struct _Buffer Buffer;

BEGIN_DECLS

Buffer     *buffer_create_empty (void);
Buffer     *buffer_create (BYTE *data, uint64_t length);
Buffer     *buffer_create_without_copy (BYTE *data, uint64_t length);
void        buffer_set_contents (Buffer *self, BYTE *data, uint64_t length);
void        buffer_destroy (Buffer *self);
uint64_t    buffer_get_length (const Buffer *self);
const BYTE *buffer_get_contents (const Buffer *self);
void        buffer_null_terminate (Buffer *self);
END_DECLS

