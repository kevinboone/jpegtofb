/*============================================================================
 
  boilerplate
  file.c
  Copyright (c)2020 Kevin Boone, GPL v3.0

  Various file-handling helper functions

  This file is not a 'class' in the broadest sense, just an assortment of
  functions. Some of these are wrapped by the path class, which provides
  a class representation of a pathname

============================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <regex.h>
#include "defs.h" 
#include "file.h" 
#include "log.h" 
#include "list.h" 
#include "string.h" 
#include "path.h" 
#include "buffer.h" 
#include "string.h" 


/*==========================================================================
  file_readline 
  Read a line from a file, allocating a buffer to store it. This function
    will probably only work with UTF8/ASCII files
  Returns the number of bytes read, or zero if end of file, or
    -1 if error
*==========================================================================*/
int file_readline (FILE *f, char **buffer)
  {
  int ch;
  int ret = -1;
  size_t buflen = 0, nchar = 64;
  int count = 0;

  LOG_IN

  *buffer = malloc (nchar);    

  while ((ch = fgetc (f)) != '\n' && ch != EOF) 
    {
    (*buffer)[buflen++] = ch;
    count++;

    if (buflen + 1 >= nchar) 
      {  
      char *tmp = realloc (*buffer, nchar * 2);
      *buffer = tmp;
      nchar *= 2;
      }
    }

  (*buffer)[buflen] = 0;

  if (buflen == 0 && ch == EOF) 
    {
    free (*buffer);
    ret = 0; 
    }
  else
    ret = count;

  return ret;
  LOG_OUT
  }


/*==========================================================================
  file_get_size
  Gets the size of a file, if possible. In failure, returns -1 and
    errno will be set. If the filename is a symlink, then the result
    is the size of the file, not the link
*==========================================================================*/
int64_t file_get_size (const char *filename)
  {
  LOG_IN
  int ret = -1;
  struct stat sb;
  if (stat (filename, &sb) == 0)
    {
    ret = sb.st_size;
    }
  LOG_OUT
  return ret;
  }


/*==========================================================================
  file_get_mtime
  Gets the mtime of a file, if possible. In failure, returns -1 and
    errno will be set. If the filename is a symlink, then the result
    applies to the file, not the link
*==========================================================================*/
time_t file_get_mtime (const char *filename)
  {
  LOG_IN
  time_t ret = -1;
  struct stat sb;
  if (stat (filename, &sb) == 0)
    {
    ret = sb.st_mtime;
    }
  LOG_OUT
  return ret;
  }

/*==========================================================================
  file_exists
  Returns TRUE if the file exists, in the most rudimentary sense -- 
    a stat() call succeeds.
*==========================================================================*/
BOOL file_exists (const char *filename)
  {
  LOG_IN
  BOOL ret = FALSE;
  struct stat sb;
  if (stat (filename, &sb) == 0)
    {
    ret = TRUE; 
    }
  LOG_OUT
  return ret;
  }

/*==========================================================================
  file_is_regular
  Return TRUE if the file exists, and is a regular file. 
    A non-existent file is, by definition, not regular 
*==========================================================================*/
BOOL file_is_regular (const char *filename)
  {
  LOG_IN
  BOOL ret = FALSE;
  struct stat sb;
  if (stat (filename, &sb) == 0)
    {
    ret = S_ISREG (sb.st_mode); 
    }
  LOG_OUT
  return ret;
  }

/*==========================================================================
  file_is_directory
  Return TRUE if the file exists, and is a directory. A non-existent file
    is, by definition, not a directory
*==========================================================================*/
BOOL file_is_directory (const char *filename)
  {
  LOG_IN
  BOOL ret = FALSE;
  struct stat sb;
  if (stat (filename, &sb) == 0)
    {
    ret = S_ISDIR (sb.st_mode); 
    }
  LOG_OUT
  return ret;
  }


/*==========================================================================
  file_expand_directory

  Expands a directory into a list of Strings representing its contents.
  The strings can be simple names or, optionally, paths -- relative
  or absolute.
  
  Various FE_XXX constants set the entities that will be included, and
  how they are to be interpreted.

  "." and ".." are never included in the results, even with FE_ALL

  This function will always return TRUE and allocate a list if the directory
  can be opened for reading. However, there is no guarantee that the 
  directory contains any files, and the resulting list might be empty.
  Nevertheless, the caller will have to free it. 

  This function always dereferences links. There is no flag to set that
  links are included, because a link to an X has the same type as X. 
  That is, a link to a file is a file.

  The list (of Strings) is allocated by this method, and must be
  destroyed if it is non-null on return.

  See also path_expand_directory

*==========================================================================*/
BOOL file_expand_directory (const char *path, 
       int flags, List **names)
  {
  LOG_IN
  BOOL ret = FALSE;
  log_debug ("expand_directory_name: %s", path);

  DIR *d = opendir (path);
  if (d)
    {
    // This realpath() call should always succeed, since the path
    //   must exist for us to have gotten this far
    char *start_fullpath = realpath (path, NULL);

    List *list = list_create ((ListItemFreeFn)string_destroy);
    struct dirent *de;
    while ((de = readdir (d)))
      {
      BOOL include = TRUE;
      const char *name = de->d_name;
      if (strcmp (name, ".") == 0 || strcmp (name, "..") == 0)
        include = FALSE;
      if (include && name[0] == '.' && !(flags & FE_HIDDEN))
        include = FALSE;

      Path *p = path_create (path);
      path_append (p, name);
      // int_path is something that actually exists, and can be
      //   operated on, where name is just a filename
      char *int_path = (char *)path_to_utf8 (p);
      path_destroy (p);

      if (include)
        {
        struct stat sb;
        stat (int_path, &sb);

        if (S_ISREG (sb.st_mode) && (flags & FE_FILES))
          {
          include = TRUE;
          }
        else if (S_ISDIR (sb.st_mode) && (flags & FE_DIRS))
          {
          include = TRUE;
          }
        else if (flags & FE_ALL)
          {
          include = TRUE;
          }
        else
          include = FALSE;
         }

      if (include)
        {
	char *fullpath = NULL;
	if (flags & FE_PREPEND_FULL_PATH)
	  {
	  Path *p = path_create (start_fullpath);
	  path_append (p, name);
	  fullpath = (char *) path_to_utf8 (p); 
	  path_destroy (p);
	  }
	else if (flags & FE_PREPEND_PATH)
	  {
	  fullpath = strdup (int_path); 
	  }
	else
	  {
	  fullpath = strdup (name);
	  }
	if (fullpath)
	  {
          list_append (list, string_create (fullpath));
	  free (fullpath);
	  }
	}
      free (int_path);
      }
    *names = list;
    free (start_fullpath);
    closedir (d);
    ret = TRUE;
    }
  else
    log_debug ("expand_directory_names failed");
  LOG_OUT
  return ret;
  }


