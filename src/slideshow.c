/*============================================================================

  jpegtofb
  slideshow.c
  Copyright (c)2017 Kevin Boone, GPL v3.0


============================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include "string.h" 
#include "defs.h" 
#include "log.h" 
#include "list.h" 
#include "slideshow.h" 
#include "jpegtofb.h" 

struct _Slideshow
  {
  char *fbdev;
  List *list;
  int index;
  }; 


/*==========================================================================

  slideshow_create

*==========================================================================*/
Slideshow *slideshow_create (const char *fbdev)
  {
  LOG_IN
  Slideshow *self = malloc (sizeof (Slideshow));
  self->fbdev = strdup (fbdev);
  self->list = list_create ((ListItemFreeFn)free);
  self->index = 0;
  LOG_OUT
  return self;
  }


/*==========================================================================

  slideshow_destroy

*==========================================================================*/
void slideshow_destroy (Slideshow *self)
  {
  LOG_IN
  if (self)
    {
    if (self->fbdev) 
      {
      free (self->fbdev);
      self->fbdev = NULL;
      }
    if (self->list)
      {
      list_destroy (self->list);
      self->list = NULL;
      }
    free (self);
    }
  LOG_OUT
  }

/*==========================================================================

  slideshow_add_picture

*==========================================================================*/
void slideshow_add_picture (Slideshow *self, const char *filename)
  {
  LOG_IN
  log_debug ("Add picture: %s", filename);
  list_append (self->list, strdup (filename));
  LOG_OUT
  }

/*==========================================================================

  slideshow_show_and_increment
  
  returns true if the current image in the sequence could be shown

*==========================================================================*/
void slideshow_show_and_increment (Slideshow *self, char **error)
  {
  LOG_IN
  int l = list_length (self->list);
  const char *filename = list_get (self->list, self->index);
  log_debug ("show_and_increment l=%d, index=%d, file=%s",
         l, self->index, filename);

  jpegtofb_putonfb (self->fbdev, filename, error);

  self->index++;
  if (self->index == l)
    self->index = 0;
  LOG_OUT
  }

/*==========================================================================

  slideshow_length
  
*==========================================================================*/
int slideshow_length (const Slideshow *self)
  {
  return list_length (self->list);
  }

/*==========================================================================

  slideshow_randomize

  Shuffle the files in the list, keeping the overall list the same
  length.
  
*==========================================================================*/
void slideshow_randomize (Slideshow *self)
  {
  List *newlist = list_create ((ListItemFreeFn)free); 

  while (list_length (self->list) > 0)
    {
    // Be aware that list length will shrink as we proceed
    int l = list_length (self->list);
    int r = (int) ((double)rand() * l / RAND_MAX);
    char *s = list_get (self->list, r); 
    list_append (newlist, strdup (s));
    list_remove_object (self->list, s);
    }

  list_destroy (self->list);
  self->list = newlist;
  }




