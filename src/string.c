/*============================================================================

  boilerplate 
  string.c
  Copyright (c)2017 Kevin Boone, GPL v3.0

  Methods for handling ASCII/UTF-8 strings. Be aware that these methods
  are just thin wrappers around standard, old-fashioned C library functions,
  and some will misbehave if the string actually contains multi-byte
  characters. In particular, the length() method returns the number of
  bytes, not the number of characters. Methods that search the string may
  potentially match the second or later byte of a multi-byte character.
  Any use of these methods for handling 'real' multibyte UTF-8 needs to
  be tested very carefully.

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
#include "convertutf.h" 
#include "wstring.h" 
#include "file.h" 
#include "path.h" 
#include "list.h" 

struct _String
  {
  char *str;
  }; 


/*==========================================================================
string_create_empty 
*==========================================================================*/
String *string_create_empty (void)
  {
  return string_create ("");
  }


/*==========================================================================
string_create
*==========================================================================*/
String *string_create (const char *s)
  {
  String *self = malloc (sizeof (String));
  self->str = strdup (s);
  return self;
  }


/*==========================================================================
string_destroy
*==========================================================================*/
void string_destroy (String *self)
  {
  if (self)
    {
    if (self->str) free (self->str);
    free (self);
    }
  }


/*==========================================================================
string_cstr
*==========================================================================*/
const char *string_cstr (const String *self)
  {
  return self->str;
  }


/*==========================================================================
string_cstr_safe
*==========================================================================*/
const char *string_cstr_safe (const String *self)
  {
  if (self)
    {
    if (self->str) 
      return self->str;
    else
      return "";
    }
  else
    return "";
  }


/*==========================================================================
string_append
*==========================================================================*/
void string_append (String *self, const char *s) 
  {
  if (!s) return;
  if (self->str == NULL) self->str = strdup ("");
  int newlen = strlen (self->str) + strlen (s) + 2;
  self->str = realloc (self->str, newlen);
  strcat (self->str, s);
  }


/*==========================================================================
string_prepend
*==========================================================================*/
void string_prepend (String *self, const char *s) 
  {
  if (!s) return;
  if (self->str == NULL) self->str = strdup ("");
  int newlen = strlen (self->str) + strlen (s) + 2;
  char *temp = strdup (self->str); 
  free (self->str);
  self->str = malloc (newlen);
  strcpy (self->str, s);
  strcat (self->str, temp);
  free (temp);
  }


/*==========================================================================
string_append_printf
*==========================================================================*/
void string_append_printf (String *self, const char *fmt,...) 
  {
  if (self->str == NULL) self->str = strdup ("");
  va_list ap;
  va_start (ap, fmt);
  char *s;
  vasprintf (&s, fmt, ap);
  string_append (self, s);
  free (s);
  va_end (ap);
  }


/*==========================================================================
string_length
*==========================================================================*/
int string_length (const String *self)
  {
  if (self == NULL) return 0;
  if (self->str == NULL) return 0;
  return strlen (self->str);
  }


/*==========================================================================
string_clone
*==========================================================================*/
String *string_clone (const String *self)
  {
  if (!self->str) return string_create_empty();
  return string_create (string_cstr (self));
  }


/*==========================================================================
string_find
*==========================================================================*/
int string_find (const String *self, const char *search)
  {
  const char *p = strstr (self->str, search);
  if (p)
    return p - self->str;
  else
    return -1;
  }


