#include "sf_frotz.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* for access() */
#include <unistd.h>

#include <stdarg.h>

#include <SDL.h>

#include "../blorb/blorb.h"
#include "../blorb/blorblow.h"

zword hx_flags;
zword hx_fore_colour;
zword hx_back_colour;

z_header_t z_header;

/* various data */
bool m_tandy = 0;
int m_v6scale;
int m_gfxScale = 1;
ulong m_defaultFore;
ulong m_defaultBack;
ulong m_colours[11];
ulong m_nonStdColours[NON_STD_COLS];
int m_nonStdIndex;
bool m_exitPause = 0;
bool m_lineInput = 0;
bool m_IsInfocomV6 = false;
bool m_morePrompts = 1;
bool m_localfiles = false;
char *m_fontdir = NULL;
bool m_aafonts = 0;
char m_names_format = 0;
char *m_reslist_file = NULL;
char *m_setupfile = ".sfrotzrc";
extern int m_frequency;

bool sdl_active;

static int countedpics = 0;
static int maxlegalpic = 0;
static int releaseno = 0;

static char *ResDir = "./";
static char *ResPict = "PIC%d";
static char *ResSnd = "SND%d";

int AcWidth = 640, AcHeight = 400;
int option_scrollback_buffer = 0;

static FILE *bfile = NULL;
static FILE *zfile = NULL;
static bb_map_t *bmap = NULL;
static long zoffset;
static long zsize;
static int zcodeinblorb = 0;


static void checkwidths()
{
	bb_resolution_t *reso;
	reso = bb_get_resolution(bmap);
	if (reso) {
		/* ignore small resolution hints */
		if ((reso->px) && (reso->px >= AcWidth))
			AcWidth = reso->px;
		if ((reso->py) && (reso->py >= AcHeight))
			AcHeight = reso->py;
	}
}


static int tryloadblorb(char *bfn)
{
	bb_err_t err;
	bfile = fopen(bfn, "rb");
	if (!bfile)
		return -1;
	err = bb_create_map(bfile, &bmap);
	if (err) {
		bmap = NULL;
		fclose(bfile);
		bfile = NULL;
		return err;
	}
	return 0;
}


static void sf_cleanup_resources()
{
	if (zfile)
		fclose(zfile);
	zfile = NULL;
	if (bmap)
		bb_destroy_map(bmap);
	bmap = NULL;
	if (bfile)
		fclose(bfile);
	bfile = NULL;
}


static int checkfile(char *fn, char *ofn)
{
	char *p, *bp, lastch;

	if (access(fn, F_OK) == 0) {
		strcpy(ofn, fn);
		return 0;
	}
	/* if qualified path, don't do anything else */
	if ((strchr(fn, '\\')) || (strchr(fn, '/')))
		return -1;

	if ((p = getenv("ZCODE_PATH")) == NULL)
		p = getenv("INFOCOM_PATH");
	if (p != NULL) {
		while (*p) {
			bp = ofn;
			while (*p && *p != OS_PATHSEP)
				lastch = *bp++ = *p++;
			if (lastch != '\\' && lastch != '/')
				*bp++ = '\\';
			strcpy(bp, fn);
			if (access(ofn, F_OK) == 0)
				return 0;
			if (*p)
				p++;
		}
	}

	return -1;
}


