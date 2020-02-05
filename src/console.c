/*==========================================================================

  boilerplate
  console.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

  Various terminal manipulation and interrogation functions. Functions
  to move the cursor, etc., could be added here if required.

  console is not a 'class', in the broadest sense -- it does not need
  to be initialized or destroyed. This file is just a collection
  general-purpose terminal-handling functions.

==========================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "defs.h" 
#include "log.h" 
#include "console.h" 

/*==========================================================================
  console_get_width 
==========================================================================*/
int console_get_width (void)
  {
  LOG_IN
  int width = -1;
  if (isatty (STDOUT_FILENO))
    {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
      {
      width = ws.ws_col;
      }
    }

  if (width < 0) 
    {
    if (isatty (STDIN_FILENO))
      {
      struct winsize ws;
      if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == 0)
        {
        width = ws.ws_col;
        }
      }
    }

  LOG_OUT
  return width;
  }


/*==========================================================================
  console_write_attribute
==========================================================================*/
void console_write_attribute (ConsoleAttr attr, BOOL force)
  {
#ifdef FEATURE_ANSI_TERMINAL
  LOG_IN
  if (force || isatty (STDOUT_FILENO))
    {
    char s[20];
    sprintf (s, "\x1B[%dm", (int)attr);
    fputs (s, stdout);
    }
#endif
  LOG_OUT
  }


/*==========================================================================
  console_fg_colour
==========================================================================*/
void console_fg_colour (ConsoleColour colour, BOOL force)
  {
#ifdef FEATURE_ANSI_TERMINAL
  LOG_IN
  if (force || isatty (STDOUT_FILENO))
    {
    char s[20];
    sprintf (s, "\x1B[3%dm", (int)colour);
    fputs (s, stdout);
    }
#endif
  LOG_OUT
  }


/*==========================================================================
  console_bg_colour
==========================================================================*/
void console_bg_colour (ConsoleColour colour, BOOL force)
  {
#ifdef FEATURE_ANSI_TERMINAL
  LOG_IN
  if (force || isatty (STDOUT_FILENO))
    {
    char s[20];
    sprintf (s, "\x1B[4%dm", (int)colour);
    fputs (s, stdout);
    }
#endif
  LOG_OUT
  }


/*==========================================================================
  console_reset
==========================================================================*/
void console_reset (void)
  {
  console_write_attribute (CA_NORMAL, FALSE);
  console_fg_colour (CC_DEFAULT, FALSE);
  console_bg_colour (CC_DEFAULT, FALSE);
  }


/*==========================================================================
  console_read_without_echo
  Read, e.g., a password from the console, without echoing characters
==========================================================================*/
void console_read_without_echo (char *password, int len)
  {
  struct termios oflags, nflags;

  tcgetattr(fileno(stdin), &oflags);
  nflags = oflags;
  nflags.c_lflag &= ~ECHO;
  tcsetattr(fileno(stdin), TCSANOW, &nflags);

  fgets (password, len, stdin);

  if (strlen (password) > 0)
    password [strlen(password) - 1] = 0; // remove \n

  tcsetattr(fileno(stdin), TCSANOW, &oflags);
  // Because the user's \n will not have been echoed, we need to
  //   print a newline to replace it, otherwise the screen will
  //   look odd
  printf ("\n");
  }


