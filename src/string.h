/*============================================================================
  boilerplate 
  string.h
  Copyright (c)2017 Kevin Boone, GPL v3.0
============================================================================*/

#pragma once

#include <stdint.h>
#include "defs.h"
#include "list.h"

struct _String;
typedef struct _String String;

BEGIN_DECLS

String      *string_create_empty (void);
String      *string_create (const char *s);
String      *string_clone (const String *self);
int          string_find (const String *self, const char *search);
int          string_find_last (const String *self, const char *search);
void         string_destroy (String *self);
const char  *string_cstr (const String *self);
const char  *string_cstr_safe (const String *self);
void         string_append_printf (String *self, const char *fmt,...);
void         string_append (String *self, const char *s);
void         string_append_c (String *self, const uint32_t c);
void         string_prepend (String *self, const char *s);
int          string_length (const String *self);
String      *string_substitute_all (const String *self, 
                const char *search, const char *replace);
void        string_delete (String *self, const int pos, 
                const int len);
void        string_insert (String *self, const int pos, 
                const char *replace);
BOOL        string_create_from_utf8_file (const char *filename, 
                String **result, char **error);
String     *string_encode_url (const char *s);
void        string_append_byte (String *self, const BYTE byte);
void        string_trim_left (String *self);
void        string_trim_right (String *self);
UTF32      *string_utf8_to_utf32 (const UTF8 *_in);
UTF8       *string_utf32_to_utf8 (const UTF32 *_in);
BOOL        string_ends_with (const String *self, const char *test);
int         string_alpha_sort_fn (const void *p1, const void*p2, 
               void *user_data);
List       *string_split (const String *self, const char *delim);

END_DECLS

