/*============================================================================
  boilerplate 
  wraptext.c

  Functions to fit text into a console of a specified width. The easiest
  ways to use these functions are just to call the wraptext_easy_XXX
  versions, but these are inefficient when many calls are made -- they 
  all allocate and free a lot of memory on each call.

  Copyright (c)2017 Kevin Boone, GPL v3.0
============================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "defs.h"
#include "wraptext.h"
#include "convertutf.h"
#include "string.h"
#include "wstring.h"

#define WT_STATE_START 0
#define WT_STATE_WORD 1
#define WT_STATE_WHITE 2

typedef struct _WrapTextContextPriv 
  {
  WrapTextOutputFn outputFn;
  int width;
  int flags;
  int state;
  int column;
  int white_count;
  void *app_data;
  UTF32 last;
  UTF32 *token;
  } WrapTextContextPriv;


void _stdout_output_fn (void *app_data, UTF32 c)
  {
  UTF8 buff [WT_UTF8_MAX_BYTES];  
  wraptext_context_utf32_char_to_utf8 (c, buff);
  fputs ((char *)buff, stdout); 
  }


static void _wraptext_append_token (WrapTextContext *context, const UTF32 c)
  {
  UTF32 *token = context->priv->token;
  if (!token)
    {
    token = malloc (sizeof (UTF32));
    token[0] = 0;
    }
   
  int l = wstring_length_utf32 (token);
   
  token = realloc (token, (l+2) * sizeof (UTF32));

  token [l] = c;
  token [l+1] = 0;

  context->priv->token = token;
  }


// Whitespace other than newline
BOOL _wraptext_is_white (UTF32 c)
  {
  if (c == 32) return TRUE;
  if (c == 9) return TRUE;
  //TODO -- other unicode whitespace chars
  return FALSE;
  }

BOOL _wraptext_is_newline (UTF32 c)
  {
  if (c == 10) return TRUE;
  return FALSE;
  }


void _wraptext_new_line (WrapTextContext *context)
  {
  context->priv->outputFn (context->priv->app_data, (UTF32)'\n'); 
  context->priv->column = 0;
  }


void _wraptext_flush_string (WrapTextContext *context, UTF32 *s)
  {
  int i, l = wstring_length_utf32 (s);

  if (l + context->priv->column + 1 >= context->priv->width)
    {
    context->priv->outputFn (context->priv->app_data, (UTF32)'\n'); 
    context->priv->column = 0;
    }
 
  //TODO TODO TODO
  
  for (i = 0; i < l; i++)
    {
    UTF32 c = s[i];
    context->priv->outputFn (context->priv->app_data, c); 
    }

  context->priv->column += l;
  }


void _wraptext_flush_space (WrapTextContext *context, BOOL allowAtStart)
  {
  if ((context->priv->column > 0) || allowAtStart)
    {
    //UTF32 s[2];
    //s[0] = ' ';
    //s[1] = 0;
    //_wraptext_flush_string (context, s);
    context->priv->outputFn (context->priv->app_data, ' '); 
    context->priv->column++;
    }
  }


void _wraptext_flush_token (WrapTextContext *context)
  {
  UTF32 *token = context->priv->token;
  // Don't flush anything -- even a space -- if the token is
  //  null. This will only happen at end-of-line or end-of-file
  //  states (hopefully)
  if (token)
    {
//printf ("\nflush %d  %ls\n", context->priv->column, token);
//printf ("!");
    _wraptext_flush_string (context, token);
//printf ("#");
    _wraptext_flush_space (context, FALSE);
//printf ("@");
    free (context->priv->token);
    }
  context->priv->token = NULL;
  }


void _wraptext_wrap_next (WrapTextContext *context, const UTF32 c)
  {
  UTF32 last = context->priv->last;

  int state = context->priv->state;

  // This logic counts spaces at the ends of lines, so MD-style
  //   double-space linebreaks can be respected.
  if (_wraptext_is_newline (c))
    {
    }
  else
    {
    if (_wraptext_is_white (c))
      context->priv->white_count++;
    else
      context->priv->white_count = 0;
    }
  
  // STATE_START

  if (state == WT_STATE_START && _wraptext_is_newline (c))
     {
     // Double blank line -- respect this as a para separator
     _wraptext_new_line (context); 
     _wraptext_new_line (context); 
     state = WT_STATE_WHITE;
     }
  else if (state == WT_STATE_START && _wraptext_is_white (c))
     {
     // Space at the beginning of the line
     // Do nothing yet TODO
     }
  else if (state == WT_STATE_START)
     {
     _wraptext_append_token (context, c);
     state = WT_STATE_WORD;
     }

  // STATE_WORD

  else if (state == WT_STATE_WORD && _wraptext_is_newline (c))
     {
     if (context->priv->flags & WRAPTEXT_RESPECT_NEWLINE)
       {
       _wraptext_flush_token (context);
       _wraptext_new_line (context); 
       }
     else
       {
       _wraptext_flush_token (context);
       }
     state = WT_STATE_START;
     }
  else if (state == WT_STATE_WORD && _wraptext_is_white (c))
     {
     _wraptext_flush_token (context);
     state = WT_STATE_WHITE;
     }
  else if (state == WT_STATE_WORD)
     {
     _wraptext_append_token (context, c);
     state = WT_STATE_WORD;
     }
  
  // STATE_WHITE

  else if (state == WT_STATE_WHITE && _wraptext_is_newline (c))
     {
     if (context->priv->flags & WRAPTEXT_RESPECT_NEWLINE)
       {
       _wraptext_flush_token (context);
       _wraptext_new_line (context); 
       }
     else
       {
       _wraptext_flush_token (context);
       if (context->priv->white_count > 1
          && (context->priv->flags & WRAPTEXT_RESPECT_MD_LINEBREAK))
         {
         _wraptext_new_line (context);
         }
       }
     state = WT_STATE_START;
     }
  else if (state == WT_STATE_WHITE && _wraptext_is_white (c))
     {
     state = WT_STATE_WHITE;
     }
  else if (state == WT_STATE_WHITE)
     {
     _wraptext_append_token (context, c);
     state = WT_STATE_WORD;
     }
  

  else
     {
     fprintf (stderr, "Internal error: char %d in state %d\n", c, state);
     exit (-1);
     }

  context->priv->last = last;
  context->priv->state = state;
  }


void wraptext_eof (WrapTextContext *context)
  {
  // Handle any input that has not been handled already
  _wraptext_flush_token (context);
  // TODO -- in MD mode, tidy up any outstanding formatting characters
  if (!_wraptext_is_newline (context->priv->last))
    _wraptext_new_line (context);
  }


void wraptext_wrap_utf32 (WrapTextContext *context, const UTF32 *utf32)
  {
  int i, len = wstring_length_utf32 (utf32);
  for (i = 0; i < len; i++)
    {
    UTF32 c = utf32[i];
    _wraptext_wrap_next (context, c);
    }
  }


void wraptext_wrap_utf8 (WrapTextContext *context, const UTF8 *utf8)
  {
  UTF32 *utf32 = string_utf8_to_utf32 (utf8); 
  wraptext_wrap_utf32 (context, utf32);
  free (utf32);
  }


void wraptext_easy_stdout_utf32 (const int width, const UTF32 *utf32,
     int flags)
  {
  WrapTextContext *context = wraptext_context_new();
  wraptext_context_set_output_fn (context, _stdout_output_fn);
  wraptext_context_set_flags (context, flags);
  wraptext_context_set_width (context, width);
  wraptext_wrap_utf32 (context, utf32);
  wraptext_eof (context);
  wraptext_context_free (context);
  }


void wraptext_easy_stdout_utf8 (const int width, const UTF8 *utf8,
     int flags)
  {
  UTF32 *utf32 = string_utf8_to_utf32 ((const UTF8 *)utf8); 
  wraptext_easy_stdout_utf32 (width, utf32, flags);
  free (utf32);
  }


WrapTextContext *wraptext_context_new (void)
  {
  WrapTextContext *self = malloc (sizeof (WrapTextContext));
  memset (self, 0, sizeof (WrapTextContext));
  WrapTextContextPriv *priv = malloc (sizeof (WrapTextContextPriv));
  memset (priv, 0, sizeof (WrapTextContextPriv));
  self->priv = priv;
  self->priv->width = 80;
  self->priv->outputFn = _stdout_output_fn;
  wraptext_context_reset (self);
  return self;
  }


void wraptext_context_reset (WrapTextContext *self)
  {
  self->priv->state = WT_STATE_START;
  self->priv->column = 0;
  self->priv->last = 0;
  self->priv->white_count = 0;
  if (self->priv->token) free (self->priv->token);
  self->priv->token = NULL;
  }


void wraptext_context_set_output_fn (WrapTextContext *self, 
    WrapTextOutputFn fn)
  {
  self->priv->outputFn = fn;
  }


void wraptext_context_set_width (WrapTextContext *self, int width)
  {
  self->priv->width = width;
  }

void wraptext_context_set_flags (WrapTextContext *self, int flags)
  {
  self->priv->flags = flags;
  }

void wraptext_context_set_app_data (WrapTextContext *self, void *app_data)
  {
  self->priv->app_data = app_data;
  }


void wraptext_context_free (WrapTextContext *self)
  {
  if (!self) return;
  if (self->priv)
    {
    free (self->priv);
    self->priv = NULL;
    }
  free (self);
  }


void wraptext_context_utf32_char_to_utf8 (const UTF32 c, UTF8* utf8)
  {
  UTF32 _in = c;
  const UTF32* in = &_in;
  int max_out = WT_UTF8_MAX_BYTES;
  UTF8 *out = (UTF8 *)utf8;
  memset (out, 0, max_out * sizeof (UTF8));
  UTF8 *out_temp = out;

  ConvertUTF32toUTF8 (&in, in + 1,
      &out_temp, out + max_out * 4, 0);
  int len = out_temp - out;
  utf8[len] = 0;
  }



