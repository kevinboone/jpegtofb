/*============================================================================
  libwraptext
  wraptext.h
  Copyright (c)2017 Kevin Boone, GPL v3.0
============================================================================*/

#ifndef __WRAPTEXT_H
#define __WRAPTEXT_H

#include <stdint.h>

// Always treat a newline in the source as forcing a newline in the
//  output. Only meaningful at all if the source cotains no newlines
//  _except_ where they have this function
#define  WRAPTEXT_RESPECT_NEWLINE      0x00000001

// Treat a double-space as a line break, as in some variants of
//   Markdown
#define  WRAPTEXT_RESPECT_MD_LINEBREAK 0x00000002

// The largest number of bytes required to store a unicode character as
//  UTF8, including a terminating 0. Theoretically this is unlimited, but
//  there are few non-sinister reasons to present a UTF-8 character longer
//  than about five bytes
#define WT_UTF8_MAX_BYTES 8 

typedef int32_t WT_UTF32;
typedef char WT_UTF8;

typedef void (*WrapTextOutputFn) (void *app_data, WT_UTF32 c);

struct _WrapTextContextPriv;

typedef struct _WrapTextContent
  {
  struct _WrapTextContextPriv *priv;
  } WrapTextContext;

#ifdef __CPLUSPLUS
extern "C" {
#endif

void wraptext_easy_stdout_utf8 (const int width, const UTF8 *utf8,
     int flags);
void wraptext_wrap_utf32 (WrapTextContext *context, const UTF32 *utf32);
void wraptext_wrap_utf8 (WrapTextContext *context, const UTF8 *utf8);

WrapTextContext *wraptext_context_new (void);
void wraptext_context_free (WrapTextContext *self);
void wraptext_context_set_output_fn (WrapTextContext *self, 
  WrapTextOutputFn fn);
void wraptext_context_set_flags (WrapTextContext *self, int flags);
void wraptext_context_set_width (WrapTextContext *self, int width);
void wraptext_context_set_app_data (WrapTextContext *self, void *app_data);
void wraptext_context_reset (WrapTextContext *self);
void wraptext_eof (WrapTextContext *context);

UTF32 *wraptext_convert_utf8_to_utf32 (const UTF8 *utf8);

/** Convert a single UTF32 character to a UTF8 representation, where
 * the UTF8 is an array of characters terminated with a zero. The 
 * utf8 parameter must be a pointed to an array of UTF8 (aka char)
 * of at least UTF8_MAX_BYTES size. */
void wraptext_context_utf32_char_to_utf8 (const UTF32 c, UTF8* utf8);

#ifdef __CPLUSPLUS
}
#endif

#endif
