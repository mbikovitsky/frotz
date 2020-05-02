/*++
Copyright (c) 2014 Michael Bikovitsky

Module Name:

	memfile.h

Abstract: 

	Library for C file I/O over memory buffers.

	Currently DOES NOT support text mode.

Environment:

	Kernel mode only

--*/

#pragma once

//
// Headers
//

#include <stdio.h>


//
// Prototypes
//

typedef void * (__cdecl * PFN_ALLOCATOR)(
	__in size_t Size
);

typedef void (__cdecl * PFN_DEALLOCATOR)(
	__inout_opt void * Memory
);

typedef void (__cdecl * PFN_RESOLVER)(
	__in_z      const char *  Filename,
	__in_z      const char *  Mode,
	__deref_out void       ** Buffer,
	__out       size_t     *  Size
);


//
// Initialization functions
//

void
__cdecl
MemInit(
	__in PFN_ALLOCATOR   Allocator,
	__in PFN_DEALLOCATOR Deallocator,
	__in PFN_RESOLVER    Resolver
);


//
// I/O functions
//

size_t
__cdecl 
MemRead(
	__out_bcount(ElementSize*Count) void   * Destination,
	__in                            size_t   ElementSize,
	__in                            size_t   Count,
	__inout                         FILE   * File
);

int
__cdecl
MemGetC(
	__inout FILE * File
);

int
__cdecl
MemPutC(
	__in    int    Char,
	__inout FILE * File
);

long
__cdecl
MemTell(
	__inout FILE * File
);

int
__cdecl
MemSeek(
	__inout FILE * File,
	__in    long   Offset,
	__in    int    Origin
);

int
__cdecl
MemClose(
	__inout FILE * File
);

int
__cdecl
MemError(
	__in FILE * File
);

size_t
__cdecl
MemWrite(
	__in_ecount(Size*Count) const void * Buffer,
	__in                    size_t       Size,
	__in                    size_t       Count,
	__inout                 FILE       * File
);

int
__cdecl
MemUngetC(
	__in    int    Char,
	__inout FILE * File
);

FILE *
__cdecl
MemOpen(
	__in_z const char * Filename,
	__in_z const char * Mode
);

FILE *
__cdecl
MemCreateFromBuffer(
	__in void   * Buffer,
	__in size_t   Size
);
