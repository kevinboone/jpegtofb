/*============================================================================
  boilerplate 
  nvp.h 
  Copyright (c)2020 Kevin Boone, GPL v3.0
============================================================================*/

#pragma once

#include <stdint.h>
#include "defs.h"

struct _NameValuePair;
typedef struct _NameValuePair NameValuePair;

typedef void (*ValueFreeFn) (void *);

BEGIN_DECLS

void nvp_destroy (NameValuePair *self);
NameValuePair *nvp_create (const char *name, 
     void *value, ValueFreeFn valueFreeFn);
const char *nvp_get_name (const NameValuePair *self);
const void *nvp_get_value (const NameValuePair *self);

END_DECLS
