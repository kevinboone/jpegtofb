/*============================================================================

  jpegtofb
  jpegtofb.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include "defs.h"

BEGIN_DECLS

void jpegtofb_putonfb (const char *fbdev, const char *filename, 
        char **error);

END_DECLS


