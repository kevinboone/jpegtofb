/*============================================================================
  boilerplate 
  wstring.h
  Copyright (c)2017 Kevin Boone, GPL v3.0
============================================================================*/

#pragma once

#include <stdint.h>
#include "defs.h"

struct _WString;
typedef struct _WString WString;

BEGIN_DECLS

WString     *wstring_create_empty (void);
WString     *wstring_create (const UTF32 *s);
WString     *wstring_create_from_utf8 (const UTF8 *s);
UTF8        *wstring_to_utf8 (const WString *self);
const UTF32 *wstring_cstr (const WString *self);
void         wstring_destroy (WString *self);
int          wstring_length_utf32 (const UTF32 *s);
int          wstring_length (const WString *self);
const UTF32 *wstring_cstr_safe (const WString *self);
void         wstring_append (WString *self, const UTF32 *s);
void         wstring_prepend (WString *self, const UTF32 *s);
void         wstring_append_printf (WString *self, const char *fmt,...); 
WString     *wstring_clone (const WString *self);
int          wstring_find (const WString *self, const UTF32 *search);
int          wstring_find_last (const WString *self, const UTF32 *search);
void         wstring_delete (WString *self, int pos, int len);
void         wstring_insert (WString *self, int pos, 
               const UTF32 *replace);
WString     *wstring_substitute_all (const WString *self, 
                const UTF32 *search, const UTF32 *replace);
void         wstring_append_c (WString *self, const uint32_t ch);
BOOL         wstring_ends_with (const WString *self, const UTF32 *test);

END_DECLS


