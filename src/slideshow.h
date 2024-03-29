/*============================================================================

  jpegtofb
  slideshow.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include "defs.h"

struct _Slideshow;
typedef struct _Slideshow Slideshow;

BEGIN_DECLS

Slideshow  *slideshow_create (const char *fbdev, BOOL fit_to_width);
void        slideshow_destroy (Slideshow *self);
void        slideshow_add_picture (Slideshow *self, const char *filename);
void        slideshow_show_and_increment (Slideshow *self, char **error);
int         slideshow_length (const Slideshow *self);
void        slideshow_randomize (Slideshow *self);
END_DECLS


