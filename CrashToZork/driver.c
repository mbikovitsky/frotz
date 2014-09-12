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

#include <wdm.h>


//
// Forward declarations
//

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD     Unload;

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
}