/* must be called as soon as possible (i.e. by os_process_arguments()) */
static int load_resources(char *givenfn)
{
	char buf[FILENAME_MAX + 1], *p;
	int st;
	FILE *f;
	unsigned char hd2[2];

	CLEANREG(sf_cleanup_resources);

	/* first check whether file exists */
	st = checkfile(givenfn, buf);
	if (st)
		os_fatal("File not found");

	/* check whether it is a story file */
	f = fopen(buf, "rb");
	if (!f)
		os_fatal("File open failed");

	fread(hd2, 1, 2, f);

	/* blorb file ? */
	if (hd2[0] == 'F' && hd2[1] == 'O') {
		bb_result_t result;
		fclose(f);
		if (tryloadblorb(buf))
			os_fatal("File is neither Zcode nor Blorb");
		/* Look for an executable chunk */
		if (bb_load_resource
		    (bmap, bb_method_FilePos, &result, bb_ID_Exec, 0)
		    == bb_err_None) {
			unsigned int id = bmap->chunks[result.chunknum].type;
			if (id == bb_ID_ZCOD) {
				/* If this is a Z-code game,
				 * set the file pointer and return */
				zoffset = result.data.startpos;
				zsize = result.length;
				zcodeinblorb = 1;
				return 0;
			} else if (id == bb_ID_GLUL) {
				os_fatal(sf_msgstring(IDS_BLORB_GLULX));
			}
		}
		/* we are given a blorb file with no executable chunck
		 * Tell the user that there was no game in the Blorb file */
		os_fatal(sf_msgstring(IDS_BLORB_NOEXEC));
	}
	/* check if possibly Zcode */
	if (hd2[0] < 1 || hd2[0] > 8)
		os_fatal("Not a Zcode file (or wrong version)");

	/* OK, assume a bona fide Z code file */
	zfile = f;
	fseek(f, 0, SEEK_END);
	zsize = ftell(f);
	zoffset = 0;

	/* See if the user explicitly provided a blorb file */
	if (f_setup.blorb_file != NULL) {
		tryloadblorb(f_setup.blorb_file);
	} else {
		/* No?  Let's see if we can find one. */
		p = malloc(strlen(f_setup.story_name) +
			   strlen(EXT_BLORB4) * sizeof(char));
		strncpy(p, f_setup.story_name, strlen(f_setup.story_name) + 1);
		strncat(p, EXT_BLORB3, strlen(EXT_BLORB3) + 1);

		if (tryloadblorb(p) != bb_err_None) {	/* Trying foo.blorb */
			free(p);
			p = malloc(strlen(f_setup.story_name) +
				   strlen(EXT_BLORB4) * sizeof(char));
			strncpy(p, f_setup.story_name,
				strlen(f_setup.story_name) + 1);
			strncat(p, EXT_BLORB, strlen(EXT_BLORB) + 1);
			tryloadblorb(p);	/* Trying foo.blb */
		}
	}
	return 0;
}


static void load_local_resources();

/* must be called as soon as possible (i.e. by os_process_arguments()) */
int sf_load_resources(char *givenfn)
{
	int st;

	st = load_resources(givenfn);
	if (st)
		return st;

	if (bmap) {
		checkwidths();
		bb_count_resources(bmap, bb_ID_Pict, &countedpics, NULL,
				   &maxlegalpic);
		releaseno = bb_get_release_num(bmap);
	}

	if ((m_reslist_file))
		load_local_resources();

	return 0;
}


/* this routine is only used for the Z code, so we can safely
 * ignore its args
 * NOTE that there is an extra argument (as in WindowsFrotz version)
 */
FILE *os_path_open(const char *name, const char *mode, long *size)
{
	FILE *f = NULL;

	*size = zsize;

	if (zcodeinblorb)
		f = bfile;
	else
		f = zfile;

	if (f)
		fseek(f, zoffset, SEEK_SET);

	return f;
}


/*
 * os_picture_data
 *
 * Return true if the given picture is available. If so, store the
 * picture width and height in the appropriate variables. Picture
 * number 0 is a special case: Write the highest legal picture number
 * and the picture file release number into the height and width
 * variables respectively when this picture number is asked for.
 *
 */
int os_picture_data(int picture, int *height, int *width)
{
	if (maxlegalpic) {
		if (picture == 0) {
			*height = maxlegalpic;
			*width = releaseno;
			return 1;
		} else {
			sf_picture *res = sf_getpic(picture);
			if (res) {
				*height = m_gfxScale * res->height;
				*width = m_gfxScale * res->width;
				return 1;
			}
		}
	}
	*height = 0;
	*width = 0;
	return 0;
}


/*
 * os_menu
 *
 * Add to or remove a menu item. Action can be:
 *     MENU_NEW    - Add a new menu with the given title
 *     MENU_ADD    - Add a new menu item with the given text
 *     MENU_REMOVE - Remove the menu at the given index
 *
 */