/*==========================================================================
string_find_last
*==========================================================================*/
int string_find_last (const String *self, const char *search)
  {
  int lsearch = strlen (search); 
  int lself = strlen (self->str);
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
string_delete
*==========================================================================*/
void string_delete (String *self, const int pos, const int len)
  {
  char *str = self->str;
  if (pos + len > strlen (str))
    string_delete (self, pos, strlen(str) - len);
  else
    {
    char *buff = malloc (strlen (str) - len + 2);
    strncpy (buff, str, pos); 
    strcpy (buff + pos, str + pos + len);
    free (self->str);
    self->str = buff;
    }
  }


/*==========================================================================
string_insert
*==========================================================================*/
void string_insert (String *self, const int pos, 
    const char *replace)
  {
  char *buff = malloc (strlen (self->str) + strlen (replace) + 2);
  char *str = self->str;
  strncpy (buff, str, pos);
  buff[pos] = 0;
  strcat (buff, replace);
  strcat (buff, str + pos); 
  free (self->str);
  self->str = buff;
  }

/*==========================================================================
string_substitute_all
*==========================================================================*/
String *string_substitute_all (const String *self, 
    const char *search, const char *replace)
  {
  const char *gibberish = "#@x!>Aa;";
  String *working = string_clone (self);
  BOOL cont = TRUE;
  while (cont)
    {
    int i = string_find (working, search);
    if (i >= 0)
      {
      string_delete (working, i, strlen (search));
      string_insert (working, i, gibberish);
      }
    else
      cont = FALSE;
    }
  cont = TRUE;
  while (cont)
    {
    int i = string_find (working, gibberish);
    if (i >= 0)
      {
      string_delete (working, i, strlen (gibberish));
      string_insert (working, i, replace);
      }
    else
      cont = FALSE;
    }
  return working;
  }


/*==========================================================================
  string_create_from_utf8_file 
*==========================================================================*/
BOOL string_create_from_utf8_file (const char *filename, 
    String **result, char **error)
  {
  String *self = NULL;
  BOOL ok = FALSE; 
  int f = open (filename, O_RDONLY);
  if (f > 0)
    {
    self = malloc (sizeof (String));
    struct stat sb;
    fstat (f, &sb);
    int64_t size = sb.st_size;
    char *buff = malloc (size + 2);
    read (f, buff, size);
    self->str = buff; 
    self->str[size] = 0;
    *result = self;
    ok = TRUE;
    }
  else
    {
    asprintf (error, "Can't open file '%s' for reading: %s", 
      filename, strerror (errno));
    ok = FALSE;
    }

  return ok;
  }


/*==========================================================================
  string_encode_url
*==========================================================================*/
static char to_hex(char code)
  {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
  }


String *string_encode_url (const char *str)
  {
  if (!str) return string_create_empty();;
  const char *pstr = str; 
  char *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr)
    {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_'
      || *pstr == '.' || *pstr == '~')
      *pbuf++ = *pstr;
    else if (*pstr == ' ')
      *pbuf++ = '+';
    else
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4),
         *pbuf++ = to_hex(*pstr & 15);
    pstr++;
    }
  *pbuf = '\0';
  String *result = string_create (buf);
  free (buf);
  return (result);
  }


/*==========================================================================
  string_append_byte
*==========================================================================*/
void string_append_byte (String *self, const BYTE byte)
  {
  char buff[2];
  buff[0] = byte;
  buff[1] = 0;
  string_append (self, buff);
  }


/*==========================================================================
  string_append_c
*==========================================================================*/
void string_append_c (String *self, const uint32_t ch)
  {
  if (ch < 0x80) 
    {
    string_append_byte (self, (BYTE)ch);
    }
  else if (ch < 0x0800) 
    {
    string_append_byte (self, (BYTE)((ch >> 6) | 0xC0));
    string_append_byte (self, (BYTE)((ch & 0x3F) | 0x80));
    }
  else if (ch < 0x10000) 
    {
    string_append_byte (self, (BYTE)((ch >> 12) | 0xE0));
    string_append_byte (self, (BYTE)((ch >> 6 & 0x3F) | 0x80));
    string_append_byte (self, (BYTE)((ch & 0x3F) | 0x80));
    }
  else 
    {
    string_append_byte (self, (BYTE)((ch >> 18) | 0xF0));
    string_append_byte (self, (BYTE)(((ch >> 12) & 0x3F) | 0x80));
    string_append_byte (self, (BYTE)(((ch >> 6) & 0x3F) | 0x80));
    string_append_byte (self, (BYTE)((ch & 0x3F) | 0x80));
    }
  }


