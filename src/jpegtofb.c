/*==========================================================================

  jpegtofb
  jpegtofb.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

  This file contains functions that write a JPEG file to the framebuffer 

==========================================================================*/
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "jpeglib.h"
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include "log.h" 
#include "jpegreader.h" 
#include "jpegtofb.h" 


/*==========================================================================

  transform

  The buffers pointed to on entry are both 3-bytes per pixel although,
  of course, the output buffer is empty, and will get filled up.
  After the transformation, the 3-byte per pixel output buffer will
  get transformed to match the framebuffer pixel size.

  This function will behave very badly if the input and output
  images are different aspect ratios

==========================================================================*/
static void transform (char *in, int in_height, int in_width, char *out, 
    int out_height, int out_width)
  {
  LOG_IN
  double scale = (double)in_width / (double)out_width;
  log_debug ("transform: scale=%f", scale);
  for (int i = 0; i < out_height; i++)
    {
    int new_y = i * scale;
    for (int j = 0; j < out_width; j++)
      {
      int new_x = j * scale; 
      int index24in = (new_y * in_width + new_x) * 3;
      int index24out = (i * out_width + j) * 3;
      char r = in[index24in + 0];
      char g = in[index24in + 1];
      char b = in[index24in + 2];
      out[index24out + 0] = r;
      out[index24out + 1] = g;
      out[index24out + 2] = b;
      }
    }
  LOG_OUT
  }



/*==========================================================================

  jpegtofb_putonfb

==========================================================================*/
void jpegtofb_putonfb (const char *fbdev, const char *filename, 
     char **error)
  {
  LOG_IN
  *error = NULL;  

  // Read the JPEG file into a buffer. The buffer _should_ be 
  //  3 bytes per pixel. I'm not sure what to do if it isn't

  int jpeg_width = 0, jpeg_height = 0, jpeg_bytes = 0;
  char *bmp_buffer = 0;

  jpegreader_file_to_mem (filename, &jpeg_height, &jpeg_width, 
    &jpeg_bytes, &bmp_buffer, error);
  if (*error == NULL)
    {
    int fbfd = open (fbdev, O_RDWR);
    if (fbfd >= 0)
      {
      struct fb_var_screeninfo vinfo;

      ioctl (fbfd, FBIOGET_VSCREENINFO, &vinfo);

      log_debug ("putonfb: xres %d", vinfo.xres); 
      log_debug ("putonfb: yres %d", vinfo.yres); 
      log_debug ("putonfb: bpp %d", vinfo.bits_per_pixel); 

      int fb_width = vinfo.xres;
      int fb_height = vinfo.yres;
      int fb_bpp = vinfo.bits_per_pixel;
      int fb_bytes = fb_bpp / 8;

      double aspect = (double)jpeg_width / (double) jpeg_height;

      // Fit height -- TODO
      int fit_height = fb_height;
      int fit_width = (int) fit_height * aspect;
      
      char *out_24bpp = malloc (fit_height * fit_width * 3);
      transform (bmp_buffer, jpeg_height, jpeg_width, 
        out_24bpp, fit_height, fit_width);

      int fb_data_size = fb_width * fb_height * fb_bytes;

      char *fbdata = mmap (0, fb_data_size, 
	     PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, (off_t)0);

      memset (fbdata, 0, fb_data_size);

      // xoff is the number of pixels between the left edge of the photo,
      //   and the left edge of the screen. 
      // If the picture is wider than the screen, then x_off will be negative,
      //  and some parts of the picture will not be displayed
      // If the picture is narrower than the screen, the x_off will be positive,
      //  and some parts of the screen will be black
 
      int x_off = (fb_width - fit_width) / 2;

      int y_off = (fb_height - fit_height) / 2;
      for (int i = 0; i < fb_height; i++)
	{
        int y24 = i - y_off;
        int y32 = i;
        if (y32 >= 0 && y32 < fb_height)
          {
	  for (int j = 0; j < fb_width; j++)
	    {
            int x24 = j - x_off;
            int x32 = j;
            if (x32 > 0 && x32 < fb_width && x24 > 0 && x24 < fit_width)
              {
	      int index24 = (y24 * fit_width + x24) * 3;
	      int index32 = (y32 * fb_width + x32) * 4;
	      char r = out_24bpp [index24 + 0];
	      char g = out_24bpp [index24 + 1];
	      char b = out_24bpp [index24 + 2];
	      char a = 0xFF;
	      fbdata [index32 + 0] = b;
	      fbdata [index32 + 1] = g;
	      fbdata [index32 + 2] = r;
	      fbdata [index32 + 3] = a;
              }
            }
	  }
        }
      munmap (fbdata, fb_data_size);
      free (out_24bpp);
      }
    else
      {
      asprintf (error, "Can't open framebuffer '%s': %s", fbdev, 
        strerror (errno));
      }
    free(bmp_buffer);
    close (fbfd);
    }
  LOG_OUT
  }