void os_menu(int action, int menu, const zword * text)
{
/*	switch (action)
	{
	case MENU_NEW:
		theWnd->AddNewMenu(menu,text);
		break;
	case MENU_ADD:
		theWnd->AddMenuItem(menu,text);
		break;
	case MENU_REMOVE:
		theWnd->RemoveMenu(menu);
		break;
	}*/
}


/*
 * os_random_seed
 *
 * Return an appropriate random seed value in the range from 0 to
 * 32767, possibly by using the current system time.
 *
 * this is a provisional workaround (time granularity is at best 1s)
 */
#include <time.h>
int os_random_seed(void)
{
	if (m_random_seed == -1) {
		return ((int)(time(NULL))) & 32767;
	}
	return m_random_seed;
}


/* The following assumes Unicode */
/*
 * os_scrollback_char
 *
 * Write a character to the scrollback buffer.
 *
 */
void os_scrollback_char(zword c)
{
	if (option_scrollback_buffer == 0)
		return;
	if (c == 13)
		c = 10;
	if (option_scrollback_buffer == 1) {	/* Latin-1 */
		if (c > 255)
			c = '?';
		putchar(c);
	} else {		/* UTF8 */
		if (c < 0x80)
			putchar(c);
		else {
			putchar(0xc0 + (c >> 6));
			putchar(0x80 + (c & 0x3f));
		}
	}
}


/*
 * os_scrollback_erase
 *
 * Remove characters from the scrollback buffer.
 *
 */
void os_scrollback_erase(int erase)
{
	if (option_scrollback_buffer)
		while (erase--)
			putchar(8);
}


/*
 * os_restart_game
 *
 * This routine allows the interface to interfere with the process of
 * restarting a game at various stages:
 *
 *     RESTART_BEGIN - restart has just begun
 *     RESTART_WPROP_SET - window properties have been initialised
 *     RESTART_END - restart is complete
 *
 */
void os_restart_game(int stage)
{
	/* Show Beyond Zork's title screen */
	if ((stage == RESTART_END) && (story_id == BEYOND_ZORK)) {
		int w, h;
		if (os_picture_data(1, &h, &w)) {
			sf_fillrect(0, 0, 0, 10000, 10000);
			os_draw_picture(1, 1, 1);
			sf_read_key(0, false, false, false);
		}
	}
}


#define DEFAULT_GAMMA 2.2
double m_gamma = DEFAULT_GAMMA;

void sf_initcolours()
{
	int i;

	sf_setgamma(m_gamma);

	/* Standard Z-Machine colours */
	m_colours[0]  = RGB5ToTrue(0x0000);	/* black */
	m_colours[1]  = RGB5ToTrue(0x001D);	/* red */
	m_colours[2]  = RGB5ToTrue(0x0340);	/* green */
	m_colours[3]  = RGB5ToTrue(0x03BD);	/* yellow */
	m_colours[4]  = RGB5ToTrue(0x59A0);	/* blue */
	m_colours[5]  = RGB5ToTrue(0x7C1F);	/* magenta */
	m_colours[6]  = RGB5ToTrue(0x77A0);	/* cyan */
	m_colours[7]  = RGB5ToTrue(0x7FFF);	/* white */
	m_colours[8]  = RGB5ToTrue(0x5AD6);	/* light grey */
	m_colours[9]  = RGB5ToTrue(0x4631);	/* medium grey */
	m_colours[10] = RGB5ToTrue(0x2D6B);	/* dark grey */

	for (i = 0; i < NON_STD_COLS; i++)
		m_nonStdColours[i] = 0xFFFFFFFF;
	m_nonStdIndex = 0;

}


