/*++
Copyright (c) 2014 Michael Bikovitsky

Module Name:

	frotz_interface.c

Abstract: 

	Contains implementations of the Frotz interface
	functions.

Environment:

	Kernel mode only

--*/

//
// Headers
//

#include <ntddk.h>
#include <windef.h>

#include <stdarg.h>

#include "..\..\common\frotz.h"
#include "memfile.h"


//
// Globals
//

f_setup_t f_setup = {0};

extern BYTE   StoryData[];
extern SIZE_T StorySize;

VOID dump_screen(VOID);


//
// Functions
//

bool
os_repaint_window(
	int win,
	int ypos_old,
	int ypos_new,
	int xpos,
	int ysize,
	int xsize
)
{
	UNREFERENCED_PARAMETER(win);
	UNREFERENCED_PARAMETER(ypos_old);
	UNREFERENCED_PARAMETER(ypos_new);
	UNREFERENCED_PARAMETER(xpos);
	UNREFERENCED_PARAMETER(ysize);
	UNREFERENCED_PARAMETER(xsize);
	
	return FALSE;
}

VOID
os_fatal(
	PCSTR pszFormat,
	...
)
{
	va_list vaArgs = NULL;
	
	va_start(vaArgs, pszFormat);
	vDbgPrintEx(
		DPFLTR_DEFAULT_ID,
		DPFLTR_ERROR_LEVEL,
		pszFormat,
		vaArgs);
	va_end(vaArgs);

	//
	// Carpe diem.
	//
	ExRaiseStatus(STATUS_UNSUCCESSFUL);
}

INT
os_random_seed(VOID)
{
	//
	// Chosen by fair random.randrange(2147483647).
	// Guaranteed to be random.
	// xkcd.com/221
	//
	return 2137389909;
}

PSTR
os_read_file_name(
	PCSTR pszDefaultName,
	INT   iFlag
)
{
	UNREFERENCED_PARAMETER(pszDefaultName);
	UNREFERENCED_PARAMETER(iFlag);
	
	DbgBreakPoint();
	return NULL;
}

zchar
os_read_key(
	INT iTimeout,
	INT iShowCursor
)
{
	zchar acCharacter[2] = {'\0'};

	UNREFERENCED_PARAMETER(iTimeout);
	UNREFERENCED_PARAMETER(iShowCursor);

	dump_screen();

	(VOID)DbgPrompt("", acCharacter, 2);

	return acCharacter[0];
}

zchar
os_read_line(
	INT     cchMax,
	zchar * pszBuffer,
	INT     iTimeout,
	INT     iWidth,
	INT     iContinued
)
{
	UNREFERENCED_PARAMETER(iTimeout);
	UNREFERENCED_PARAMETER(iWidth);
	UNREFERENCED_PARAMETER(iContinued);
	
	dump_screen();
	(VOID)DbgPrompt("", pszBuffer, cchMax);
	return ZC_RETURN;
}

VOID
os_init_setup(VOID)
{
	f_setup.format = FORMAT_DISABLED;
}

FILE *
os_load_story(VOID)
{
	return MemCreateFromBuffer(StoryData, StorySize);
}

int
os_storyfile_seek(
	FILE *	hFile,
	long	cbOffset, 
	int		eWhence
)
{
	return MemSeek(hFile, cbOffset, eWhence);
}

int
os_storyfile_tell(
	FILE *	hFile
)
{
	return MemTell(hFile);
}
