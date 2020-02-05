/*============================================================================
  boilerplate 
  wstring.c
  Copyright (c)2017 Kevin Boone, GPL v3.0

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
#include "wstring.h" 
#include "string.h" 
#include "defs.h" 
#include "log.h" 
#include "convertutf.h" 

struct _WString
  {
  UTF32 *str;
  }; 


/*==========================================================================
wstring_create_empty 
*==========================================================================*/
WString *wstring_create_empty (void)
  {
  WString *self = malloc (sizeof (WString));
  self->str = malloc (1 * sizeof (UTF32));
  self->str[0] = 0;
  return self;
  }


/*==========================================================================
string_create
NOTE: by very careful about using this method with a string of the form
L"foo". There's no guarantee that the compiler will use 32-bit characters
for wide strings, and it might be locale-specific. Better to initialise
a string from UTF8/ASCII if practicable.
*==========================================================================*/
WString *wstring_create (const UTF32 *s)
  {
  WString *self = malloc (sizeof (WString));
  int l = wstring_length_utf32 (s);
  self->str = malloc (sizeof (UTF32) * (l + 1));
  for (int i = 0; i <= l; i++)
    self->str[i] = s[i];
  return self;
  }


/*==========================================================================
wstring_cstr_safe
*==========================================================================*/
const UTF32 *wstring_cstr_safe (const WString *self)
  {
  static UTF32 empty[] = { 0 };
  if (self)
    {
    if (self->str) 
      return self->str;
    else
      return empty;
    }
  else
    return empty;
  }



/*==========================================================================
wstring_destroy
*==========================================================================*/
void wstring_destroy (WString *self)
  {
  if (self)
    {
    if (self->str) free (self->str);
    free (self);
    }
  }


/*==========================================================================
wstring_cstr
*==========================================================================*/
const UTF32 *wstring_cstr (const WString *self)
  {
  return self->str;
  }


/*==========================================================================
wstring_length
*==========================================================================*/
int wstring_length (const WString *self)
  {
  if (self == NULL) return 0;
  if (self->str == NULL) return 0;
  UTF32 *s = self->str;
  int i = 0;
  UTF32 c = 0;
  do
    {
    c = s[i];
    i++;
    } while (c != 0);
  int ret = i - 1;
  return ret;
  }


/*==========================================================================
wstring_length_utf32
*==========================================================================*/
int wstring_length_utf32 (const UTF32 *s)
  {
  int i = 0;
  UTF32 c = 0;
  do
    {
    c = s[i];
    i++;
    } while (c != 0);
  int ret = i - 1;
  return ret;
  }

/*==========================================================================
wstring_create_from_utf8
*==========================================================================*/
WString *wstring_create_from_utf8 (const UTF8 *s)
  {
  WString *self = malloc (sizeof (WString));
  UTF32 *ss = string_utf8_to_utf32 (s);
  self->str = ss;
  return self;
  }

/*==========================================================================
wstring_append
*==========================================================================*/
void wstring_append (WString *self, const UTF32 *s) 
  {
  int orig_len = wstring_length (self);
  int append_len = wstring_length_utf32 (s);
  int total_len = orig_len + append_len;
  self->str = realloc (self->str, (total_len + 1) * sizeof (UTF32));
  memcpy (self->str + orig_len, s, (append_len + 1) * sizeof (UTF32));
  }


/*==========================================================================
wstring_prepend
*==========================================================================*/
void wstring_prepend (WString *self, const UTF32 *s) 
  {
  int orig_len = wstring_length (self);
  int prepend_len = wstring_length_utf32 (s);
  int total_len = orig_len + prepend_len;
  self->str = realloc (self->str, (total_len + 1) * sizeof (UTF32));
  memmove (self->str + prepend_len, self->str, (orig_len + 1) * sizeof (UTF32));
  memcpy (self->str, s, prepend_len * sizeof (UTF32));
  }


/*==========================================================================
wstring_append_printf
*==========================================================================*/
void wstring_append_printf (WString *self, const char *fmt,...) 
  {
  va_list ap;
  va_start (ap, fmt);
  char *s;
  vasprintf (&s, fmt, ap);
  UTF32 *ss = string_utf8_to_utf32 ((UTF8 *)s);
  wstring_append (self, ss);
  free (ss);
  free (s);
  va_end (ap);
  }

/*==========================================================================
wstring_clone
*==========================================================================*/
WString *wstring_clone (const WString *self)
  {
  return wstring_create (wstring_cstr (self));
  }