/* Read in settings */
void sf_readsettings(void)
{
	char *p;

	sf_InitProfile(m_setupfile);

	m_aafonts = sf_GetProfileInt("Fonts", "antialias", 0);
	m_fontdir = sf_GetProfileString("Fonts", "fontdir", NULL);
	m_fontfiles[0] = sf_GetProfileString("Fonts", "textroman", NULL);
	m_fontfiles[1] = sf_GetProfileString("Fonts", "textbold", NULL);
	m_fontfiles[2] = sf_GetProfileString("Fonts", "textitalic", NULL);
	m_fontfiles[3] = sf_GetProfileString("Fonts", "textbolditalic", NULL);
	m_fontfiles[4] = sf_GetProfileString("Fonts", "fixedroman", NULL);
	m_fontfiles[5] = sf_GetProfileString("Fonts", "fixedbold", NULL);
	m_fontfiles[6] = sf_GetProfileString("Fonts", "fixeditalic", NULL);
	m_fontfiles[7] = sf_GetProfileString("Fonts", "fixedbolditalic", NULL);
	m_fontfiles[8] = sf_GetProfileString("Fonts", "graphics", NULL);

	ResDir = sf_GetProfileString("Resources", "Dir", ResDir);
	ResPict = sf_GetProfileString("Resources", "Pict", ResPict);
	ResSnd = sf_GetProfileString("Resources", "Snd", ResSnd);

	if (f_setup.interpreter_number == 0) {
		z_header.interpreter_number =
		    sf_GetProfileInt("Interpreter", "Number", INTERP_AMIGA);
	} else
		z_header.interpreter_number = f_setup.interpreter_number;

	f_setup.err_report_mode =
	    sf_GetProfileInt("Interpreter", "Error Reporting", ERR_REPORT_ONCE);
	f_setup.ignore_errors =
	    sf_GetProfileInt("Interpreter", "Ignore Errors", 0);
	f_setup.expand_abbreviations =
	    sf_GetProfileInt("Interpreter", "Expand Abbreviations", 0);
	m_tandy =
	    sf_GetProfileInt("Interpreter", "Tandy Bit", 0) ? true : false;
	f_setup.script_cols =
	    sf_GetProfileInt("Interpreter", "Wrap Script Lines", 1) ? 80 : 0;

	if ((p = sf_GetProfileString("Interpreter", "SaveNames", NULL)))
		m_names_format = p[0];

	AcWidth = sf_GetProfileInt("Window", "AcWidth", AcWidth);
	AcHeight = sf_GetProfileInt("Window", "AcHeight", AcHeight);

	m_frequency = sf_GetProfileInt("Audio", "Frequency", m_frequency);
	m_v6scale = sf_GetProfileInt("Display", "Infocom V6 Scaling", 2);
	m_gfxScale = 1;
	m_defaultFore = (sf_GetProfileInt("Display", "Foreground", 0xffffff));
	m_defaultBack = (sf_GetProfileInt("Display", "Background", 0x800000));
	m_morePrompts =
	    sf_GetProfileInt("Display", "Show More Prompts", 1) ? true : false;
	m_gamma = sf_GetProfileDouble("Display", "Gamma", DEFAULT_GAMMA);
	sf_initcolours();

	sf_FinishProfile();
}


/* Get a colour */
ulong sf_GetColour(int colour)
{
	/* Standard colours */
	if ((colour >= BLACK_COLOUR) && (colour <= DARKGREY_COLOUR))
		return m_colours[colour - BLACK_COLOUR];

	/* Default colours */
	if (colour == 16)
		return m_defaultFore;
	if (colour == 17)
		return m_defaultBack;

	/* Non standard colours */
	if ((colour >= 18) && (colour < 256)) {
		if (m_nonStdColours[colour - 18] != 0xFFFFFFFF)
			return m_nonStdColours[colour - 18];
	}
	return m_colours[0];
}


/* Get a default colour */
ulong sf_GetDefaultColour(bool fore)
{
	if (m_IsInfocomV6)
		return sf_GetColour(fore ? WHITE_COLOUR : BLACK_COLOUR);
	return fore ? m_defaultFore : m_defaultBack;
}


