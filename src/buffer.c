/*============================================================================

  boilerplate 
  buffer.c
  Copyright (c)2017 Kevin Boone, GPL v3.0

  Methods for storing and retrieving blocks of data. These methods are
    just thin wrappers around malloc() and free(), but the object stores
    the size of the data, which is often convenient when handling
    data blocks of variable size. The data is considered to be a block
    of BYTE, but the actual data is irrelevant to these methods.

============================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include "string.h" 
#include "defs.h" 
#include "log.h" 
#include "buffer.h" 

struct _Buffer
  {
  BYTE *data;
  uint64_t length;
  }; 


/*==========================================================================
  buffer_create
  Create a buffer from a copy of the data provided. The caller can, and
    probably should, free the data. This method is safe to call on
    static data, should the need arise.
*==========================================================================*/
Buffer *buffer_create (BYTE *data, uint64_t length)
  {
  LOG_IN
  Buffer *self = buffer_create_empty();
  buffer_set_contents (self, data, length);
  LOG_OUT 
  return self;
  }

/*==========================================================================
  buffer_create_without_copy
  Create a buffer and own the data supplied. The caller _must_ not free
    this data. This method cannot be called on static data, because
    it will try to free it later
*==========================================================================*/
Buffer *buffer_create_without_copy (BYTE *data, uint64_t length)
  {
  LOG_IN
  Buffer *self = buffer_create_empty();
  self->data = data; 
  self->length = length; 
  LOG_OUT 
  return self;
  }

/*==========================================================================
  buffer_create_empty 
*==========================================================================*/
Buffer *buffer_create_empty (void)
  {
  LOG_IN
  Buffer *self = malloc (sizeof (Buffer));
  self->data = NULL;
  self->length = 0;
  LOG_OUT
  return self;
  }


/*==========================================================================
  buffer_set_contents
  COPY and set the contents of the buffer, freeing any previous contents.
  The caller can (and probably should) free its own copy of the data,
    if it was dynamically created. It is safe to call this function on
    static data, although there are probably few good reasons to.
*==========================================================================*/
void buffer_set_contents (Buffer *self, BYTE *data, uint64_t length)
  {
  LOG_IN
  if (self->data) free (self->data);
  self->data = malloc (length);
  memcpy (self->data, data, length);
  self->length = length;
  LOG_OUT 
  }


/*==========================================================================
  buffer_destroy
*==========================================================================*/
void buffer_destroy (Buffer *self)
  {
  LOG_IN
  if (self)
    {
    if (self->data) free (self->data);
    free (self);
    }
  LOG_OUT
  }


/*==========================================================================
  buffer_get_length
*==========================================================================*/
uint64_t buffer_get_length (const Buffer *self)
  {
  return self->length;
  }


/*==========================================================================
  buffer_get_contents
*==========================================================================*/
const BYTE *buffer_get_contents (const Buffer *self)
  {
  return self->data;
  }


/*==========================================================================
  buffer_null_terminate
  Write four 0's on the end of the buffer, to make life easier for callers
   that know the data is of the kind that can be manipulated as
   null-terminated, but where there is no guarantee it actually is. Note
   that the length of the string is not affected -- these nulls do not
   count towards the length.

  This whole method is rather ugly, and should only be used for debugging
    purposes.
*==========================================================================*/
void buffer_null_terminate (Buffer *self)
  {
  LOG_IN
  self->data = realloc (self->data , self->length + 4);
  for (int i = 0; i < 4; i++)
    self->data [self->length + i] = 0;
  LOG_OUT
  }

