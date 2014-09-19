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

#include <stdlib.h>
#include <stdarg.h>

#include "..\common\frotz.h"
#include "..\memfile\memfile.h"


//
// Globals
//

f_setup_t f_setup = {0};

extern BYTE   StoryData[];
extern SIZE_T StorySize;

static char *latin1_to_ascii[] =  {
	""  , "!"  , "c" , "L" , ">o<", "Y" , "|" , "S", "''", "C" , "a", "<<", "not", "-"  , "R"  , "_" ,
	"^0", "+/-", "^2", "^3", "'"  , "my", "P" , ".", "," , "^1", "o", ">>", "1/4", "1/2", "3/4", "?" ,
	"A" , "A"  , "A" , "A" , "Ae" , "A" , "AE", "C", "E" , "E" , "E", "E" , "I"  , "I"  , "I"  , "I" ,
	"Th", "N"  , "O" , "O" , "O"  , "O" , "Oe", "*", "O" , "U" , "U", "U" , "Ue" , "Y"  , "Th" , "ss",
	"a" , "a"  , "a" , "a" , "ae" , "a" , "ae", "c", "e" , "e" , "e", "e" , "i"  , "i"  , "i"  , "i" ,
	"th", "n"  , "o" , "o" , "o"  , "o" , "oe", ":", "o" , "u" , "u", "u" , "ue" , "y"  , "th" , "y"
};

int g_row = 1;


//
// Functions
//

int
os_char_width(
	zchar character
)
{
	if (ZC_LATIN1_MIN <= character && ZC_LATIN1_MAX >= character)
	{
		return strlen(latin1_to_ascii[character - ZC_LATIN1_MIN]);
	}

	return 1;
}

int
os_string_width(
	const zchar *zstring
)
{
	int width = 0;
	zchar character = '\0';

	while (0 != (character = *zstring++))
	{
		if (ZC_NEW_STYLE == character || ZC_NEW_FONT == character)
		{
			zstring++;
			continue;
		}

		width += os_char_width(character);
	}

	return width;
}

void
os_display_char(
	zchar character
)
{
	if (ZC_LATIN1_MIN <= character && ZC_LATIN1_MAX >= character)
	{
		DbgPrint("%s", latin1_to_ascii[character - ZC_LATIN1_MIN]);
	}
	else if (32 <= character && 126 >= character)
	{
		DbgPrint("%c", character);
	}
	else if (ZC_GAP == character)
	{
		DbgPrint("  ");
	}
	else if (ZC_INDENT == character)
	{
		DbgPrint("   ");
	}
}

void
os_display_string(
	const zchar *zstring
)
{
	zchar character = '\0';

	while (0 != (character = *zstring++))
	{
		if (ZC_NEW_STYLE == character || ZC_NEW_FONT == character)
		{
			zstring++;
			continue;
		}
		
		os_display_char(character);
	}
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

int
os_random_seed(void)
{
	//
	// Chosen by fair random.randrange(2147483647).
	// Guaranteed to be random.
	// xkcd.com/221
	//
	return 2137389909;
}

int os_read_file_name(char *file_name, const char *default_name, int flag)
{
	DbgBreakPoint();
	return FALSE;
}

zchar os_read_key(int timeout, int show_cursor)
{
	zchar cCharacter[2] = {'\0'};

	(VOID)DbgPrompt("", cCharacter, 2);

	return cCharacter[0];
}

zchar os_read_line(int max, zchar *buf, int timeout, int width, int continued)
{
	(VOID)DbgPrompt("", buf, max);
	return ZC_RETURN;
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

void os_init_screen(void)
{
	//
	// Dumber than dumb frotz :)
	//
	h_flags &= ~GRAPHICS_FLAG;
	h_flags &= ~OLD_SOUND_FLAG;
	h_flags &= ~SOUND_FLAG;
	h_flags &= ~MOUSE_FLAG;
	h_flags &= ~COLOUR_FLAG;
	h_flags &= ~MENU_FLAG;

	h_interpreter_number = INTERP_MSDOS;
	h_interpreter_version = 'F';

	h_screen_width = 80;
	h_screen_height = 25;
	h_font_height = 1;
	h_font_width = 1;
	h_default_foreground = LIGHTGREY_COLOUR;
	h_default_background = BLUE_COLOUR;

	h_screen_cols = h_screen_width / h_font_width;
	h_screen_rows = h_screen_height / h_font_height;
}

void os_set_cursor(int row, int col)
{
	if (1 == col)
	{
		DbgPrint("\r");
	}
	
	if (row != g_row)
	{
		DbgPrint("\n");
		g_row = row;
	}
}

FILE *
os_load_story(void)
{
	return MemCreateFromBuffer(StoryData, StorySize);
}
