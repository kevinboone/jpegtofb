/*==========================================================================

  jpegtofb
  jpegreader.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

  This file contains functions that wrap the libjpeg functions to
  make them easier to use, and also functions to check the 
  basic sanity of JPEG files

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


/*==========================================================================

  jpegreader_get_image_size

==========================================================================*/
BOOL jpegreader_get_image_size (const char *filename, int *height, 
       int *width, int *components)
  {
  LOG_IN
  BOOL ret = FALSE;
  log_debug ("get_image_size: file=%s", filename);
  if (jpegreader_check (filename, NULL)) 
    {
    FILE *fin = fopen (filename, "r");
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error (&jerr);
    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src (&cinfo, fin);

    int rc = jpeg_read_header(&cinfo, TRUE);
    if (rc == 1) 
      {
      jpeg_start_decompress (&cinfo);
      *width = cinfo.output_width;
      *height = cinfo.output_height;
      *components = cinfo.output_components;
      //jpeg_finish_decompress(&cinfo);
      jpeg_destroy_decompress(&cinfo);
      ret = TRUE;
      }
    fclose (fin);
    }
  LOG_OUT
  return ret;
  }


/*==========================================================================

  jpegreader_file_to_mem

==========================================================================*/
void jpegreader_file_to_mem (const char *filename, int *jpeg_height, 
      int *jpeg_width, int *bytespp, char **buffer, char **error)
  {
  LOG_IN
  log_debug ("read_jpeg: file=%s", filename);
  if (jpegreader_check (filename, error)) 
    {
    FILE *fin = fopen (filename, "r");
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error (&jerr);
    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src (&cinfo, fin);

    int rc = jpeg_read_header(&cinfo, TRUE);
    if (rc == 1) 
      {
      jpeg_start_decompress(&cinfo);
	    
      int width = cinfo.output_width;
      int height = cinfo.output_height;
      int pixel_size = cinfo.output_components;
      if (pixel_size == 3)
        {
	*jpeg_width = width;
	*jpeg_height = height;
	*bytespp = pixel_size;

        log_debug ("read_jpeg: image is %d by %d with %d components", 
	    width, height, pixel_size);
	unsigned long bmp_size;
	char *bmp_buffer;

	bmp_size = width * height * pixel_size;
	bmp_buffer = (char*) malloc(bmp_size);

	int row_stride = width * pixel_size;

	while (cinfo.output_scanline < cinfo.output_height) 
	  {
	  char *buffer_array[1];
	  buffer_array[0] = bmp_buffer + cinfo.output_scanline * row_stride;
	  jpeg_read_scanlines (&cinfo, (unsigned char **)buffer_array, 1);
	  }
        *buffer = bmp_buffer;
        } 
      else
        {
        asprintf (error, "JPEG file '%s' is not RGB", filename); 
        }
      jpeg_finish_decompress(&cinfo);
      jpeg_destroy_decompress(&cinfo);
      }
    else
      {
      asprintf (error, "Invalid JPEG file '%s'", filename); 
      }
    fclose (fin);
    }
  LOG_OUT
  }


/*==========================================================================

  jpegreader_check

  This is a quick check that the file exists, and looks like a JPEG.
  We don't want to rely on libjpeg functions to do this check, because
  their error handling really stinks. Better to make sure the file
  is basically sane, before letting libjpeg get stuck in.

==========================================================================*/
BOOL jpegreader_check (const char *filename, char **error)
  {
  LOG_IN
  BOOL ret = FALSE;

  int f = open (filename, O_RDONLY);
  if (f >= 0)
    {
    unsigned char buff[2];
    int n = read (f, buff, 2);
    if (n == 2)
      {
      if (buff[0] == 0xff && buff[1] == 0xd8)
        { 
        ret = TRUE;
        }
      else
        { 
        if (error)
           asprintf (error, "Can't read '%s': %s", 
            filename, "no JPEG header");
        }
      }
    else
      {
      if (error)
         asprintf (error, "Can't read '%s': %s", 
          filename, "file too short");
      }
    close (f);
    }
  else
    {
    if (error)
       asprintf (error, "Can't read '%s': %s", 
          filename, strerror (errno));
    }
  LOG_OUT
  return ret;
  }




