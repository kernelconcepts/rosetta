/*
 main.c - mainfile of rosetta. graphical frontend and event handling
 (c) 2003 by Ole Reinhardt <ole.reinhardt@kernelconcepts.de>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Library General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <X11/extensions/shape.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "rosetta.h"
#include "libvirtkeys.h"

#define     PEN_MOVE  0
#define     PEN_UP    1
#define     PEN_DOWN  2

#define     BAR_HEIGHT 18
#define     PROP_MOTIF_WM_HINTS_ELEMENTS    5
#define     MWM_HINTS_DECORATIONS          (1L << 1)
#define     MWM_DECOR_BORDER               (1L << 1)

Display     *myDisplay;
Window      myWindow;
GC          myGC;
XFontStruct *myFont;


int         myScreen;
int         window_x;
int         window_y;
int         window_width;
int         window_height;
 
int         start_time;
int         repeat = 0;
int         learn_set_group_mode  = LEARN_SET_GROUP_CONFIG;


typedef struct
{
	unsigned long flags;
	unsigned long functions;
	unsigned long decorations;
	long inputMode;
	unsigned long status;
} PropMotifWmHints;

typedef struct PenEvent
{
	int x;
	int y;
} PenEvent;

void penMove (PenEvent * pe);
void penUp   (PenEvent * pe);
void penDown (PenEvent * pe);

t_data_struct input_data;
t_charlist    analized_data;
t_database    stroke_database;
t_config      configuration;

int           mode;
int           keycount;
int           clear = FALSE;
int           pendown = FALSE;

int           flag_use_precalc = TRUE;
int           flag_use_dict    = FALSE;
char          dict_file_name[100];
char          conf_file_name[100];

int timeout_timer = 0;

static int x_error_handler (Display * xdisplay, XErrorEvent * error)
{
	char buf[64];

	XGetErrorText (xdisplay, error->error_code, buf, 63);

	fprintf (stderr, "Unexpected X error: %s serial %ld error_code %d request_code %d minor_code %d)\n",
		 buf, error->serial, error->error_code, error->request_code, error->minor_code);

	exit (1);
}

void assignDefault (void)
{
	window_x      = 0;
	window_y      = 0;
	window_width  = WIDTH;
	window_height = HEIGHT;
}

unsigned long getColor (Display * dsp, unsigned short red, unsigned green, unsigned blue)
{
	XColor col;
	Colormap cmap = DefaultColormapOfScreen (ScreenOfDisplay (dsp, 0));
	col.red = (red + 1) * 256 - 1;
	col.green = (green + 1) * 256 - 1;
	col.blue = (blue + 1) * 256 - 1;
	if (!XAllocColor (dsp, cmap, &col))
		fprintf (stderr, "Can't alloc color\n");
	return col.pixel;
}

void show_statistic(void)
{
	int   count;
	int   lcount;
	int   multi;
	int   xmin;
	int   xmax;
	int   ymin;
	int   ymax;
	
	for (lcount = 0; lcount < input_data.lcount; lcount++)
	{		
		xmin = input_data.lines[lcount].xmin;
		ymin = input_data.lines[lcount].ymin;
		xmax = input_data.lines[lcount].xmax;
		ymax = input_data.lines[lcount].ymax;
		multi = FALSE;
		
		for (count = lcount+1; count < input_data.lcount; count++)
			if (input_data.lines[count].multi &&
			    (input_data.lines[count].reference == lcount))
			{
				multi = TRUE;
				if (xmin > input_data.lines[count].xmin) xmin = input_data.lines[count].xmin;
				if (ymin > input_data.lines[count].ymin) ymin = input_data.lines[count].ymin;
		
				if (xmax < input_data.lines[count].xmax) xmax = input_data.lines[count].xmax;
				if (ymax < input_data.lines[count].ymax) ymax = input_data.lines[count].ymax;
			}
		
		
		if (multi)
		{			
			XSetForeground (myDisplay, myGC, getColor (myDisplay, 200, 0, 0));
			XDrawRectangle (myDisplay, myWindow, myGC, xmin, ymin, xmax-xmin, ymax-ymin);
		}

	}	
}

int get_text_width(char *text)
{
	int d, asc, desc;
	XCharStruct cs;
	
	XTextExtents(myFont, text, strlen(text), &d, &asc, &desc, &cs);
	return (cs.width);
}

int get_text_height(char *text)
{
	int d, asc, desc;
	XCharStruct cs;
	
	XTextExtents(myFont, text, strlen(text), &d, &asc, &desc, &cs);
	return (asc+desc);
}

void draw(int mode)
{
	int lcount;
	int pcount;
	Pixmap doublebuf;	
	GC     dbGC;
	XGCValues            myGCValues;
	
       	doublebuf = XCreatePixmap(myDisplay, myWindow, window_width, window_height, DefaultDepth  (myDisplay, myScreen));
	dbGC = XCreateGC (myDisplay, doublebuf, (unsigned long) 0, &myGCValues);
	
	XSetForeground (myDisplay, dbGC, getColor (myDisplay, 255, 255, 255));
	XFillRectangle (myDisplay, doublebuf, dbGC, 0, BAR_HEIGHT, window_width,
			window_height-BAR_HEIGHT);

	XSetForeground (myDisplay, dbGC, getColor (myDisplay, 222, 222, 222));

	XFillRectangle (myDisplay, doublebuf, dbGC, 0, 0, window_width,
			BAR_HEIGHT);
	XSetForeground (myDisplay, dbGC, getColor (myDisplay, 200, 200, 200));
	XSetLineAttributes (myDisplay, dbGC, 1, LineOnOffDash,
			    CapNotLast, JoinMiter);
	/* inner lines */
	

	XDrawLine (myDisplay, doublebuf, dbGC, 0,
		   BAR_HEIGHT, window_width, BAR_HEIGHT);

	XDrawLine (myDisplay, doublebuf, dbGC, 0,
		   (window_height - BAR_HEIGHT) / 3 + BAR_HEIGHT,
		   window_width,
		   (window_height - BAR_HEIGHT) / 3 + BAR_HEIGHT);
	
	XDrawLine (myDisplay, doublebuf, dbGC, 0,
		   (window_height - BAR_HEIGHT) * 2 / 3 + BAR_HEIGHT,
		   window_width,
		   (window_height - BAR_HEIGHT) * 2 / 3 + BAR_HEIGHT);

	XDrawLine (myDisplay, doublebuf, dbGC, window_width / 3,
		   0, window_width / 3, window_height);
	XDrawLine (myDisplay, doublebuf, dbGC, (window_width / 3) * 2,
		   0, (window_width / 3) * 2, window_height);

	
	XSetForeground (myDisplay, dbGC, getColor( myDisplay, 90, 180, 200 ) );

	XDrawString( myDisplay, doublebuf, dbGC, 5+1, get_text_height("abc")+1,
		"abc", strlen("abc"));
	XDrawString( myDisplay, doublebuf, dbGC, window_width/2 - get_text_width("ABC")/2 + 1, get_text_height("ABC") + 1,
		"ABC", strlen("ABC"));
	XDrawString( myDisplay, doublebuf, dbGC, window_width - get_text_width("Symbol") - 5 + 1, get_text_height("Symbol") + 1,
		"Symbol", strlen("Symbol"));
	
	XSetForeground (myDisplay, dbGC, getColor( myDisplay, 60, 120, 200 ) );
	
	XDrawString( myDisplay, doublebuf, dbGC, 5, get_text_height("abc"),
		"abc", strlen("abc"));
	XDrawString( myDisplay, doublebuf, dbGC, window_width/2 - get_text_width("ABC")/2, get_text_height("ABC"),
		"ABC", strlen("ABC"));
	XDrawString( myDisplay, doublebuf, dbGC, window_width - get_text_width("Symbol") - 5, get_text_height("Symbol"),
		"Symbol", strlen("Symbol"));	
	
	XSetForeground (myDisplay, dbGC, BlackPixel( myDisplay, myScreen ) );

	XSetLineAttributes (myDisplay, dbGC, 1, LineSolid,
			    CapNotLast, JoinMiter);

	/* borders */
	XDrawLine (myDisplay, doublebuf, dbGC, 0, 0, window_width - 1, 0);
	XDrawLine (myDisplay, doublebuf, dbGC, 0, 0, 0, window_height - 1);
	XDrawLine (myDisplay, doublebuf, dbGC, 0,
		   window_height - 1, window_width, window_height - 1);
	XDrawLine (myDisplay, doublebuf, dbGC, window_width - 1,
		   0, window_width - 1, window_height - 1);


	XSetLineAttributes (myDisplay, dbGC, 3, LineSolid,
			    CapRound, JoinMiter);

	if (mode) 
		XSetForeground (myDisplay, dbGC, BlackPixel (myDisplay, myScreen));
	else 	
		XSetForeground (myDisplay, dbGC, getColor (myDisplay, 222, 222, 222));
	
	for (lcount = 0; lcount < input_data.lcount; lcount++)
		for (pcount = 1; pcount < input_data.lines[lcount].pcount; pcount++)
			XDrawLine (myDisplay, doublebuf, dbGC, 
			   input_data.lines[lcount].points[pcount-1].x,
			   input_data.lines[lcount].points[pcount-1].y, 
			   input_data.lines[lcount].points[pcount].x,
			   input_data.lines[lcount].points[pcount].y); 

	XCopyArea(myDisplay, doublebuf, myWindow, myGC, 0, 0, window_width, window_height, 0, 0);	
		
	XFreeGC(myDisplay, dbGC);
	XFreePixmap(myDisplay, doublebuf);
		
	clear = FALSE;
}