/* Get an index for a non-standard colour */
int sf_GetColourIndex(ulong colour)
{
	int i, index = -1;
	/* Is this a standard colour? */
	for (i = 0; i < 11; i++) {
		if (m_colours[i] == colour)
			return i + BLACK_COLOUR;
	}

	/* Is this a default colour? */
	if (m_defaultFore == colour)
		return 16;
	if (m_defaultBack == colour)
		return 17;

	/* Is this colour already in the table? */
	for (i = 0; i < NON_STD_COLS; i++) {
		if (m_nonStdColours[i] == colour)
			return i + 18;
	}

	/* Find a free colour index */
	while (index == -1) {
		if (colour_in_use(m_nonStdIndex + 18) == 0) {
			m_nonStdColours[m_nonStdIndex] = colour;
			index = m_nonStdIndex + 18;
		}

		m_nonStdIndex++;
		if (m_nonStdIndex >= NON_STD_COLS)
			m_nonStdIndex = 0;
	}
	return index;
}


/*
 * os_set_colour
 *
 * Set the foreground and background colours which can be:
 *
 *     1
 *     BLACK_COLOUR
 *     RED_COLOUR
 *     GREEN_COLOUR
 *     YELLOW_COLOUR
 *     BLUE_COLOUR
 *     MAGENTA_COLOUR
 *     CYAN_COLOUR
 *     WHITE_COLOUR
 *     TRANSPARENT_COLOUR
 *
 *     Amiga only:
 *
 *     LIGHTGREY_COLOUR
 *     MEDIUMGREY_COLOUR
 *     DARKGREY_COLOUR
 *
 * There may be more colours in the range from 16 to 255; see the
 * remarks about os_peek_colour.
 *
 */
void os_set_colour(int new_foreground, int new_background)
{
	SF_textsetting *ts = sf_curtextsetting();
	sf_flushtext();
	if (new_foreground == 1)
		ts->fore = sf_GetDefaultColour(true);
	else if (new_foreground < 256)
		ts->fore = sf_GetColour(new_foreground);
	ts->foreDefault = (new_foreground == 1);

	if (new_background == 1)
		ts->back = sf_GetDefaultColour(false);
	else if (new_background < 256)
		ts->back = sf_GetColour(new_background);
	ts->backDefault = (new_background == 1);
	ts->backTransparent = (new_background == 15);
}


/*
 * os_from_true_cursor
 *
 * Given a true colour, return an appropriate colour index.
 *
 */
int os_from_true_colour(zword colour)
{
	return sf_GetColourIndex(RGB5ToTrue(colour));
}


/*
 * os_to_true_cursor
 *
 * Given a colour index, return the appropriate true colour.
 *
 */
zword os_to_true_colour(int index)
{
	return TrueToRGB5(sf_GetColour(index));
}


/*
 * os_init_screen
 *
 * Initialise the IO interface. Prepare screen and other devices
 * (mouse, sound card). Set various OS depending story file header
 * entries:
 *
 *     z_header.config (aka flags 1)
 *     z_header.flags (aka flags 2)
 *     z_header.screen_cols (aka screen width in characters)
 *     z_header.screen_rows (aka screen height in lines)
 *     z_header.screen_width
 *     z_header.screen_height
 *     z_header.font_height (defaults to 1)
 *     z_header.font_width (defaults to 1)
 *     z_header.default_foreground
 *     z_header.default_background
 *     z_header.interpreter_number
 *     z_header.interpreter_version
 *     z_header.user_name (optional; not used by any game)
 *
 * Finally, set reserve_mem to the amount of memory (in bytes) that
 * should not be used for multiple undo and reserved for later use.
 *
 */
