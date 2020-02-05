/*==========================================================================
boilerplate
program_context.c
Copyright (c)2020 Kevin Boone
Distributed under the terms of the GPL v3.0

Methods for handling RC files and parsing the command line. It is assumed
that both the command line and the RC files potentially specify the same
properties, and that the command line overwrites values from the RC files.

There can be a system RC file at /etc/foo and a user RC file at 
/home/user/.foo, and the user file takes precedence.
==========================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <getopt.h>
#include "defs.h" 
#include "log.h" 
#include "props.h" 
#include "program_context.h" 
#include "console.h" 
#include "wraptext.h"
#include "string.h"
#include "path.h"
#include "usage.h"

struct _ProgramContext
  {
  Props *props;
  int nonswitch_argc;
  char **nonswitch_argv;
  BOOL stdout_is_tty;
  // Text output width, as determined from the console. May be -1 if
  //   not known
  int console_width;
  // Desired output width. Initialized to -1, meaning 'not set', but 
  //   may be set to 0 by the user. Printing functions will take -1
  //   to mean 'use console width', and 0 to mean 'do not format'. 
  int width;
  }; 


/*==========================================================================
  program_context_create
==========================================================================*/
ProgramContext *program_context_create (void)
  {
  LOG_IN
  ProgramContext *self = malloc (sizeof (ProgramContext));
  Props *props = props_create();
  self->props = props;
  props_put_integer (props, "log-level", MYLOG_WARNING);
  self->nonswitch_argc = 0;
  self->width = -1; // Might be overridden 
  LOG_OUT
  return self;
  }


/*==========================================================================
  program_context_query_console
==========================================================================*/
void program_context_query_console (ProgramContext *self)
  {
  // Find out console/stdout properties

  self->stdout_is_tty = isatty (STDOUT_FILENO);
  // We may be able to get the console width, even if stdout is
  //   redirected. That information may, or may not, be useful
  self->console_width = console_get_width ();

  if (self->console_width < 0) self->console_width = 80;
  log_debug ("Console width is %d", self->console_width);

  int set_width = program_context_get_integer (self, "width", -1);
  if (set_width >= 0)
    self->width = set_width;
  }


