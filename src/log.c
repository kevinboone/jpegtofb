/*==========================================================================
 
  boilerplate
  log.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

  Functions for logging at various levels. Users should probably call
  log_set_level to set the logging verbosity, and log_set_handler to
  define a function that will actually output the log messages to a
  specific place.

==========================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <stdarg.h>
#include "defs.h" 
#include "log.h" 

int log_level = MYLOG_INFO;
static LogHandler log_handler = NULL;
static void *log_userdata = NULL;

/*==========================================================================
  log_set_level
==========================================================================*/
void log_set_level (int level)
  {
  log_level = level;
  }


/*===========================================================================
log_v
============================================================================*/
static void log_v (int level, const char *fmt, va_list ap)
  {
  if (level > log_level) return;
  char *s;
  vasprintf (&s, fmt, ap);
  if (log_handler)
    log_handler (level, s, log_userdata);
  else
    fprintf (stderr, "%s\n", s);
  free (s);
  }


/*===========================================================================
log_info
============================================================================*/
void log_info (const char *fmt, ...)
  {
  va_list ap;
  va_start (ap, fmt);
  log_v (MYLOG_INFO, fmt, ap);
  va_end (ap);
  }

/*===========================================================================
log_error
============================================================================*/
void log_error (const char *fmt, ...)
  {
  va_list ap;
  va_start (ap, fmt);
  log_v (MYLOG_ERROR, fmt, ap);
  va_end (ap);
  }

/*===========================================================================
log_warning
============================================================================*/
void log_warning (const char *fmt, ...)
  {
  va_list ap;
  va_start (ap, fmt);
  log_v (MYLOG_WARNING, fmt, ap);
  va_end (ap);
  }

/*===========================================================================
log_debug
============================================================================*/
void log_debug (const char *fmt, ...)
  {
  va_list ap;
  va_start (ap, fmt);
  log_v (MYLOG_DEBUG, fmt, ap);
  va_end (ap);
  }


/*===========================================================================
log_trace
============================================================================*/
void log_trace (const char *fmt, ...)
  {
  va_list ap;
  va_start (ap, fmt);
  log_v (MYLOG_TRACE, fmt, ap);
  va_end (ap);
  }


/*===========================================================================
log_set_handler
============================================================================*/
void log_set_handler (LogHandler handler, void *userdata)
  {
  log_handler = handler;
  log_userdata = userdata;
  }