void os_init_screen(void)
{

	sf_initvideo(AcWidth, AcHeight, (m_fullscreen != -1));

	/* Set the graphics scaling */
	if (sf_IsInfocomV6() || (story_id == BEYOND_ZORK))
		m_gfxScale = m_v6scale;
	else
		m_gfxScale = 1;

	/* Set the configuration */
	if (z_header.version == V3) {
		z_header.config |= CONFIG_SPLITSCREEN;
		z_header.config |= CONFIG_PROPORTIONAL;
		if (m_tandy)
			z_header.config |= CONFIG_TANDY;
		else
			z_header.config &= ~CONFIG_TANDY;
	}
	if (z_header.version >= V4) {
		z_header.config |= CONFIG_BOLDFACE;
		z_header.config |= CONFIG_EMPHASIS;
		z_header.config |= CONFIG_FIXED;
		z_header.config |= CONFIG_TIMEDINPUT;
	}
	if (z_header.version >= V5)
		z_header.config |= CONFIG_COLOUR;
	if (z_header.version == V6) {
		if (bmap) {
			z_header.config |= CONFIG_PICTURES;
			z_header.config |= CONFIG_SOUND;
		}
	}

	z_header.interpreter_version = 'F';
	if (z_header.version == V6) {
		z_header.default_foreground =
		    sf_GetColourIndex(sf_GetDefaultColour(true));
		z_header.default_background =
		    sf_GetColourIndex(sf_GetDefaultColour(false));
	} else {
		z_header.default_foreground = 1;
		z_header.default_background = 1;
	}

	os_set_font(FIXED_WIDTH_FONT);
	os_set_text_style(0);

	{
		int H, W;
		os_font_data(FIXED_WIDTH_FONT, &H, &W);
		z_header.font_width = (zbyte) W;
		z_header.font_height = (zbyte) H;
	}

	z_header.screen_width = (zword) AcWidth;
	z_header.screen_height = (zword) AcHeight;
	z_header.screen_cols = (zbyte) (z_header.screen_width / z_header.font_width);
	z_header.screen_rows = (zbyte) (z_header.screen_height / z_header.font_height);

	/* Check for sound */
	if ((z_header.version == V3) && (z_header.flags & OLD_SOUND_FLAG)) {
		if (((bmap == NULL) && (m_localfiles == 0))
		    || (!sf_initsound()))
			z_header.flags &= ~OLD_SOUND_FLAG;
	} else if ((z_header.version >= V4) && (z_header.flags & SOUND_FLAG)) {
		if (((bmap == NULL) && (m_localfiles == 0))
		    || (!sf_initsound()))
			z_header.flags &= ~SOUND_FLAG;
	}

	if (z_header.version >= V5) {
		zword mask = 0;
		if (z_header.version == V6)
			mask |= TRANSPARENT_FLAG;

		/* Mask out any unsupported bits in the extended flags */
		hx_flags &= mask;

		hx_fore_colour = TrueToRGB5(sf_GetDefaultColour(true));
		hx_back_colour = TrueToRGB5(sf_GetDefaultColour(false));
	}
}


/*
 * os_fatal
 *
 * Display error message and stop interpreter.
 *
 */
void os_fatal(const char *s, ...)
{
	va_list m;

	fprintf(stderr, "\n%s: ", sf_msgstring(IDS_FATAL));
	va_start(m, s);
	vfprintf(stderr, s, m);
	va_end(m);
	fprintf(stderr, "\n");

	if (sdl_active) {
		os_set_text_style(0);
		os_display_string((zchar *) "\n\n");
		os_beep(BEEP_HIGH);
		os_set_text_style(BOLDFACE_STYLE);

		os_display_string((zchar *) "Fatal error: ");
		os_set_text_style(0);
		os_display_string((zchar *) s);
		os_display_string((zchar *) "\n\n");
		new_line();
		flush_buffer();
	}

	if (f_setup.ignore_errors) {
		if (sdl_active) {
			os_display_string((zchar *) "Continuing anyway...");
			new_line();
			new_line();
		}
		fprintf(stderr, "Continuing anyway...\n");
		return;
	}

	if (sdl_active) {
		os_reset_screen();
		SDL_Quit();
	}
	sf_cleanup_all();
	exit(EXIT_FAILURE);
}


/* If true, running one of Infocom's V6 games */
bool sf_IsInfocomV6()
{
	switch (story_id) {
	case ARTHUR:
	case JOURNEY:
	case SHOGUN:
	case ZORK_ZERO:
		return true;
	default:
		break;
	}
	return false;
}


/* If true, this picture has an adaptive palette */
bool sf_IsAdaptive(int picture)
{
	bb_result_t result;
	bool adaptive = FALSE;

	if (bb_load_chunk_by_type
	    (bmap, bb_method_Memory, &result, bb_ID_APal, 0) == bb_err_None) {
		for (int i = 0; i < (int)result.length; i += 4) {
			unsigned char *data =
			    ((unsigned char *)result.data.ptr) + i;
			int entry =
			    (data[0] << 24) | (data[1] << 16) | (data[2] << 8) |
			    data[3];
			if (picture == entry) {
				adaptive = TRUE;
				break;
			}
		}
	}
	bb_unload_chunk(bmap, result.chunknum);
	return adaptive;
}