void initGraphics (void)
{
	XSetWindowAttributes myWindowAttributes;
	int                  myDepth;
	XGCValues            myGCValues;
	unsigned long        myWindowMask;
	Atom                 window_type_atom;
	Atom                 window_type_toolbar_atom;
	Atom                 mwm_atom;
	XWMHints             *wm_hints;
	
	PropMotifWmHints *mwm_hints;

	myDisplay = XOpenDisplay ("");
	if (myDisplay == NULL)
	{
		fprintf (stderr, "Rosetta failed to open display\n");
		exit (0);
	}

	myScreen = DefaultScreen (myDisplay);
	myDepth  = DefaultDepth  (myDisplay, myScreen);
	assignDefault ();
	window_width =  DisplayWidth(myDisplay, myScreen);

	window_type_atom =
		XInternAtom (myDisplay, "_NET_WM_WINDOW_TYPE", False);
	window_type_toolbar_atom =
		XInternAtom (myDisplay, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
	mwm_atom = XInternAtom (myDisplay, "_MOTIF_WM_HINTS", False);

	XSetErrorHandler (x_error_handler);

	myWindowAttributes.border_pixel     = BlackPixel (myDisplay, myScreen);
	myWindowAttributes.background_pixel = getColor (myDisplay, 252, 234, 212);

	myWindowMask = CWBackPixel | CWBorderPixel;

	myWindow = XCreateSimpleWindow (myDisplay,
					RootWindow (myDisplay, myScreen),
					window_x,
					window_y,
					window_width, window_height, 0,
					BlackPixel (myDisplay, myScreen),
					WhitePixel (myDisplay, myScreen));

	XSelectInput (myDisplay, myWindow,
		      ExposureMask | ButtonPressMask |
		      ButtonReleaseMask | PointerMotionMask |
		      StructureNotifyMask);

	XStoreName (myDisplay, myWindow, "Rosetta");

	XChangeProperty (myDisplay, myWindow, window_type_atom, XA_ATOM, 32,
			 PropModeReplace, (unsigned char *) &window_type_toolbar_atom, 1);

	/* Tell the WM we dont want no borders */
	mwm_hints = malloc (sizeof (PropMotifWmHints));
	memset (mwm_hints, 0, sizeof (PropMotifWmHints));

	mwm_hints->flags = MWM_HINTS_DECORATIONS;
	mwm_hints->decorations = 0;

	XChangeProperty (myDisplay, myWindow, mwm_atom,
			 XA_ATOM, 32, PropModeReplace,
			 (unsigned char *) mwm_hints,
			 PROP_MOTIF_WM_HINTS_ELEMENTS);

	free (mwm_hints);

	wm_hints = XAllocWMHints ();
	wm_hints->input = False;
	wm_hints->flags = InputHint;
	XSetWMHints (myDisplay, myWindow, wm_hints);

	myGC = XCreateGC (myDisplay, myWindow, (unsigned long) 0,
			  &myGCValues);
	if (myGC == 0)
	{
		XDestroyWindow (myDisplay, myScreen);
		exit (0);
	}

	XSetForeground (myDisplay, myGC, BlackPixel (myDisplay, myScreen));

	myFont = XLoadQueryFont (myDisplay, "*helvetica-medium-r-*-*-12*");
	if (myFont == (XFontStruct *)NULL)
	{
		fprintf(stderr,"Cannot get font: *helvetica-medium-r-*-*-12*.\n");
		myFont = NULL;
		myFont = XLoadQueryFont (myDisplay, "*13*");
		if (myFont == (XFontStruct *)NULL)
		{
			fprintf(stderr,"Cannot get font: *13*.\n");
			myFont = NULL;
		}
	}

	if (myFont != NULL) XSetFont (myDisplay, myGC, myFont->fid);
	
	XMapWindow (myDisplay, myWindow);

	XFree (wm_hints);
	draw(TRUE);
	
	setupKeyboardVariables(myDisplay);	
}

// Graphics section ends here


void init(void)
{
	memset(&input_data, 0, sizeof(input_data));
	memset(&analized_data, 0, sizeof(analized_data));
	
	input_data.lcount = 0;
	input_data.timeout = WRITE_TIMEOUT;	
}

void add_point (int x, int y)
{
	input_data.timeout = WRITE_TIMEOUT;
	
	input_data.lines[input_data.lcount - 1].pcount++;
	input_data.lines[input_data.lcount - 1].points[input_data.lines[input_data.lcount - 1].pcount - 1].x = x;
	input_data.lines[input_data.lcount - 1].points[input_data.lines[input_data.lcount - 1].pcount - 1].y = y;

	if (input_data.lines[input_data.lcount - 1].pcount == 1)
	{ 
//		gdk_draw_point (pixmap, window->style->black_gc, x, y);
	}
	else
	{
		XSetLineAttributes (myDisplay, myGC, 3, LineSolid,
			    CapRound, JoinMiter);

		XDrawLine (myDisplay, myWindow, myGC, 
			   input_data.lines[input_data.lcount - 1].points[input_data.lines[input_data.lcount - 1].pcount - 2].x,
			   input_data.lines[input_data.lcount - 1].points[input_data.lines[input_data.lcount - 1].pcount - 2].y, x, y);
	}
}


Time getTimeStamp (void)
{
	int tint;
	struct timeval tv;
	struct timezone tz;
	gettimeofday (&tv, &tz);
	tint = (int) tv.tv_sec * 1000;
	tint = tint / 1000 * 1000;
	tint = tint + tv.tv_usec / 1000;
	return (Time) tint;
}

void send_keypress(KeySym ks)
{
	struct keycodeEntry vk_keycodes[10];
	
	if (ks == 0) return; /* no keysym defined, abort */

	if (lookupKeyCodeSequence(ks, vk_keycodes, NULL))
		sendKeySequence(vk_keycodes, FALSE, FALSE, FALSE, FALSE);
}

void write_timeout (void)
{
	KeySym read_symbols[MAX_LINES+1];
#ifdef DEBUG
	int    elapsed_time;
#endif
	int    count;
	int    symbolcount;
	char   temp_filename[100];
	if (input_data.lcount > 0) 
	{
		input_data.timeout = WRITE_TIMEOUT;		

		analize_strokes(&input_data, &analized_data);
			
		if (mode == MODE_LEARNING)
		{	
#ifdef DEBUG
			show_statistic();
#else
			clear = TRUE;				
#endif				
			analized_data.chars[0].symbol = XStringToKeysym(configuration.keys[keycount].symbol[0]);
			add_to_stroke_database(&analized_data.chars[0], &stroke_database);
			input_data.lcount = 0;
			draw(TRUE);
			repeat++;
			if (repeat >= configuration.keys[keycount].repeat)
			{
				keycount++;
				repeat = 0;
			}
			if (keycount >= configuration.keycount) 
			{
				keycount = 0;
				mode = MODE_RECOGNITION;
				printf("Saving database...\n");
				sprintf(temp_filename, "%s/.rosetta/rosetta.db", getenv("HOME"));
				save_stroke_database(temp_filename, &stroke_database);							
			} else 
				printf("Enter character %s\n", configuration.keys[keycount].text[0]);
		} else
		{
#ifdef DEBUG
			elapsed_time = getTimeStamp();
#endif
			match(&stroke_database, &analized_data);
			memset(read_symbols, 0, sizeof(read_symbols));
			symbolcount = generate_symbol_list(&analized_data, read_symbols);
#ifdef DEBUG
			elapsed_time = abs(elapsed_time - getTimeStamp());
			printf("Time: %d\n", elapsed_time);
#endif

			for (count = 0; count < symbolcount; count++)
			{
				send_keypress(read_symbols[count]);
			}
			draw(FALSE);
#ifdef DEBUG
			show_statistic();
#else
			clear = TRUE;
#endif
		}
		init();
	}
}

void penDown (PenEvent * pe)
{
	if (clear) draw(TRUE);
	input_data.lcount++;
	input_data.lines[input_data.lcount - 1].pcount = 0;
	input_data.lines[input_data.lcount - 1].dotcount = 0;
	input_data.lines[input_data.lcount - 1].multi  = FALSE;
	input_data.lines[input_data.lcount - 1].reference = 0xff;
	input_data.lines[input_data.lcount - 1].newword = FALSE;
	
	//add_point (pe->x, pe->y);
	pendown = TRUE;
}

void penUp (PenEvent * pe)
{
	start_time = getTimeStamp();
	
	timeout_timer = 1;
	
	pendown = FALSE;
}

void penMoved (PenEvent * pe)
{
	if (pendown)
	{
		add_point (pe->x, pe->y);
	}
}

void handleEvent (int x, int y, int buttonStatus)
{
	PenEvent pe;
	pe.x = x;
	pe.y = y;

	switch (buttonStatus)
	{
	case PEN_MOVE:
		penMoved (&pe);
		break;
	case PEN_UP:
		penUp (&pe);
		break;
	case PEN_DOWN:
		penDown (&pe);
		break;
	}
}

int checkEvent (XEvent * ev)
{
	int now;
	
	do
	{
		if (XCheckWindowEvent (myDisplay, myWindow,
				       ButtonPressMask | PointerMotionMask,
				       ev)) 
			return TRUE;
		
		now = getTimeStamp();
	}
	while (((now - start_time) < WRITE_TIMEOUT) && (now >= start_time)); 	/* Wait until write_timeout */
	return FALSE;
}


void eventLoop (void)
{
	int x = 0, y = 0;
	XEvent report;
	int status = PEN_UP;

	/* Select event types wanted 
	 * XSelectInput( myDisplay, myWindow, 
	 * ExposureMask | ButtonPressMask | 
	 * ButtonReleaseMask | PointerMotionMask |
	 * StructureNotifyMask );
	 */
	while (1)
	{
		if (timeout_timer)
		{
			if (!checkEvent (&report))
			{
				report.type = 0;
				write_timeout ();
				timeout_timer = 0;
			}
			else if (report.type == ButtonPress)  // timeout is running, clear timeout again
			{
				timeout_timer = 0;				
			}
		}
		else
			XNextEvent (myDisplay, &report);

		switch (report.type)
		{
			case 0:	/* No event occured */
				break;
			case Expose:
				/* unless this is the last contiguous expose,
				 * don't draw the window */
				if (report.xexpose.count != 0)
					break;
				if (report.xexpose.window == myWindow)
					draw (TRUE);
				break;
			case ButtonPress:
				handleEvent (x, y, status = PEN_DOWN);
				break;
			case ButtonRelease:
				handleEvent (x, y, status = PEN_UP);
				break;
			case LeaveNotify:
				break;
			case EnterNotify:
				break;
			case MotionNotify:
				x = report.xmotion.x;
				y = report.xmotion.y;
				if (status == PEN_DOWN)
					handleEvent (x, y, PEN_MOVE);
				break;
			case ConfigureNotify:
				if (report.xconfigure.width != window_width
				    || report.xconfigure.height != window_height)
				{
					window_width = report.xconfigure.width;
					window_height = report.xconfigure.height;
					draw (TRUE);
				}
				break;
			default:
				break;
		}
	}
}

void handle_sig(int sig)
{
	XWindowAttributes attr;
	XGetWindowAttributes(myDisplay, myWindow, &attr);
	
	if (attr.map_state == IsUnmapped || attr.map_state == IsUnviewable )
	{
		XMapWindow(myDisplay, myWindow);
		XRaiseWindow(myDisplay, myWindow);
	} else 
	{
		XUnmapWindow(myDisplay, myWindow);
	}
}

void show_usage_info(const char* errormessage)
{
        printf("\n");
	printf("Rosetta V%s\n", PROGRAM_VERSION);
        printf("(c) 2003 kernel concepts\n\n");
        printf("Usage: rosetta <options>\n\nOptions:\n");
        printf("  -p               Do not use precalculations\n");
        printf("  -P               Calculate and save precalculations\n");
        printf("  -d <filename>    Use a dictionary to correct the recognized words\n");
	printf("  -c <filename>    Use config file <filename> [default = rosetta.conf]");
	printf("\n");
	printf("  -l               Enter learning mode\n");
        printf("%s\n", errormessage);
        exit(1);
}


int main (int argc, char **argv)
{
	int      learning = FALSE;
	int      count;
	int      c;
	char     conf_filename[100];
	char     db_filename[100];
	char     temp_filename[100];
	
        while(  ( c = getopt(argc, argv,"pPd:c:l?") ) != EOF )
	{
                switch(c)
		{
                        // precalculation usage
                        case 'p':       flag_use_precalc = FALSE;
					break;
                        // precalculate 
                        case 'P':       precalc(); save_precalc(PACKAGE_DATA_DIR"rosetta/rosetta.pc"); exit(0);
                                        break;
                        // dictionary
                        case 'd':       flag_use_dict = TRUE;
					strncpy(dict_file_name, optarg, 99);
                                        break;
                        // configuration file
                        case 'c':       strncpy(conf_file_name, optarg, 99);
                                        break;
                        // learning mode
                        case 'l':       learning = TRUE;
                                        break;
                        case '?':
                        default:        show_usage_info("");
                                        break;
                }
        }	
	
	sprintf(temp_filename, "%s/.rosetta/", getenv("HOME"));
	
	if (access(temp_filename, F_OK) != 0) 
		mkdir(temp_filename, 0755);
	
	memset(&stroke_database, 0, sizeof(stroke_database));
	memset(&configuration, 0, sizeof(configuration));
	
	sprintf(temp_filename, "%s/.rosetta/rosetta.conf", getenv("HOME"));
	if (access(temp_filename, F_OK) == 0) strcpy(conf_filename, temp_filename); else
	if (access("/etc/rosetta.conf", F_OK) == 0)       strcpy(conf_filename, "/etc/rosetta.conf"); else
		strcpy(conf_filename, "rosetta.conf");
	
	sprintf(temp_filename, "%s/.rosetta/rosetta.db", getenv("HOME"));
	if (access(temp_filename, F_OK) == 0)   strcpy(db_filename, temp_filename); else
	if (access(PACKAGE_DATA_DIR"rosetta/rosetta.db", F_OK) == 0)   strcpy(db_filename, PACKAGE_DATA_DIR"rosetta/rosetta.db"); else
		strcpy(db_filename, "rosetta.db");
	
	if (!load_configfile(conf_filename)) show_usage_info("");
	
	if (learning)
	{
		mode = MODE_LEARNING; 
	} else 
	{
		mode = MODE_RECOGNITION;
		if (!load_stroke_database(db_filename, &stroke_database))
		{
			mode = MODE_LEARNING;
			printf("Entered leaning mode.\n");
		}
	}	
	
	if (flag_use_dict)	
		if (!load_dictionary(dict_file_name))
			show_usage_info("Can not load dictionary");
	
	if (flag_use_precalc) 
		if (!load_precalc(PACKAGE_DATA_DIR"rosetta/rosetta.pc"))
		{
			printf("Precalculations will be calculated and saved.\n");
			precalc();
			save_precalc(PACKAGE_DATA_DIR"rosetta/rosetta.pc");
		}


	//system("stty cbreak");
	printf("Rosetta V%s\n", PROGRAM_VERSION);
        printf("(c) 2003 kernel concepts\n\n");

	init ();
	keycount = 0;	
		
	if (mode == MODE_LEARNING) 
		printf("Enter character %s\n", configuration.keys[keycount].text[0]);
	
	signal(SIGUSR1, handle_sig); /* for extenal mapping / unmapping */

        initGraphics ();
        eventLoop ();

	//system("stty -cbreak");
	if (dictionary != NULL) 
	{
		for (count = 0; count < MAX_WORDS; count++) free(dictionary[count]);
		free(dictionary);
	}		
		
        return 0;
}
