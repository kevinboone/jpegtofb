/*==========================================================================
boilerplate
log.h
Copyright (c)2020 Kevin Boone
Distributed under the terms of the GPL v3.0
==========================================================================*/

#pragma once

#include "defs.h"

#define MYLOG_ERROR 0
#define MYLOG_WARNING 1
#define MYLOG_INFO 2
#define MYLOG_DEBUG 3
#define MYLOG_TRACE 4

#define LOG_IN log_trace ("Entering %s", __PRETTY_FUNCTION__);
#define LOG_OUT log_trace ("Leaving %s", __PRETTY_FUNCTION__);

typedef void (*LogHandler)(int level, const char *message, 
          void *userdata);

BEGIN_DECLS

// Set the logging level, 0-5
void log_set_level (int level);

/** Log a message at INFO level */
void log_info (const char *fmt,...);

/** Log a message at ERROR level */
void log_error (const char *fmt,...);

/** Log a message at WARNING level */
void log_warning (const char *fmt,...);

/** Log a message at DEBUG level */
void log_debug (const char *fmt,...);

/** Log a message at TRACE level */
void log_trace (const char *fmt,...);

/** Set the overal log level to one of the KLIB_LOG_XXX values */
void log_set_level (int level);

/** Set the application-specific log handler */
void log_set_handler (LogHandler logHandler, void *userdata);

END_DECLS