#define LOCAL_MEM	-1
#define LOCAL_FILE	-2

void sf_freeresource(myresource * res)
{
	int cnu;

	if (!res)
		return;

	cnu = res->bbres.chunknum;

	if (cnu == LOCAL_MEM) {
		if (res->bbres.data.ptr)
			free(res->bbres.data.ptr);
		return;
	}
	if (cnu == LOCAL_FILE) {
		if (res->file)
			fclose(res->file);
		return;
	}

	if ((bmap) && (cnu >= 0))
		bb_unload_chunk(bmap, cnu);
}


static FILE *findlocal(int ispic, int num, int *size)
{
	FILE *f;
	char *tpl, buf[MAX_FILE_NAME + 1];

	tpl = ispic ? ResPict : ResSnd;
	strcpy(buf, ResDir);
	sprintf(buf + strlen(buf), tpl, num);
	f = fopen(buf, "rb");
	if (!f)
		return f;
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	fseek(f, 0, SEEK_SET);
	return f;
}


static FILE *findfromlist(int ispic, int num, int *size);


static int loadlocal(int num, int ispic, int method, myresource * res)
{
	FILE *f;
	int size;
	byte hd[4];

	f = findlocal(ispic, num, &size);
	if (!f)
		f = findfromlist(ispic, num, &size);
	if (!f)
		return bb_err_NotFound;

	fread(hd, 1, 4, f);
	fseek(f, 0, SEEK_SET);
	res->type = 0;
	if (ispic) {
		if (hd[0] == 0xff && hd[1] == 0xd8)
			res->type = bb_ID_JPEG;
		else if (hd[0] == 0x89 && hd[1] == 0x50)
			res->type = bb_ID_PNG;
	} else {
		if (memcmp(hd, "FORM", 4) == 0)
			res->type = bb_ID_FORM;
		else if (memcmp(hd, "OggS", 4) == 0)
			res->type = bb_ID_OGGV;
		else
			res->type = bb_ID_MOD;
	}
	if (!res->type) {
		fclose(f);
		return bb_err_NotFound;
	}

	res->bbres.data.startpos = 0;
	res->file = f;
	res->bbres.length = size;
	if (method == bb_method_FilePos)
		res->bbres.chunknum = LOCAL_FILE;
	else {
		void *ptr;
		res->bbres.chunknum = LOCAL_MEM;
		ptr = res->bbres.data.ptr = malloc(size);
		if (ptr)
			fread(ptr, 1, size, f);
		fclose(f);
		if (!ptr)
			return bb_err_NotFound;
	}

	return bb_err_None;
}


int sf_getresource(int num, int ispic, int method, myresource * res)
{
	int st;
	ulong usage;

	res->bbres.data.ptr = NULL;
	res->file = NULL;

	if (m_localfiles)
		if ((st = loadlocal(num, ispic, method, res)) == bb_err_None)
			return st;

	if (!bmap)
		return bb_err_NotFound;

	if (ispic)
		usage = bb_ID_Pict;
	else
		usage = bb_ID_Snd;
	/* XXX Should use bb_load_resource_{pict,snd} with auxdata? */
	st = bb_load_resource(bmap, method, (bb_result_t *) res, usage, num);
	if (st == bb_err_None) {
		res->type = bmap->chunks[res->bbres.chunknum].type;
		if (method == bb_method_FilePos)
			res->file = bfile;
	}
	return st;
}

/***************/

typedef struct {
	void *next;
	int num, ispic;
	ulong type;
	char *name;
} LLENTRY;

static LLENTRY *Lpics = NULL, *Lsnds = NULL;

static int numlocal = 0, numlocalpic = 0, numlocalsnd = 0;
static int p_ispic, p_num;
static ulong p_type;
static char *p_name;


