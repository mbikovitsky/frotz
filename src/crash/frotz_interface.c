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

#include <stdarg.h>

#include "..\common\frotz.h"


//
// Globals
//

f_setup_t f_setup = {0};


//
// Functions
//

void os_beep(int volume)
{
}

int os_char_width(zchar c)
{
	return 0;
}

void os_display_char(zchar c)
{
}

void os_display_string(const zchar *zstring)
{
}

void os_draw_picture(int num, int y, int x)
{
}

void os_erase_area(int top, int left, int bottom, int right, int win)
{
}

void
os_fatal(
	const char * format,
	...
)
{
	va_list vaArgs = NULL;
	
	va_start(vaArgs, format);
	vDbgPrintEx(
		DPFLTR_DEFAULT_ID,
		DPFLTR_ERROR_LEVEL,
		format,
		vaArgs);
	va_end(vaArgs);

	//
	// Carpe diem.
	//
	ExRaiseStatus(STATUS_UNSUCCESSFUL);
}

void os_finish_with_sample()
{
}

int os_font_data(int font, int *height, int *width)
{
	return 0;
}

void os_init_screen(void)
{
}

void os_more_prompt(void)
{
}

int os_peek_colour(void)
{
	return 0;
}

int os_picture_data(int num, int *height, int *width)
{
	return 0;
}

void os_prepare_sample(int a)
{
}

int os_random_seed(void)
{
	return 0;
}

int os_read_file_name(char *file_name, const char *default_name, int flag)
{
	return 0;
}

zchar os_read_key(int timeout, int show_cursor)
{
	return 0;
}

zchar os_read_line(int max, zchar *buf, int timeout, int width, int continued)
{
	return 0;
}

void os_reset_screen(void)
{
}

void os_restart_game(int stage)
{
}

void os_scroll_area(int top, int left, int bottom, int right, int units)
{
}

void os_set_colour(int foreground, int background)
{
}

void os_set_cursor(int row, int col)
{
}

void os_set_font(int font)
{
}

void os_set_text_style(int style)
{
}

void os_start_sample(int a, int b, int c, zword d)
{
}

void os_stop_sample()
{
}

int os_string_width(const zchar *zstring)
{
	return 0;
}

void os_init_setup(void)
{
	RtlSecureZeroMemory(&f_setup, sizeof(f_setup));
	f_setup.undo_slots = MAX_UNDO_SLOTS;
	f_setup.script_cols = 80;
	f_setup.save_quetzal = 1;
	f_setup.sound = 1;
	f_setup.err_report_mode = ERR_DEFAULT_REPORT_MODE;
}

int os_speech_output(const zchar *zstring)
{
	return 0;
}

FILE* os_load_story(void)
{
	return 0;
}
