/*==========================================================================
boilerplate
defs.h
Copyright (c)2020 Kevin Boone
Distributed under the terms of the GPL v3.0
==========================================================================*/

#pragma once

// Boolean

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE 
#define FALSE 0
#endif

#ifndef BOOL
typedef int BOOL;
#endif

// Byte

#ifndef BYTE
typedef unsigned char BYTE;
#endif

// File and path sizes

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

// Character sizes

#ifndef UTF8
typedef unsigned char UTF8;
#endif

#ifndef UTF32
typedef int32_t UTF32;
#endif


#ifdef __cplusplus
#define BEGIN_DECLS exetern "C" { 
#define END_DECLS }
#else
#define BEGIN_DECLS 
#define END_DECLS 
#endif



