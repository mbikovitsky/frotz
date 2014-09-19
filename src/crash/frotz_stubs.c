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

#include "..\common\frotz.h"


//
// Effects
//

void os_set_colour(int foreground, int background) {}
int os_peek_colour(void) { return 0; }
void os_set_text_style(int style) {}
void os_set_font(int font) {}
int os_font_data(int font, int *height, int *width) { return 0; }

//
// Sound
//

void os_prepare_sample(int a) {}
void os_start_sample(int a, int b, int c, zword d) {}
void os_stop_sample() {}
void os_finish_with_sample() {}
void os_beep(int volume) {}
int os_speech_output(const zchar *zstring) { return 0; }

//
// Pictures
//

void os_draw_picture(int num, int y, int x) {}
int os_picture_data(int num, int *height, int *width) { return 0; }

//
// Screen
//

void os_reset_screen(void) {}
void os_erase_area(int top, int left, int bottom, int right, int win) {}
void os_scroll_area(int top, int left, int bottom, int right, int units) {}

//
// Game
//

void os_process_arguments(int argc, char *argv[]) {}
void os_restart_game(int stage) {}
void os_more_prompt(void) {}
