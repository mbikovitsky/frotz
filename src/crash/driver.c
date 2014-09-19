/*++
Copyright (c) 2014 Michael Bikovitsky

Module Name:

	driver.c

Abstract: 

	Contains driver initialization logic

Environment:

	Kernel mode only

--*/

//
// Headers
//

#include <ntddk.h>

#include "..\memfile\memfile.h"
#include "..\memmgr\memmgr.h"


//
// Globals
//

KBUGCHECK_CALLBACK_RECORD CallbackRecord     = {0};
BOOLEAN                   CallbackRegistered = FALSE;


//
// Forward declarations
//

extern int __cdecl main(int argc, char *argv[]);

DRIVER_INITIALIZE          DriverEntry;
DRIVER_UNLOAD              Unload;
KBUGCHECK_CALLBACK_ROUTINE BugCheckCallback;

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, Unload)


//
// Functions
//

NTSTATUS
DriverEntry(
	__in PDRIVER_OBJECT  DriverObject,
	__in PUNICODE_STRING RegistryPath
)
/*++

Routine Description:

	Driver entry-point.
    
Arguments:

	DriverObject - Caller-supplied pointer to a DRIVER_OBJECT structure.
		This is the driver's driver object.

	RegistryPath - Pointer to a counted Unicode string
		specifying the path to the driver's registry key.

Return Value:

	NTSTATUS

--*/
{
	UNREFERENCED_PARAMETER(RegistryPath);

	PAGED_CODE();

	DriverObject->DriverUnload = Unload;

	KeInitializeCallbackRecord(&CallbackRecord);
	CallbackRegistered = KeRegisterBugCheckCallback(
		&CallbackRecord,
		BugCheckCallback,
		NULL,
		0,
		NULL);

	return STATUS_SUCCESS;
}

VOID
Unload(
	__in PDRIVER_OBJECT DriverObject
)
/*++

Routine Description:

	Unloads the driver.
    
Arguments:

	DriverObject - Caller-supplied pointer to a DRIVER_OBJECT structure.
		This is the driver's driver object.

--*/
{
	UNREFERENCED_PARAMETER(DriverObject);

	PAGED_CODE();

	if (CallbackRegistered)
	{
		KeDeregisterBugCheckCallback(&CallbackRecord);
		CallbackRegistered = FALSE;
	}
}

VOID
BugCheckCallback(
	__in_opt PVOID Buffer,
	__in     ULONG Length
)
/*++

Routine Description:

	Bug check callback that begins the game.
    
Arguments:

	Buffer - Supplies a pointer to the buffer
		that was specified when the callback was registered.

	Length - Supplies the length, in bytes, of the buffer
		that is specified by the Buffer parameter.

--*/
{
	CHAR acResponse[2] = {'\0'};

	UNREFERENCED_PARAMETER(Buffer);
	UNREFERENCED_PARAMETER(Length);

	if (KD_DEBUGGER_NOT_PRESENT)
	{
		return;
	}

	while (TRUE)
	{
		DbgPrompt(
			"Would you like to play a game? (y/n) ",
			acResponse,
			ARRAYSIZE(acResponse));

		if ('Y' == acResponse[0] || 'y' == acResponse[0])
		{
			break;
		}
		else if ('N' == acResponse[0] || 'n' == acResponse[0])
		{
			return;
		}
	}

	memmgr_init();
	MemInit(memmgr_alloc, memmgr_free, NULL);

	try
	{
		(VOID)main(0, NULL);
	}
	except(EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("\nA fatal error has occurred in the interpreter\n");
		DbgPrint("and an exception was raised.\n");
		DbgPrint("Don't worry, though - your system is wrecked anyway :)\n");
	}
}
