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

#include "..\common\frotz.h"
#include "..\memfile\memfile.h"


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

INT
os_read_file_name(
	PSTR  pszFileName,
	PCSTR pszDefaultName,
	INT   iFlag
)
{
	DbgBreakPoint();
	return FALSE;
}

zchar
os_read_key(
	INT iTimeout,
	INT iShowCursor
)
{
	zchar acCharacter[2] = {'\0'};

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
	dump_screen();
	(VOID)DbgPrompt("", pszBuffer, cchMax);
	return ZC_RETURN;
}

VOID
os_init_setup(VOID)
{
	RtlSecureZeroMemory(&f_setup, sizeof(f_setup));
	f_setup.undo_slots = MAX_UNDO_SLOTS;
	f_setup.script_cols = 80;
	f_setup.save_quetzal = 1;
	f_setup.sound = 1;
	f_setup.err_report_mode = ERR_DEFAULT_REPORT_MODE;
}

FILE *
os_load_story(VOID)
{
	return MemCreateFromBuffer(StoryData, StorySize);
}
