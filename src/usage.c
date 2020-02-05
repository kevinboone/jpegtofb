/*==========================================================================

  boilerplate
  usage.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

==========================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "feature.h" 
#include "usage.h" 


/*==========================================================================
  usage_show
==========================================================================*/
void usage_show (FILE *fout, const char *argv0)
  {
  fprintf (fout, "Usage: %s [options] {images}\n", argv0);
  fprintf (fout, "  -d,--fbdev=device    framebuffer device\n");
  fprintf (fout, "  -h,--help            show this message\n");
  fprintf (fout, "  -l,--landscape       only include landscape format in slideshow\n");
  fprintf (fout, "  -r,--randomize       randomize slideshow order\n");
  fprintf (fout, "     --log-level=N     log level, 0-5 (default 2)\n");
  fprintf (fout, "  -s,--sleep=seconds   time between images in slideshow mode (60)\n");
  fprintf (fout, "     --syslog          messages to system log\n");
  fprintf (fout, "  -v,--version         show version\n");
  fprintf (fout, "  -w,--width=N         set text output width; 0=no format\n");
  }

 
