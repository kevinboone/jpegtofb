/*==========================================================================

  jpegtofb
  program.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

  This file contains the main body of the program. By the time
  program_run() has been called, RC files will have been read and comand-
  line arguments parsed, so all the contextual information will be in the
  ProgramContext. Logging will have been initialized, so the log_xxx
  methods will work, and be filtered at the appopriate levels.
  The unparsed command-line arguments will be available
  in the context as nonswitch_argc and nonswitch_argv.

==========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <signal.h>
#include "program_context.h" 
#include "feature.h" 
#include "program.h" 
#include "console.h" 
#include "string.h" 
#include "list.h" 
#include "usage.h" 
#include "jpegtofb.h" 
#include "jpegreader.h" 
#include "slideshow.h" 


Slideshow *slideshow = NULL;


/*==========================================================================

  program_next_picture

==========================================================================*/
BOOL program_next_picture (void)
  {
  LOG_IN
  BOOL ret = FALSE;
  char *error = NULL;
  slideshow_show_and_increment (slideshow, &error);
  if (error)
    {
    log_error (error);
    free (error);
    ret = FALSE;
    }
  else
    ret = TRUE;
  LOG_OUT
  return ret;
  }


/*==========================================================================

  program_signal_usr1

==========================================================================*/
void program_signal_usr1 (int dummy)
  {
  program_next_picture ();
  }


/*==========================================================================

  program_check_for_slideshow

  Check whether the file exists, seems to be good JPEG, and meets 
  whatever inclusion criteria the user has specified

==========================================================================*/
BOOL program_check_for_slideshow (const ProgramContext *context, 
       const char *filename)
  {
  LOG_IN
  BOOL ret = FALSE;
  log_debug ("check_for_slideshow: %s", filename);

  char *error = NULL;
  jpegreader_check (filename, &error); 
  if (error == NULL)
    {
    int height = 0, width = 0, components = 0;
    if (jpegreader_get_image_size (filename, &height, 
            &width, &components))
      {
      BOOL landscape = program_context_get_boolean 
        (context, "landscape", FALSE);
      if (landscape) 
        {
        if (width > height)
          ret = TRUE;
        else
          log_warning ("Excluding from slideshow: %s: portrait format", 
                filename);
        }
      else
        ret = TRUE;
      }
    }
  else
    {
    log_warning ("Excluding from slideshow: %s: %s", filename,
       error);
    free (error);
    }

  LOG_OUT
  return ret;
  }


/*==========================================================================

  program_run

  The return value will eventually become the exit value from the program.

==========================================================================*/
int program_run (ProgramContext *context)
  {
  int ret = 0;
  char ** const argv = program_context_get_nonswitch_argv (context);
  int argc = program_context_get_nonswitch_argc (context);

  if (argc >= 2)
    {
    log_debug ("Single image mode");
    const char *filename = argv[1]; 
    const char *fbdev = "/dev/fb0";
    const char *arg_fbdev = program_context_get (context, "fbdev");
    if (arg_fbdev) fbdev = arg_fbdev;

    if (argc == 2)
      {
      char *error = NULL;
      jpegtofb_putonfb (fbdev, filename, &error);
      if (error)
        {
        log_error (error);
        free (error);
        ret = -1;
        }
      else ret = 0;
      }
    else
      {
      log_debug ("Slideshow mode");
      // We are in slideshow mode, with potentially multiple
      //   pictures
      slideshow = slideshow_create (fbdev);

      for (int i = 1; i < argc; i++)
        {
        if (program_check_for_slideshow (context, argv[i]))
          {
          slideshow_add_picture (slideshow, argv[i]);
          }
        } 
  
      int l = slideshow_length (slideshow);
      if (l > 0)
        {
        BOOL randomize = program_context_get_boolean (context, 
          "randomize", FALSE);

        if (randomize)
          {
          log_debug ("Slideshow randomize");
          srand (time (NULL));
          slideshow_randomize (slideshow);

          }
 
        signal (SIGUSR1, program_signal_usr1); 

        int seconds = program_context_get_integer (context, 
          "sleep", 60);
        log_debug ("slideshow sleep is %d seconds", seconds);
        while (TRUE)
          {
          program_next_picture ();
          sleep (seconds); // TODO
          }
        }
      else
        {
        log_error ("No valid JPEG pictures found in list");
        }

      slideshow_destroy (slideshow);
      slideshow = NULL;
      }
    }
  else
    {
    usage_show (stderr, argv[0]);
    ret = -1;
    }

  return ret;
  }