static void cleanLLENTRY(LLENTRY * e)
{
	while (e) {
		LLENTRY *n = e->next;
		if (e->name)
			free(e->name);
		free(e);
		e = n;
	}
}


static void cleanlocallist()
{
	cleanLLENTRY(Lpics);
	Lpics = NULL;
	cleanLLENTRY(Lsnds);
	Lsnds = NULL;
}


static int parseline(char *s)
{
	char *p, p3;
	int n;
	p = strtok(s, " \t\n");
	if (!p)
		return 0;
	if (strcmp(p, "Pict") == 0)
		p_ispic = 1;
	else if (strcmp(p, "Snd") == 0)
		p_ispic = 0;
	else
		return -1;
	p = strtok(NULL, " \t\n");
	if (!p)
		return -1;
	p_num = atoi(p);
	p = strtok(NULL, " \t\n");
	if (!p)
		return -1;
	n = strlen(p);
	if (n < 3)
		return -1;
	if (p[3])
		p3 = p[3];
	else
		p3 = ' ';
	p_type = bb_make_id(p[0], p[1], p[2], p3);
	p = strtok(NULL, " \t\n");
	if (!p)
		return -1;
	p_name = p;
	return 1;
}


static void load_local_resources()
{
	FILE *f;
	LLENTRY *e;
	char s[256];
	int st;
	f = fopen(m_reslist_file, "r");
	if (!f)
		return;
	CLEANREG(cleanlocallist);
	for (;;) {
		fgets(s, 254, f);
		if (feof(f))
			break;
		st = parseline(s);
		if (st < 1)
			continue;
		e = calloc(1, sizeof(LLENTRY));
		if (e) {
			e->num = p_num;
			e->ispic = p_ispic;
			e->type = p_type;
			e->name = strdup(p_name);
			if (p_ispic) {
				e->next = Lpics;
				Lpics = e;
				numlocalpic++;
				if (p_num > maxlegalpic)
					maxlegalpic = p_num;
			} else {
				e->next = Lsnds;
				Lsnds = e;
				numlocalsnd++;
			}
		}
	}
	numlocal = numlocalpic + numlocalsnd;
	if (numlocal)
		m_localfiles = 1;
	fclose(f);
}


static FILE *findfromlist(int ispic, int num, int *size)
{
	FILE *f;
	LLENTRY *l;
	char buf[MAX_FILE_NAME + 1];

	if (ispic)
		l = Lpics;
	else
		l = Lsnds;
	while (l) {
		if (l->num == num)
			break;
		l = l->next;
	}
	if (!l)
		return NULL;

	strcpy(buf, ResDir);
	strcat(buf, l->name);

	f = fopen(buf, "rb");
	if (!f)
		return f;
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	fseek(f, 0, SEEK_SET);
	return f;
}


void os_init_setup(void)
{
	sf_setdialog();
	sf_initloader();

	f_setup.attribute_assignment = 0;
	f_setup.attribute_testing = 0;
	f_setup.context_lines = 0;
	f_setup.object_locating = 0;
	f_setup.object_movement = 0;
	f_setup.left_margin = 0;
	f_setup.right_margin = 0;
	f_setup.ignore_errors = 0;
	f_setup.interpreter_number = 0;
	f_setup.piracy = 0;	/* enable the piracy opcode */
	f_setup.undo_slots = MAX_UNDO_SLOTS;
	f_setup.expand_abbreviations = 0;
	f_setup.script_cols = 80;
	f_setup.sound = 1;
	f_setup.err_report_mode = ERR_DEFAULT_REPORT_MODE;
	f_setup.restore_mode = 0;

	f_setup.blorb_file = NULL;
	f_setup.story_file = NULL;
	f_setup.story_name = NULL;
	f_setup.story_base = NULL;
	f_setup.script_name = NULL;
	f_setup.command_name = NULL;
	f_setup.save_name = NULL;
	f_setup.tmp_save_name = NULL;
	f_setup.aux_name = NULL;
	f_setup.story_path = NULL;
	f_setup.zcode_path = NULL;
	f_setup.restricted_path = NULL;

	sdl_active = FALSE;
}