/*==========================================================================
  string_trim_left
*==========================================================================*/
void string_trim_left (String *self)
  {
  const char *s = self->str;
  int l = strlen (s);
  int i = 0;
  int pos = 0;
  BOOL stop = FALSE;
  while (i < l && !stop)
    {
    char c = s[i];
    if (c == ' ' || c == '\n' || c == '\t')
      {
      pos++;
      }
    else
      stop = TRUE;
    i++;
    }
  char *s_new = strdup (s + pos);
  free (self->str);
  self->str = s_new;
  }


/*==========================================================================
  string_trim_right
*==========================================================================*/
void string_trim_right (String *self)
  {
  char *s = self->str;
  int l = strlen (s);
  int i = l - 1;
  BOOL stop = FALSE;
  while (i >= 0 && !stop)
    {
    char c = s[i];
    if (c == ' ' || c == '\n' || c == '\t')
      {
      s[i] = 0;
      }
    else
      stop = TRUE;
    i--;
    }
  }


/*==========================================================================
  string_utf8_to_utf32 
*==========================================================================*/
UTF32 *string_utf8_to_utf32 (const UTF8 *_in)
  {
  const UTF8* in = (UTF8 *)_in;
  int max_out = strlen ((char *)_in);
  UTF32 *out = malloc ((max_out + 1) * sizeof (UTF32));
  memset (out, 0, max_out * sizeof (UTF32));
  UTF32 *out_temp = out;

  ConvertUTF8toUTF32 ((const UTF8 **)&in, (const UTF8 *)in+strlen((char *)in),
      &out_temp, out + max_out, 0);

  int len = out_temp - out;
  out [len] = 0;
  return out;
  }


/*==========================================================================
  string_utf32_to_UTF8
*==========================================================================*/
UTF8 *string_utf32_to_utf8 (const UTF32 *_in)
  {
  const UTF32 *s = _in;
  String *temp = string_create_empty();
  int i, l = wstring_length_utf32 (s);
  for (i = 0; i < l; i++)
     string_append_c (temp, s[i]);

  char *ret = strdup (string_cstr (temp));
  string_destroy (temp);
  return (UTF8 *)ret;
  }


/*==========================================================================
  string_ends_with 
*==========================================================================*/
BOOL string_ends_with (const String *self, const char *test)
  {
  BOOL ret = FALSE;
  int pos = string_find (self, test);
  if (pos >= 0)
    {
    int lself = string_length (self);
    int ltest = strlen (test);
    if (pos == lself - ltest)
      ret = TRUE;
    }
  return ret;
  }


/*==========================================================================
  string_alpha_sort_fn
  A function to use with list_sort to sort a list of strings into 
    alphabetic (ASCII) order
*==========================================================================*/
int string_alpha_sort_fn (const void *p1, const void*p2, void *user_data)
  {
  String *s1 = * (String * const *) p1;
  String *s2 = * (String * const *) p2;
  const char *ss1 = string_cstr (s1);
  const char *ss2 = string_cstr (s2);
  return strcmp (ss1, ss2);
  }


/*==========================================================================
  string_write_to_file
*==========================================================================*/
BOOL string_write_to_file (const String *self, const char *file)
  {
  return file_write_from_string (file, self);
  }


/*==========================================================================
  string_write_to_path
*==========================================================================*/
BOOL string_write_to_path (const String *self, const Path *path)
  {
  return path_write_from_string (path, self);
  }

/*==========================================================================
  string_split

  Returns a List of String objects. The string is split using strtok(),
  and so this method has all the limitations that strtok() has. In 
  particular, there's no way to enter an empty token -- multiple delimiters
  are collapsed into one.

  This method always returns a List, but it may be empty if the input
  string was empty.
*==========================================================================*/
List *string_split (const String *self, const char *delim)
  {
  List * l = list_create ((ListItemFreeFn)string_destroy);

  char *s = strdup (self->str);
  
  char *tok = strtok (s, delim);

  if (tok)
    {
    do
      {
      list_append (l, string_create (tok));
      } while ((tok = strtok (NULL, delim)));
    }

  free (s);

  return l;
  }


