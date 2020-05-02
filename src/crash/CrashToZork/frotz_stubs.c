/*++
Copyright (c) 2014 Michael Bikovitsky

Module Name:

	frotz_stubs.c

Abstract: 

	Contains Frotz interface stubs.

Environment:

	Kernel mode only

--*/

//
// Headers
//

#include <ntddk.h>
#include <windef.h>

#include "..\..\common\frotz.h"


#pragma warning(push)
#pragma warning(disable: 4100)  // unreferenced formal parameter



//
// Effects
//

VOID os_set_colour     (INT iForeground, INT iBackground)       {}
INT  os_peek_colour    (VOID)                                   { return 0; }
VOID os_set_text_style (INT iStyle)                             {}
VOID os_set_font       (INT iFont)                              {}
INT  os_font_data      (INT iFont, PINT piHeight, PINT piWidth) { return 0; }

//
// Sound
//

VOID os_init_sound         (VOID)                         {}
VOID os_prepare_sample     (INT a)                        {}
VOID os_start_sample       (INT a, INT b, INT c, zword d) {}
VOID os_stop_sample        (INT a)                        {}
VOID os_finish_with_sample (INT a)                        {}
VOID os_beep               (INT iVolume)                  {}
INT  os_speech_output      (CONST zchar * pszString)      { return 0; }

//
// Pictures
//

VOID os_draw_picture (INT num, INT y, INT x)                {}
INT  os_picture_data (INT num, PINT piHeight, PINT piWidth) { return 0; }

//
// Screen
//

VOID os_reset_screen (VOID) {}

//
// Game
//

VOID os_process_arguments (INT iArgc, PSTR apszArgv[]) {}
VOID os_restart_game      (INT iStage)                 {}
VOID os_more_prompt       (VOID)                       {}
VOID os_tick              (VOID)                       {}
VOID os_quit              (INT a)                      {}


#pragma warning(pop)
