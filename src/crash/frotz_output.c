/*++
Copyright (c) 2014 Michael Bikovitsky

Module Name:

	frotz_output.c

Abstract: 

	Frotz output routines.

Environment:

	Kernel mode only

--*/

//
// Headers
//

#include <ntddk.h>
#include <windef.h>

#include <string.h>

#include "..\common\frotz.h"


//
// Constants
//

#define SCREEN_COLS (80)
#define SCREEN_ROWS (25)
#define SCREEN_SEPARATOR ("********************************************\n")


//
// Globals
//

static PSTR g_apszLatin1ToAscii[] = {
	""  , "!"  , "c" , "L" , ">o<", "Y" , "|" , "S", "''", "C" , "a", "<<", "not", "-"  , "R"  , "_" ,
	"^0", "+/-", "^2", "^3", "'"  , "my", "P" , ".", "," , "^1", "o", ">>", "1/4", "1/2", "3/4", "?" ,
	"A" , "A"  , "A" , "A" , "Ae" , "A" , "AE", "C", "E" , "E" , "E", "E" , "I"  , "I"  , "I"  , "I" ,
	"Th", "N"  , "O" , "O" , "O"  , "O" , "Oe", "*", "O" , "U" , "U", "U" , "Ue" , "Y"  , "Th" , "ss",
	"a" , "a"  , "a" , "a" , "ae" , "a" , "ae", "c", "e" , "e" , "e", "e" , "i"  , "i"  , "i"  , "i" ,
	"th", "n"  , "o" , "o" , "o"  , "o" , "oe", ":", "o" , "u" , "u", "u" , "ue" , "y"  , "th" , "y"
};

static CHAR  g_acScreen[SCREEN_ROWS][SCREEN_COLS] = {' '};
static DWORD g_nRow                                = 0;
static DWORD g_nColumn                             = 0;


//
// Functions
//

INT
os_char_width(
	zchar cCharacter
)
{
	if (ZC_LATIN1_MIN <= cCharacter && ZC_LATIN1_MAX >= cCharacter)
	{
		return strlen(g_apszLatin1ToAscii[cCharacter - ZC_LATIN1_MIN]);
	}

	return 1;
}

INT
os_string_width(
	CONST zchar * pszString
)
{
	INT   iWidth     = 0;
	zchar cCharacter = '\0';

	while (0 != (cCharacter = *pszString++))
	{
		if (ZC_NEW_STYLE == cCharacter || ZC_NEW_FONT == cCharacter)
		{
			pszString++;
			continue;
		}

		iWidth += os_char_width(cCharacter);
	}

	return iWidth;
}

static
VOID
dumb_display_char(
	CHAR cCharacter
)
{
	g_acScreen[g_nRow][g_nColumn] = cCharacter;

	if (++g_nColumn == h_screen_cols)
	{
		if (h_screen_rows - 1 == g_nRow)
		{
			--g_nColumn;
		}
		else
		{
			++g_nRow;
			g_nColumn = 0;
		}
	}
}

VOID
os_display_char(
	zchar cCharacter
)
{
	if (ZC_LATIN1_MIN <= cCharacter && ZC_LATIN1_MAX >= cCharacter)
	{
		INT nIndex = 0;
		for (nIndex = 0; nIndex < os_char_width(cCharacter); ++nIndex)
		{
			dumb_display_char(
				g_apszLatin1ToAscii[cCharacter - ZC_LATIN1_MIN][nIndex]);
		}
	}
	else if (32 <= cCharacter && 126 >= cCharacter)
	{
		dumb_display_char(cCharacter);
	}
	else if (ZC_GAP == cCharacter)
	{
		dumb_display_char(' ');
		dumb_display_char(' ');
	}
	else if (ZC_INDENT == cCharacter)
	{
		dumb_display_char(' ');
		dumb_display_char(' ');
		dumb_display_char(' ');
	}
}

VOID
os_display_string(
	CONST zchar * pszString
)
{
	zchar cCharacter = '\0';

	while (0 != (cCharacter = *pszString++))
	{
		if (ZC_NEW_STYLE == cCharacter || ZC_NEW_FONT == cCharacter)
		{
			pszString++;
			continue;
		}

		os_display_char(cCharacter);
	}
}

VOID
os_init_screen(VOID)
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

	h_default_foreground = LIGHTGREY_COLOUR;
	h_default_background = BLUE_COLOUR;

	h_screen_cols = SCREEN_COLS;
	h_screen_rows = SCREEN_ROWS;

	h_font_height   = 1;
	h_font_width    = 1;
	h_screen_width  = h_screen_cols * h_font_width;
	h_screen_height = h_screen_rows * h_font_height;
}

VOID
os_set_cursor(
	INT nRow,
	INT nColumn
)
{
	g_nRow    = nRow - 1;
	g_nColumn = nColumn - 1;
}

VOID
os_erase_area(
	INT iTop,
	INT iLeft,
	INT iBottom,
	INT iRight,
	INT nWin
)
{
	DWORD nRow    = 0;
	DWORD nColumn = 0;

	UNREFERENCED_PARAMETER(nWin);

	for (nRow = iTop - 1; nRow < (DWORD)iBottom; ++nRow)
	{
		for (nColumn = iLeft - 1; nColumn < (DWORD)iRight; ++nColumn)
		{
			g_acScreen[nRow][nColumn] = ' ';
		}
	}
}

VOID
os_scroll_area(
	INT iTop,
	INT iLeft,
	INT iBottom,
	INT iRight,
	INT nUnits
)
{
	DWORD nRow    = 0;
	DWORD nColumn = 0;

	if (0 < nUnits)
	{
		for (nRow = iTop - 1; nRow < (DWORD)(iBottom - nUnits); ++nRow)
		{
			for (nColumn = iLeft - 1; nColumn < (DWORD)iRight; ++nColumn)
			{
				g_acScreen[nRow][nColumn] = g_acScreen[nRow + nUnits][nColumn];
			}
		}

		os_erase_area(iBottom - nUnits + 1, iLeft, iBottom, iRight, -1);
	}
	else if (0 > nUnits)
	{
		for (nRow = iBottom - 1; nRow > (DWORD)(iTop - nUnits); --nRow)
		{
			for (nColumn = iLeft - 1; nColumn < (DWORD)iRight; ++nColumn)
			{
				g_acScreen[nRow][nColumn] = g_acScreen[nRow + nUnits][nColumn];
			}
		}

		os_erase_area(iTop, iLeft, iTop - nUnits - 1, iRight, -1);
	}
}

VOID
dump_screen(VOID)
{
	DWORD nRow    = 0;
	DWORD nColumn = 0;

	DbgPrint(SCREEN_SEPARATOR);
	DbgPrint(SCREEN_SEPARATOR);

	for (nRow = 0; nRow < h_screen_rows; ++nRow)
	{
		DbgPrint("%.*s\n", h_screen_cols, g_acScreen[nRow]);
	}
}
