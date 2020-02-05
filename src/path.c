/*============================================================================
 
  boilerplate 
  path.c
  Copyright (c)2017 Kevin Boone, GPL v3.0

  Methods to manipulate pathnames. Path is effectively a subclass of
  WString, and all WString's methods can be used on Path, with 
  appropriate casts. 

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
#include <sys/wait.h>
#include "wstring.h" 
#include "string.h" 
#include "defs.h" 
#include "log.h" 
#include "path.h" 

struct _Path
  {
  UTF32 *str;
  }; 


/*==========================================================================
  path_create_empty 
*==========================================================================*/
Path *path_create_empty (void)
  {
  Path *self = malloc (sizeof (Path));
  self->str = malloc (1 * sizeof (UTF32));
  self->str[0] = 0;
  return self;
  }


/*==========================================================================
  path_create
*==========================================================================*/
Path *path_create (const char *s)
  {
  Path *self = malloc (sizeof (Path));
  self->str = string_utf8_to_utf32 ((UTF8 *)s);
  return self;
  }


/*==========================================================================
  path_clone
*==========================================================================*/
Path *path_clone (const Path *p)
  {
  return (Path *)wstring_clone ((WString *) p);
  }


/*==========================================================================
  path_create
*==========================================================================*/
Path *path_create_home (void)
  {
  char *home = getenv ("HOME");
  if (home)
    return path_create (home);
  else
    return path_create ("/");
  }

/*==========================================================================
  path_destroy
*==========================================================================*/
void path_destroy (Path *self)
  {
  if (self)
    {
    if (self->str) free (self->str);
    free (self);
    }
  }


/*==========================================================================
  path_cstr
*==========================================================================*/
const UTF32 *path_cstr (const Path *self)
  {
  return self->str;
  }


/*==========================================================================
  path_length
*==========================================================================*/
int path_length (const Path *self)
  {
  UTF32 *s = self->str;
  int i = 0;
  UTF32 c = 0;
  do
    {
    c = s[i];
    i++;
    } while (c != 0);
  int ret = i - 1;
  return ret;
  }


/*==========================================================================
  path_separator
*==========================================================================*/
const UTF32 *path_separator (void)
  {
  static UTF32 sep[2] = {'/', 0};
  return sep;
  }


/*==========================================================================
  path_fwd_slash
*==========================================================================*/
const UTF32 *path_fwd_slash (void)
  {
  static UTF32 sep[2] = {'/', 0};
  return sep;
  }


/*==========================================================================
  path_append
  Append a name to this path, adding a separator if there is not already one
  present
*==========================================================================*/
void path_append (Path *self, const char *name)
  {
  // Empty string is a special case -- append without a separator
  // This is conventionally a relative file
  if (wstring_length ((WString *)self) == 0)
    {
    wstring_append_printf ((WString *)self, "%s", name);
    }
  else
    {
    if (!wstring_ends_with ((WString *)self, path_separator()))
      {
      wstring_append ((WString *)self, path_separator());
      }
    wstring_append_printf ((WString *)self, "%s", name);
    }
  }


/*==========================================================================
  path_to_utf8
*==========================================================================*/
UTF8 *path_to_utf8 (const Path *self)
  {
  return wstring_to_utf8 ((WString *)self);
  }


/*==========================================================================
  path_expand_directory

  See the notes for file_expand_directory() for more information

*==========================================================================*/
BOOL path_expand_directory (const Path *self, 
                    int flags, List **names)
  {
  BOOL ret = FALSE;
  LOG_IN
  UTF8 *s = path_to_utf8 (self);
  ret = file_expand_directory ((char *)s, flags, names);
  free (s);
  LOG_OUT
  return ret;
  }


/*==========================================================================

  path_ends_with_separator

*==========================================================================*/
BOOL path_ends_with_separator (const Path *self)
  {
  return wstring_ends_with ((WString *)self, path_separator());
  }

/*==========================================================================

  path_ends_with_fwd_slash

*==========================================================================*/
BOOL path_ends_with_fwd_slash (const Path *self)
  {
  return wstring_ends_with ((WString *)self, path_fwd_slash());
  }


/*==========================================================================

  path_create_directory
  ... and all the parent directories

  This is a bit nasty -- I haven't had time to code all the logic
  myself, so this method just invokes "mkdir -p". This is a reasonable
  this to do on Unix-like systems, but perhaps not elsewhere. A problem
  with working this way is it is a little awkward to tell when the
  directory creation has failed.

*==========================================================================*/
BOOL path_create_directory (const Path *self)
  {
  LOG_IN
  BOOL ret = FALSE;

  char *s_path = (char *)path_to_utf8 (self);
  
  pid_t pid = fork();

  if (pid != -1)
    {
    if (pid > 0)
      {
      int status;
      waitpid (pid, &status, 0);
      if (status == 0) ret = TRUE;
      errno = status;
      }
    else 
      {
      // we are the child
      execlp ("mkdir", "mkdir", "-p", s_path, NULL);
      _exit (EXIT_FAILURE);   // exec never returns
      }
    } 
  // else fork() failed

  free (s_path);
  LOG_OUT
  return ret;
  }


/*==========================================================================

  path_get_filename_utf8

*==========================================================================*/

