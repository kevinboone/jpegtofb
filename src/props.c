/*============================================================================

  boilerplate
  props.c
  Copyright (c)2020 Kevin Boone, GPL v3.0

  Methods for handling a set (actually a list, but there is logic to keep
    the items unique) of name-value pairs, that can be read in from a file. 
  These functions wrap the complexity involved in using the List and 
    NameValuePair classes for this purpose

============================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include <string.h>
#include "defs.h" 
#include "list.h" 
#include "string.h" 
#include "nvp.h" 
#include "props.h" 
#include "log.h" 
#include "file.h" 
#include "path.h" 
#include "numberformat.h" 

struct _Props
  {
  List *list;
  }; 


/*==========================================================================
  props_get_boolean
*==========================================================================*/
BOOL props_get_boolean (const Props *self, const char *key, BOOL deflt)
  {
  LOG_IN
  BOOL ret = deflt;
  const char *s = props_get (self, key);
  if (s)
    {
    ret = atoi (s);
    }
  LOG_IN
  return ret;
  }


/*==========================================================================
  props_get_int64
*==========================================================================*/
int64_t props_get_int64 (const Props *self, const char *key, 
              int64_t deflt)
  {
  LOG_IN
  uint64_t ret = deflt;
  const char *s = props_get (self, key);
  if (s)
    {
    if (!numberformat_read_integer (s, &ret, FALSE))
      {
      ret = deflt;
      }
    }
  LOG_IN
  return ret;
  }


/*==========================================================================
  props_get_integer
*==========================================================================*/
int props_get_integer (const Props *self, const char *key, int deflt)
  {
  LOG_IN
  int ret = deflt;
  const char *s = props_get (self, key);
  if (s)
    {
    ret = atoi (s);
    }
  LOG_IN
  return ret;
  }


/*==========================================================================
  props_get
*==========================================================================*/
const char *props_get (const Props *self, const char *key)
  {
  LOG_IN

  log_debug ("props_get, key=%s", key);
  
  BOOL found = FALSE;
  String *result = NULL;
  int l = list_length (self->list);
  for (int i = 0; i < l && !found; i++)
    {
    const NameValuePair *nvp = list_get (self->list, i);
    const char *name = nvp_get_name (nvp);
    if (strcmp (name, key) == 0)
      {
      result = (String *)nvp_get_value (nvp);
      log_debug ("Found key %s, value=%s", key, string_cstr (result));
      found = TRUE;
      }
    }

  LOG_OUT
  if (found)
    return string_cstr (result);
  else 
    return NULL;
  }


/*==========================================================================
  props_delete
*==========================================================================*/
void props_delete (Props *self, const char *name)
  {
  LOG_IN

  log_debug ("props_delete, key=%s", name);
  
  BOOL found = FALSE;
  int l = list_length (self->list);
  for (int i = 0; i < l && !found; i++)
    {
    const NameValuePair *nvp = list_get (self->list, i);
    if (nvp)
      {
      log_debug ("props_delete, found NVP, deleting", name);
      list_remove_object (self->list, nvp);
      found = TRUE;
      }
    }

  LOG_OUT
  }


/*==========================================================================
  props_put
*==========================================================================*/
void props_put (Props *self, const char *name, const char *value)
  {
  LOG_IN
  
  log_debug ("props_put, name=%s, value=%s", name, value);

  if (props_get (self, name))
    {
    props_delete (self, name);
    }

  NameValuePair *nvp = nvp_create (name, 
    string_create (value), (ValueFreeFn)string_destroy);
  list_append (self->list, nvp);

  LOG_OUT
  }


/*==========================================================================
  props_create
*==========================================================================*/
Props *props_create (void)
  {
  LOG_IN

  Props *self = malloc (sizeof (Props));

  List *list = list_create ((ListItemFreeFn)nvp_destroy);
  self->list = list;

  return self;
  
  LOG_OUT
  }


/*==========================================================================
  props_destroy
*==========================================================================*/
void props_destroy (Props *self)
  {
  LOG_IN
  if (self)
    {
    if (self->list)
      {
      list_destroy (self->list);
      }
    free (self);
    }

  LOG_OUT
  }


/*==========================================================================
  props_put_boolean
*==========================================================================*/
void props_put_boolean (Props *self, const char *name, BOOL value)
  {
  LOG_IN
  log_debug ("props_put_boolean: key=%s, value=%d", name, value);
  if (value)
    props_put (self, name, "1");
  else
    props_put (self, name, "0");
  LOG_OUT
  }


/*==========================================================================
  props_put_integer
*==========================================================================*/
void props_put_integer (Props *self, const char *name, int value)
  {
  LOG_IN
  log_debug ("props_put_integer: key=%s, value=%d", name, value);
  char s[20];
  snprintf (s, sizeof (s), "%d", value);
  props_put (self, name, s);
  LOG_OUT
  }


/*==========================================================================
  props_put_int64
*==========================================================================*/
void props_put_int64 (Props *self, const char *name, int64_t value)
  {
  LOG_IN
  log_debug ("props_put_integer: key=%s, value=%ld", name, value);
  char s[30];
  snprintf (s, sizeof (s), "%ld", value);
  props_put (self, name, s);
  LOG_OUT
  }


/*==========================================================================
  props_read_from_path
*==========================================================================*/
BOOL props_read_from_path (Props *self, const Path *path)
  {
  LOG_IN
  char *s = (char *)path_to_utf8 (path);
  BOOL ret = props_read_from_file (self, s);
  free (s);
  LOG_OUT
  return ret;
  }


/*==========================================================================
  props_read_from_file
*==========================================================================*/
BOOL props_read_from_file (Props *self, const char *filename)
  {
  LOG_IN
  log_debug ("props_read_from_file, file=%s", filename);
  FILE *f = fopen (filename, "r");
  if (f)
    {
    char *buff = NULL;
    BOOL cont;
    do
      {
      cont = (file_readline (f, &buff) > 0);
      if (cont)
        {
        String *line = string_create (buff); 
        free (buff);

        string_trim_left (line);
        string_trim_right (line);

        log_debug ("line='%s'\n", string_cstr(line));

	char *s_line = strdup (string_cstr (line));
        if (s_line[0] != '#')
	  {
          char *eq = strchr (s_line, '=');
	  if (eq)
	    {
	    const char *key = s_line;
	    const char *value = eq + 1;
	    *eq = 0;
            log_debug ("key=%s, value=%s", key, value);
	    props_put (self, key, value);
	    }
	  }
        free (s_line); 
        string_destroy (line);
        }
      } while (cont);
    fclose (f);
    LOG_OUT
    return TRUE;
    }
  else
    {
    log_debug ("Could not open file for reading");
    LOG_OUT
    return FALSE;
    }
  }

 
/*==========================================================================
  props_read_from_file
*==========================================================================*/
void props_dump (const Props *self)
  {
  int l = list_length (self->list);
  for (int i = 0; i < l; i++)
    {
    const NameValuePair *nvp = list_get (self->list, i); 
    const char *name = nvp_get_name (nvp);
    const String *value = nvp_get_value (nvp);
    printf ("%d '%s' '%s'\n", i, name, string_cstr (value));
    }
  }


