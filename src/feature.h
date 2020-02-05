/*==========================================================================
  
  boilerplate

  features.h
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

  Define which features are included

==========================================================================*/

#pragma once

// Enable ANSI terminal features like bold, underline. If not defined the
//   relevant functions still exist, but have no effect
#define FEATURE_ANSI_TERMINAL 1

// If defined, program_context_read_rc_files reads a system file
//   from /etc
#define FEATURE_SYSTEM_RC 1

// If defined, program_context_read_rc_files reads a user file
//   from $HOME
#define FEATURE_USER_RC 1


