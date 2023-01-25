/*============================================================================

  jpegtofb
  jpegreader.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include "defs.h"


BEGIN_DECLS

void     jpegreader_file_to_mem (const char *filename, int *jpeg_height, 
            int *jpeg_width, int *bytespp, char **buffer, char **error);
BOOL     jpegreader_check (const char *filename, char **error);
BOOL     jpegreader_get_image_size (const char *filename, int *height, 
            int *width, int *components);

END_DECLS