/*==========================================================================
  program_context_parse_command_line
  This needs to be called after program_context_read_rc_files, in order
  for command-line values to overwrite rc file values (if they have
  corresponding names)

  This method just process the command line, and turn the arguments into
  either context properties (program_context_put) on simply into values
  of attributes in the ProgramContext structure itself. Using properties is
  probably best, as it allows command-line arguments to override the proerties
  read from RC files. i
  
  In either case, it needs to be clear to the main program how the 
  command-line arguments have been translated in context data. 

  This method should only return TRUE if the intention is to proceed to
  run the rest of the program. Command-line switches that have the effect
  of terminating (like --help) should be handled internally, and
  FALSE returned.
==========================================================================*/
BOOL program_context_parse_command_line (ProgramContext *self, 
     int argc, char **argv)
  {
  LOG_IN

  BOOL ret = TRUE;
  static struct option long_options[] =
    {
      {"help", no_argument, NULL, 'h'},
      {"version", no_argument, NULL, 'v'},
      {"log-level", required_argument, NULL, 0},
      {"syslog", no_argument, NULL, 0},
      {"width", required_argument, NULL, 'w'},
      {"fbdev", required_argument, NULL, 'd'},
      {"sleep", required_argument, NULL, 's'},
      {"landscape", no_argument, NULL, 'l'},
      {"randomize", no_argument, NULL, 'r'},
      {0, 0, 0, 0}
    };

   int opt;
   while (ret)
     {
     int option_index = 0;
     opt = getopt_long (argc, argv, "hvlw:d:s:r",
     long_options, &option_index);

     if (opt == -1) break;

     switch (opt)
       {
       case 0:
         if (strcmp (long_options[option_index].name, "help") == 0)
           program_context_put_boolean (self, "show-usage", TRUE);
         else if (strcmp (long_options[option_index].name, "landscape") == 0)
           program_context_put_boolean (self, "landscape", TRUE);
         else if (strcmp (long_options[option_index].name, "syslog") == 0)
           program_context_put_boolean (self, "syslog", TRUE);
         else if (strcmp (long_options[option_index].name, "randomize") == 0)
           program_context_put_boolean (self, "randomize", TRUE);
         else if (strcmp (long_options[option_index].name, "version") == 0)
           program_context_put_boolean (self, "show-version", TRUE);
         else if (strcmp (long_options[option_index].name, "log-level") == 0)
           program_context_put_integer (self, "log-level", atoi (optarg)); 
         else if (strcmp (long_options[option_index].name, "width") == 0)
           program_context_put_integer (self, "width", atoi (optarg)); 
         else if (strcmp (long_options[option_index].name, "sleep") == 0)
           program_context_put_integer (self, "sleep", atoi (optarg)); 
         else if (strcmp (long_options[option_index].name, "fbdev") == 0)
           program_context_put (self, "fbdev", optarg); 
         else
           exit (-1);
         break;
       case 'h': case '?': 
         program_context_put_boolean (self, "show-usage", TRUE); break;
       case 'v': 
         program_context_put_boolean (self, "show-version", TRUE); break;
       case 'l': 
         program_context_put_boolean (self, "landscape", TRUE); break;
       case 'r': 
         program_context_put_boolean (self, "randomize", TRUE); break;
       case 'w': program_context_put_integer (self, "width", 
           atoi (optarg)); break;
       case 's': program_context_put_integer (self, "sleep", 
           atoi (optarg)); break;
       case 'd': program_context_put (self, "fbdev", optarg); break;
       default:
         ret = FALSE; 
       }
    }

  if (ret)
    {
    self->nonswitch_argc = argc - optind + 1;
    self->nonswitch_argv = malloc (self->nonswitch_argc * sizeof (char *));
    self->nonswitch_argv[0] = strdup (argv[0]);
    int j = 1;
    for (int i = optind; i < argc; i++)
      {
      self->nonswitch_argv[j] = strdup (argv[i]);
      j++;
      }
    }

  if (program_context_get_boolean (self, "show-version", FALSE))
    {
    printf ("%s: %s version %s\n", argv[0], NAME, VERSION);
    printf ("Copyright (c)2020 Kevin Boone\n");
    printf ("Distributed under the terms of the GPL v3.0\n");
    ret = FALSE;
    }

   if (program_context_get_boolean (self, "show-usage", FALSE))
    {
    usage_show (stdout, argv[0]);
    ret = FALSE;
    }

  LOG_OUT
  return ret;
  }


/*==========================================================================
  context_destroy
==========================================================================*/
void program_context_destroy (ProgramContext *self)
  {
  LOG_IN
  if (self)
    {
    if (self->props)
      props_destroy (self->props);
    for (int i = 0; i < self->nonswitch_argc; i++)
      free (self->nonswitch_argv[i]);
    free (self->nonswitch_argv);
    free (self);
    }
  LOG_OUT
  }


/*==========================================================================
  program_context_read_user_rc_file
==========================================================================*/
void program_context_read_user_rc_file (ProgramContext *self, 
       const char *rc_filename)
  {
#ifdef FEATURE_USER_RC
  LOG_IN
  char name[PATH_MAX];
  snprintf (name, PATH_MAX - 1, ".%s", rc_filename); 
  log_debug ("User RC file: %s\n", name);
  Path *path = path_create_home();
  path_append (path, name);
  props_read_from_path (self->props, path);
  path_destroy (path);
  LOG_OUT
#endif
  }


/*==========================================================================
  program_context_read_system_rc_file
==========================================================================*/
void program_context_read_system_rc_file (ProgramContext *self, 
       const char *rc_filename)
  {
#ifdef FEATURE_SYSTEM_RC
  LOG_IN
  char file[PATH_MAX];
  snprintf (file, PATH_MAX - 1, "/etc/%s", rc_filename); 
  log_debug ("System RC file: %s\n", file);
  props_read_from_file (self->props, file);
  LOG_OUT
#endif
  }