UTF8 *path_get_filename_utf8 (const Path *path)
  {
  LOG_IN
  Path *fpath = path_clone (path);
  path_remove_directory (fpath); // note that this might give empty path
  UTF8 *s = path_to_utf8 (fpath);
  path_destroy (fpath);
  LOG_OUT
  return s;
  }


/*==========================================================================

  path_remove_directory

  Remove the directory path of a path, if there is one. This method 
  specifically does not require the path to exist -- it works entirely
  on the name pattern. This makes it possible to use in cases where
  we need to create a file and a directory for it to go it, but it
  means that there are certain ambiguous cases.

  Any path that ends in a separator is assumed to be a directory, and
  so is completely emptied. Otherwise the last component of the
  pathname is taken to be the filename whether it is, in fact, a
  regular file or not.

*==========================================================================*/
void path_remove_directory (Path *self)
  {
  LOG_IN
  if (path_ends_with_separator (self))
    {
    // Assume there is no filename -- leave an empty path
    free (self->str);
    self->str = malloc (1);
    self->str[0] = 0;
    }
  else
    {
    int p = wstring_find_last ((const WString *)self, path_separator());
    if (p >= 0)
      {
      int delete_to = p;
      wstring_delete ((WString *)self, 0, delete_to + 1);
      }
    else
      {
      // No separator. Assume this is a filename, and leave alone
      }
    }
  LOG_OUT
  }

/*==========================================================================

  path_remove_filename

  Remove the filename path of a path, if there is one. This method 
  specifically does not require the path to exist -- it works entirely
  on the name pattern. This makes it possible to use in cases where
  we need to create a file and a directory for it to go it, but it
  means that there are certain ambiguous cases.

  Any path that ends in a separator is assumed to be a directory, and
  is not altered.

*==========================================================================*/
void path_remove_filename (Path *self)
  {
  LOG_IN
  if (path_ends_with_separator (self))
    {
    // Assume there is already no filename
    }
  else
    {
    int p = wstring_find_last ((const WString *)self, path_separator());
    if (p >= 0)
      {
      int delete_from = p + 1;
      int to_delete = path_length (self) - delete_from;
      wstring_delete ((WString *)self, delete_from, to_delete);
      }
    else
      {
      // No separator. This should never really happen, but if the path is
      //   simply 'foo', there's no way to know (if it isn't actually 
      //   a file that already exists) whether it's a filename or a 
      //   directory. So do nothing :/
      }
    }
  LOG_OUT
  }

/*==========================================================================

  path_read_to_buffer

*==========================================================================*/
BOOL path_read_to_buffer (const Path *self, Buffer **buffer)
  {
  LOG_IN
  BOOL ret = FALSE;
  UTF8 *s = path_to_utf8 (self);
  ret = file_read_to_buffer ((char *)s, buffer);
  free (s);
  LOG_OUT
  return ret;
  }
 

/*==========================================================================

  path_fopen

*==========================================================================*/
FILE *path_fopen (const Path *self, const char *mode)
  {
  LOG_IN
  FILE *ret = NULL;
  UTF8 *s = path_to_utf8 (self);
  ret = fopen ((char *)s, mode);
  free (s);
  LOG_OUT
  return ret;
  }
 

/*==========================================================================

  path_write_from_buffer

*==========================================================================*/
BOOL path_write_from_buffer (const Path *self, const Buffer *buffer)
  {
  LOG_IN
  BOOL ret = FALSE;
  UTF8 *s = path_to_utf8 (self);
  ret = file_write_from_buffer ((char *)s, buffer);
  free (s);
  LOG_OUT
  return ret;
  }


/*==========================================================================

  path_write_from_string

*==========================================================================*/
BOOL path_write_from_string (const Path *self, const String *ss)
  {
  LOG_IN
  BOOL ret = FALSE;
  UTF8 *s = path_to_utf8 (self);
  ret = file_write_from_string ((char *)s, ss);
  free (s);
  LOG_OUT
  return ret;
  }


/*==========================================================================

  path_is_regular

  Returns TRUE if the path corresponds to a regular file -- and we have
    access rights to find out. Returns FALSE if the file does not exist
    or mode cannot be dermined, as well as if it is not regular

*==========================================================================*/
BOOL path_is_regular (const Path *self)
  {
  LOG_IN
  BOOL ret = FALSE;
  UTF8 *s = path_to_utf8 (self);
  ret = file_is_regular ((const char *)s);
  free (s);
  LOG_OUT
  return ret;
  }

/*==========================================================================

  path_is_directory

  Returns TRUE if the path corresponds to a directory -- and we have
    access rights to find out. Returns FALSE if the file does not exist
    or mode cannot be dermined, as well as if it is not regular

*==========================================================================*/
BOOL path_is_directory (const Path *self)
  {
  LOG_IN
  BOOL ret = FALSE;
  UTF8 *s = path_to_utf8 (self);
  ret = file_is_directory ((const char *)s);
  free (s);
  LOG_OUT
  return ret;
  }


/*==========================================================================

  path_stat

  Simple wrapper for the stat() function. Returns TRUE is the stat call
    succeeds. If it returns FALSE, errno will be set.

*==========================================================================*/
BOOL path_stat (const Path *self, struct stat *sb)
  {
  LOG_IN
  UTF8 *s = path_to_utf8 (self);
  int err = stat ((const char *)s, sb);
  free (s);
  LOG_OUT
  return (err==0);
  }




