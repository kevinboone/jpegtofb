/*============================================================================
 
  boilerplate
  nvp.c
  Copyright (c)2020 Kevin Boone, GPL v3.0

  Methds for handling name-value pairs. A Props object is essentially a 
  list of NVPs. Each NVP has a key, which is an ordinary C string, and
  a value, which can be anything. Because the value is "owned" by the 
  NVP, and should therefore be destroyed when the NVP is destroyed, 
  an NVP is associated not only with a name and a value, but with a 
  value destructor. If the value is a plain string, or any unstructured
  block of data allocated using malloc(), then free() is a valid destructor.
  Structured objects have their own individual destructors (usually
  xxx_destroy) that can be used.   

============================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include "defs.h" 
#include "nvp.h" 
#include "log.h" 

struct _NameValuePair
  {
  char *name;
  void *value;
  ValueFreeFn valueFreeFn;
  }; 


/*==========================================================================
  nvp_create
*==========================================================================*/
NameValuePair *nvp_create (const char *name, 
     void *value, ValueFreeFn valueFreeFn)
  {
  LOG_IN

  log_debug ("name=%s, value=%08x", name, value);

  NameValuePair *self = malloc (sizeof (NameValuePair));

  self->name = strdup (name);
  self->value = value;
  self->valueFreeFn = valueFreeFn;

  return self;
  
  LOG_OUT
  }


/*==========================================================================
  nvp_destroy
*==========================================================================*/
void nvp_destroy (NameValuePair *self)
  {
  LOG_IN
  
  if (self)
    {
    if (self->name) free (self->name);
    if (self->value) self->valueFreeFn (self->value);
    free (self);
    }

  LOG_OUT
  }


/*==========================================================================
  nvp_get_name
*==========================================================================*/
const char *nvp_get_name (const NameValuePair *self)
  {
  return self->name;
  }


/*==========================================================================
  nvp_get_value
*==========================================================================*/
const void *nvp_get_value (const NameValuePair *self)
  {
  return self->value;
  }