/*==========================================================================
  program_context_read_rc_files
==========================================================================*/
void program_context_read_rc_files (ProgramContext *self, 
       const char *rc_filename)
  {
  LOG_IN
  // Note that you can call props_read_from_file on multiple files, and
  //   values from the later reads will over-write the earlier ones. So
  //   you could read from /etc and from a home directory, for example
  program_context_read_user_rc_file (self, rc_filename);
  program_context_read_system_rc_file (self, rc_filename);
  LOG_OUT
  }


/*==========================================================================
  program_context_put
==========================================================================*/
void program_context_put (ProgramContext *self, const char *name, 
    const char *value)
  {
  props_put (self->props, name, value);
  }

/*==========================================================================
  program_context_get
==========================================================================*/
const char *program_context_get (const ProgramContext *self, const char *key)
  {
  return props_get (self->props, key);
  }

/*==========================================================================
  program_context_put_boolean
==========================================================================*/
void program_context_put_boolean (ProgramContext *self, 
    const char *key, BOOL value)
  {
  props_put_boolean (self->props, key, value);
  }

/*==========================================================================
  program_context_put_integer
==========================================================================*/
void program_context_put_integer (ProgramContext *self, 
    const char *key, int value)
  {
  props_put_integer (self->props, key, value);
  }

/*==========================================================================
  program_context_put_int64
==========================================================================*/
void program_context_put_int64 (ProgramContext *self, 
    const char *key, int64_t value)
  {
  props_put_int64 (self->props, key, value);
  }

/*==========================================================================
  program_context_get_boolean
==========================================================================*/
BOOL program_context_get_boolean (const ProgramContext *self, 
    const char *key, BOOL deflt)
  {
  return props_get_boolean (self->props, key, deflt);
  }

/*==========================================================================
  program_context_get_integer
==========================================================================*/
int program_context_get_integer (const ProgramContext *self, 
    const char *key, int deflt)
  {
  return props_get_integer (self->props, key, deflt);
  }


/*==========================================================================
  program_context_get_int64
==========================================================================*/
int64_t program_context_get_int64 (const ProgramContext *self, 
    const char *key, int64_t deflt)
  {
  return props_get_int64 (self->props, key, deflt);
  }


/*==========================================================================
  program_context_get_nonswitch_argc
==========================================================================*/
int program_context_get_nonswitch_argc (const ProgramContext *self)
  {
  return self->nonswitch_argc;
  }

/*==========================================================================
  program_context_get_nonswitch_argv
==========================================================================*/
char** const program_context_get_nonswitch_argv (const ProgramContext *self)
  {
  return self->nonswitch_argv;
  }

/*==========================================================================
  program_context_get_console_width
==========================================================================*/
int program_context_get_console_width (const ProgramContext *self)
  {
  return self->console_width;
  }

/*==========================================================================
  program_context_is_stdout_tty
==========================================================================*/
BOOL program_context_is_stdout_tty (const ProgramContext *self)
  {
  return self->stdout_is_tty;
  }


/*==========================================================================
  program_context_setup_logging
==========================================================================*/
void program_context_setup_logging 
       (ProgramContext *self, LogHandler log_handler, void *userdata)
  {
  log_set_level (program_context_get_integer (self, 
       "log-level", MYLOG_WARNING));
  log_set_handler (log_handler, userdata);
  }


/*==========================================================================
  program_context_printf
  Print a line or arbitrary length, breaking if necessary according to 
  the values of self->width and self->console_width. This function does
  not print a newline at the end if not asked for, just like printf. 
  However, it might not print a newline even if it _is_asked for. 
==========================================================================*/
void program_context_printf (const ProgramContext *self,
       int flags, const char *fmt,...)
  {
  va_list ap;
  va_start (ap, fmt);
  char *s;
  vasprintf (&s, fmt, ap);
  int width = self->width;
  if (self->width == 0) // Do not format, even to stdout
    {
    printf ("%s", s);
    }
  else if (self->width == -1) // No width set
    {
    if (self->stdout_is_tty)
      wraptext_easy_stdout_utf8 (self->console_width, (const UTF8 *)s, 0); 
    else
      {
      printf ("%s", s);
      }
    }
  else // A width was set by the user -- use it, even if not console
    wraptext_easy_stdout_utf8 (width, (const UTF8 *)s, 0); 
  free (s);
  va_end (ap);
  }


