/*++
Copyright (c) 2014 Michael Bikovitsky

Module Name:

	file.c

Abstract: 

	Contains implementation of C file I/O functions
	over memory buffers.

	Currently DOES NOT support text mode.

Environment:

	Kernel mode only

--*/

//
// Headers
//

#include <wdm.h>
#include <stdio.h>

#include "..\memmgr\memmgr.h"


//
// Replacement for the C FILE structure
//

typedef struct _MEM_FILE {

    PCHAR  pcBuffer;
	PCHAR  pcCurrentPos;
	SIZE_T cbBuffer;
    
} MEM_FILE, *PMEM_FILE;


//
// Functions
//

size_t
__cdecl 
MemRead(
	__out_bcount(ElementSize*Count) void   * Destination,
	__in                            size_t   ElementSize,
	__in                            size_t   Count,
	__inout                         FILE   * File
)
/*++

Routine Description:

	fread implementation over a MEM_FILE.
    
Arguments:

	Destination - Storage location for data.

	ElementSize - Item size in bytes.

	Count - Maximum number of items to be read.

	File - Pointer to a MEM_FILE.

Return Value:

	size_t (Number of full items actually read)

--*/
{
	PMEM_FILE ptMemoryFile = (PMEM_FILE)File;
	SIZE_T    cbToRead     = 0;

	if (ptMemoryFile->pcCurrentPos + (ElementSize * Count) >= ptMemoryFile->pcBuffer + ptMemoryFile->cbBuffer)
	{
		cbToRead = (ptMemoryFile->pcBuffer + ptMemoryFile->cbBuffer - ptMemoryFile->pcCurrentPos);
		cbToRead = (cbToRead / ElementSize) * ElementSize;
	}
	else
	{
		cbToRead = ElementSize * Count;
	}

	RtlCopyMemory(Destination, ptMemoryFile->pcCurrentPos, cbToRead);
	ptMemoryFile->pcCurrentPos += cbToRead;

	return cbToRead / ElementSize;
}

int
__cdecl
MemGetC(
	__inout FILE * File
)
/*++

Routine Description:

	fgetc implementation over a MEM_FILE.
    
Arguments:

	File - Pointer to a MEM_FILE.

Return Value:

	int (Char read or EOF)

--*/
{
	PMEM_FILE ptMemoryFile = (PMEM_FILE)File;

	if (ptMemoryFile->pcCurrentPos == ptMemoryFile->pcBuffer + ptMemoryFile->cbBuffer)
	{
		return EOF;
	}

	return (int)((ptMemoryFile->pcCurrentPos)++);
}

int
__cdecl
MemPutC(
	__in    int    Char,
	__inout FILE * File
)
/*++

Routine Description:

	fputc implementation over a MEM_FILE.
    
Arguments:

	Char - Character to write.

	File - Pointer to a MEM_FILE.

Return Value:

	int (Char written or EOF)

--*/
{
	PMEM_FILE ptMemoryFile = (PMEM_FILE)File;

	if (ptMemoryFile->pcCurrentPos == ptMemoryFile->pcBuffer + ptMemoryFile->cbBuffer)
	{
		return EOF;
	}

	*(ptMemoryFile->pcCurrentPos) = (CHAR)Char;
	return Char;
}

long
__cdecl
MemTell(
	__inout FILE * File
)
/*++

Routine Description:

	ftell implementation over a MEM_FILE.
    
Arguments:

	File - Pointer to a MEM_FILE.

Return Value:

	long

--*/
{
	PMEM_FILE ptMemoryFile = (PMEM_FILE)File;

	return ptMemoryFile->pcCurrentPos - ptMemoryFile->pcBuffer;
}

int
__cdecl
MemSeek(
	__inout FILE * File,
	__in    long   Offset,
	__in    int    Origin
)
/*++

Routine Description:

	fseek implementation over a MEM_FILE.
    
Arguments:

	File - Pointer to a MEM_FILE.

	Offset - Number of bytes from origin.

	Origin - Initial position.

Return Value:

	int

--*/
{
	PMEM_FILE ptMemoryFile = (PMEM_FILE)File;

	switch (Origin)
	{
	case SEEK_CUR:
		ptMemoryFile->pcCurrentPos += Offset;
		break;

	case SEEK_END:
		ptMemoryFile->pcCurrentPos = ptMemoryFile->pcBuffer + ptMemoryFile->cbBuffer - 1 + Offset;
		break;

	case SEEK_SET:
		ptMemoryFile->pcCurrentPos = ptMemoryFile->pcBuffer + Offset;
		break;

	default:
		return -1;
	}

	if (ptMemoryFile->pcCurrentPos > ptMemoryFile->pcBuffer + ptMemoryFile->cbBuffer)
	{
		ptMemoryFile->pcCurrentPos = ptMemoryFile->pcBuffer + ptMemoryFile->cbBuffer;
	}
	else if (ptMemoryFile->pcCurrentPos < ptMemoryFile->pcBuffer)
	{
		ptMemoryFile->pcCurrentPos = ptMemoryFile->pcBuffer;
	}

	return 0;
}

int
__cdecl
MemClose(
	__inout FILE * File
)
/*++

Routine Description:

	fclose implementation over a MEM_FILE.
    
Arguments:

	File - Pointer to a MEM_FILE.

Return Value:

	int

--*/
{
	memmgr_free(File);
	
	return 0;
}

int
__cdecl
MemError(
	__in FILE * File
)
/*++

Routine Description:

	ferror implementation over a MEM_FILE.
    
Arguments:

	File - Pointer to a MEM_FILE.

Return Value:

	int

--*/
{
	//
	// Error free :)
	//
	return 0;
}

size_t
__cdecl
MemWrite(
	__in_ecount(Size*Count) const void * Buffer,
	__in                    size_t       Size,
	__in                    size_t       Count,
	__inout                 FILE       * File
)
{
	PMEM_FILE ptMemoryFile = (PMEM_FILE)File;
	SIZE_T    cbToWrite     = 0;

	if (ptMemoryFile->pcCurrentPos + (Size * Count) >= ptMemoryFile->pcBuffer + ptMemoryFile->cbBuffer)
	{
		cbToWrite = (ptMemoryFile->pcBuffer + ptMemoryFile->cbBuffer - ptMemoryFile->pcCurrentPos);
		cbToWrite = (cbToWrite / Size) * Size;
	}
	else
	{
		cbToWrite = Size * Count;
	}

	RtlCopyMemory(ptMemoryFile->pcCurrentPos, Buffer, cbToWrite);
	ptMemoryFile->pcCurrentPos += cbToWrite;

	return cbToWrite / Size;
}

int
__cdecl
MemUngetC(
	__in    int    Char,
	__inout FILE * File
)
/*++

Routine Description:

	ungetc stub.
    
Arguments:

	Char - Character to be pushed.

	File - Pointer to a MEM_FILE.

Return Value:

	int (Always EOF)

--*/
{
	UNREFERENCED_PARAMETER(Char);
	UNREFERENCED_PARAMETER(File);

	return EOF;
}
