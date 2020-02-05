/*==========================================================================
  
  boilerplate

  console.h
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

==========================================================================*/

#pragma once

#include "feature.h"

BEGIN_DECLS

typedef enum 
  {
  CA_NORMAL = 0,
  CA_BRIGHT = 1,
  CA_DIM = 2,
  CA_STANDOUT = 3,
  CA_UNDERSCORE = 4,
  CA_BLINK = 5,
  CA_REVERSE = 7
  } ConsoleAttr;

typedef enum 
  {
  CC_BLACK = 0,
  CC_RED = 1,
  CC_GREEN = 2,
  CC_YELLOW = 3,
  CC_BLUE = 4,
  CC_MAGENTA = 5, 
  CC_CYAN = 6,
  CC_WHITE = 7, 
  CC_DEFAULT = 9
  } ConsoleColour;


// Get width of console, if possible. Being able to do so does not
//   guarantee that stdout or stdin are a TTY -- use isatty() for that
int console_get_width (void);

void console_write_attribute (ConsoleAttr attr, BOOL force);
void console_fg_colour (ConsoleColour colour, BOOL force);
void console_bg_colour (ConsoleColour colour, BOOL force);
void console_reset (void);
void console_read_without_echo (char *password, int len);

END_DECLS

