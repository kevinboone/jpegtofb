/*============================================================================
  boilerplate 
  program_context.h 
  Copyright (c)2020 Kevin Boone, GPL v3.0
============================================================================*/

#pragma once

#include "defs.h"
#include "log.h" 

struct _ProgramContext;
typedef struct _ProgramContext ProgramContext;

BEGIN_DECLS

void program_context_destroy (ProgramContext *self);
ProgramContext *program_context_create (void);
void program_context_read_rc_files (ProgramContext *self, 
  const char *rc_filename);
void program_context_read_system_rc_file (ProgramContext *self, 
       const char *rc_filename);
void program_context_read_user_rc_file (ProgramContext *self, 
       const char *rc_filename);
void program_context_put (ProgramContext *self, const char *name, 
    const char *value);
const char *program_context_get (const ProgramContext *self, const char *key);
void program_context_put_boolean (ProgramContext *self, 
    const char *key, BOOL value);
void program_context_put_integer (ProgramContext *self, 
    const char *key, int value);
void program_context_put_int64 (ProgramContext *self, 
    const char *key, int64_t value);
BOOL program_context_get_boolean (const ProgramContext *self, 
    const char *key, BOOL deflt);
int  program_context_get_integer (const ProgramContext *self, 
    const char *key, int deflt);
int64_t program_context_get_int64 (const ProgramContext *self, 
    const char *key, int64_t deflt);
BOOL program_context_parse_command_line (ProgramContext *self, 
     int argc, char **argv);
int program_context_get_nonswitch_argc (const ProgramContext *self);
char ** const program_context_get_nonswitch_argv (const ProgramContext *self);
BOOL program_context_is_stdout_tty (const ProgramContext *self);
int program_context_get_console_width (const ProgramContext *self);
void program_context_query_console (ProgramContext *self);
void program_context_setup_logging 
       (ProgramContext *self, LogHandler log_handler, void *userdata);
void program_context_printf (const ProgramContext *self,
       int flags, const char *fmt,...);

END_DECLS