/*==========================================================================
wstring_find
*==========================================================================*/
int wstring_find (const WString *self, const UTF32 *search)
  {
  int lsearch = wstring_length_utf32 (search);
  int lself = wstring_length_utf32 (self->str);
  if (lsearch > lself) return -1; // Can't find a long string in short one
  for (int i = 0; i < lself - lsearch + 1; i++)
    {
    BOOL diff = FALSE;
    for (int j = 0; j < lsearch && !diff; j++)
      {
      if (search[j] != self->str[i + j]) diff = TRUE;
      }
    if (!diff) return i;
    }
  return -1;
  }


/*==========================================================================
wstring_find_last
*==========================================================================*/
int wstring_find_last (const WString *self, const UTF32 *search)
  {
  int lsearch = wstring_length_utf32 (search);
  int lself = wstring_length_utf32 (self->str);
  if (lsearch > lself) return -1; // Can't find a long string in short one
  for (int i = lself - lsearch; i >= 0; i--)
    {
    BOOL diff = FALSE;
    for (int j = 0; j < lsearch && !diff; j++)
      {
      if (search[j] != self->str[i + j]) diff = TRUE;
      }
    if (!diff) return i;
    }
  return -1;
  }


/*==========================================================================
wstring_delete
*==========================================================================*/
void wstring_delete (WString *self, int pos, int len)
  {
  UTF32 *str = self->str;
  int lself = wstring_length_utf32 (str);
  if (pos + len > lself)
    wstring_delete (self, pos, lself - len);
  else
    {
    UTF32 *buff = malloc ((lself - len + 2) * sizeof (UTF32));
    memcpy (buff, str, pos  * sizeof (UTF32)); 
    memcpy (buff + pos, str + pos + len, 
      (1 + wstring_length_utf32 (str + pos + len)) * sizeof (UTF32)); 
    free (self->str);
    self->str = buff;
    }
  }


/*==========================================================================
wstring_insert
*==========================================================================*/
void wstring_insert (WString *self, int pos, 
    const UTF32 *replace)
  {
  int lself = wstring_length (self);
  int lrep = wstring_length_utf32 (replace);
  UTF32 *buff = malloc ((lself + lrep + 1) * sizeof (UTF32));
  UTF32 *str = self->str;
  memcpy (buff, str, pos * sizeof (UTF32));
  buff[pos] = 0;
  memcpy (buff + pos, replace, lrep * sizeof (UTF32));
  memcpy (buff + pos + lrep, str + pos, (lself - pos + 1) * sizeof (UTF32)); 
  free (self->str);
  self->str = buff;
  }


/*==========================================================================
wstring_substitute_all
*==========================================================================*/
WString *wstring_substitute_all (const WString *self, 
    const UTF32 *search, const UTF32 *replace)
  {
  const UTF32 gibberish[] = { 1, 99, 2, 4432, 0 };
  WString *working = wstring_clone (self);
  BOOL cont = TRUE;
  int lsearch = wstring_length_utf32 (search);
  while (cont)
    {
    int i = wstring_find (working, search);
    if (i >= 0)
      {
      wstring_delete (working, i, lsearch);
      wstring_insert (working, i, gibberish);
      }
    else
      cont = FALSE;
    }
  cont = TRUE;
  while (cont)
    {
    int i = wstring_find (working, gibberish);
    if (i >= 0)
      {
      wstring_delete (working, i, 4);
      wstring_insert (working, i, replace);
      }
    else
      cont = FALSE;
    }
  return working;
  }

/*==========================================================================
  wstring_append_c
*==========================================================================*/
void wstring_append_c (WString *self, const uint32_t ch)
  {
  UTF32 s[2];
  s[0] = ch;
  s[1] = 0;
  wstring_append (self, s);
  }


/*==========================================================================
  wstring_ends_with
*==========================================================================*/
BOOL wstring_ends_with (const WString *self, const UTF32 *test)
  {
  BOOL ret = FALSE;
  int pos = wstring_find_last (self, test);
  if (pos >= 0)
    {
    int lself = wstring_length (self);
    int ltest = wstring_length_utf32 (test);
    if (pos == lself - ltest)
      ret = TRUE;
    }
  return ret;
  }


/*==========================================================================
  wstring_to_utf8
*==========================================================================*/
UTF8 *wstring_to_utf8 (const WString *self)
  {
  return string_utf32_to_utf8 (self->str); 
  }