/*==========================================================================
  file_read_to_buffer
  Return TRUE if the file exists, and XXX 
*==========================================================================*/
BOOL file_read_to_buffer (const char *filename, Buffer **buffer)
  {
  LOG_IN
  BOOL ret = FALSE;

  log_debug ("file_read_to_buffer: %s", filename);
  int64_t size = file_get_size (filename);
  if (size != -1)
    {
    int f = open (filename, O_RDONLY);
    if (f)
      {
      log_debug ("file opened");
      BYTE *buff = malloc (size);
      if (buff)
        {
        if (read (f, buff, size) == size)
          {
          *buffer = buffer_create_without_copy (buff, size); 
          ret = TRUE;
          }
        else
          free (buff);
        }
      else
        errno = ENOMEM; // Never happens on Linux
      close (f);
      }
    else
      {
      log_debug ("can't open file for reading: %s: %s", filename,
        strerror (errno));
      }

    }

  LOG_OUT
  return ret;
  }


/*==========================================================================

  file_write_from_string

  Writes a buffer to a file. If it exists, the file is replaced without
    warning

*==========================================================================*/
BOOL file_write_from_string (const char *filename, const String *string)
  {
  LOG_IN
  BOOL ret = FALSE;

  log_debug ("file_write_from_string: %s", filename);
  int f = open (filename, O_WRONLY | O_CREAT | O_TRUNC, 0770);
  if (f)
    {
    log_debug ("file opened");
    if (write (f, string_cstr(string), 
         string_length (string)) == string_length (string))
      ret = TRUE;
    close (f);
    }
  else
    {
    log_debug ("can't open file for writing: %s: %s", filename,
      strerror (errno));
    }

  LOG_OUT
  return ret;
  }


/*==========================================================================

  file_write_from_buffer

  Writes a buffer to a file. If it exists, the file is replaced without
    warning

*==========================================================================*/
BOOL file_write_from_buffer (const char *filename, const Buffer *buffer)
  {
  LOG_IN
  BOOL ret = FALSE;

  log_debug ("file_write_from_buffer: %s", filename);
  int f = open (filename, O_WRONLY | O_CREAT | O_TRUNC, 0770);
  if (f)
    {
    log_debug ("file opened");
    if (write (f, buffer_get_contents (buffer), 
         buffer_get_length (buffer)) == buffer_get_length (buffer))
      ret = TRUE;
    close (f);
    }
  else
    {
    log_debug ("can't open file for writing: %s: %s", filename,
      strerror (errno));
    }

  LOG_OUT
  return ret;
  }

/*==========================================================================

  file_glob_to_regex

*==========================================================================*/
char *file_glob_to_regex (const char *glob)
  {
  LOG_IN
  String *s = string_create (glob);
  String *s1 = string_substitute_all (s, ".", "\\.");
  String *s2 = string_substitute_all (s1, "?", ".");
  String *s3 = string_substitute_all (s2, "*", ".*");

  string_append (s3, "$");
  string_prepend (s3, "^");
  char *ret = strdup (string_cstr (s3));

  string_destroy (s3); 
  string_destroy (s2); 
  string_destroy (s1); 
  string_destroy (s);
  LOG_OUT
  return ret;
  }

/*==========================================================================

  file_name_matches_pattern_case

  Test the filename against the glob pattern, with an option to set 
    whether the test should be sensitive or insensitive to letter
    case.

*==========================================================================*/
BOOL file_name_matches_pattern_case (const char *name, const char *pattern,
    BOOL insensitive)
  {
  LOG_IN
  BOOL ret = FALSE;
  
  char *regex = file_glob_to_regex (pattern);
  regex_t preg;
  int flags = REG_NOSUB;
  if (insensitive) flags |= REG_ICASE;
  if (regcomp (&preg, regex, flags) == 0)
    {
    if (regexec (&preg, name, 0, NULL, 0) == 0)
      ret = TRUE;
    }
  else
    {
    log_debug ("Failed to compile regex %s", pattern);
    }
  regfree (&preg); 
  free (regex);

  LOG_OUT
  return ret;
  }

/*==========================================================================

  file_name_matches_pattern

  Test the filename against the glob pattern, paying attention to
    case sensitivity

*==========================================================================*/
BOOL file_name_matches_pattern (const char *name, const char *pattern)
  {
  LOG_IN
  BOOL ret = file_name_matches_pattern_case (name, pattern, FALSE);
  LOG_OUT
  return ret;
  }

