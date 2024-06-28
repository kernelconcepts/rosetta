/*
 learn.c - main file of the gtk based leaning utility for rosetty
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

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <fcntl.h>
#include <locale.h>
#include <signal.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <X11/extensions/shape.h>
#include <gpe/init.h>
#include <gpe/pixmaps.h>
//#include <gpe/render.h>
#include <gpe/smallbox.h>
#include <gpe/errorbox.h>
#include <gpe/question.h>

#include "libvirtkeys.h"
#include "rosetta.h"
#include "gui_learn.h"
#include "callbacks.h"

int     flag_use_precalc = TRUE;
int     flag_use_dict    = FALSE;
int     mode;
int     learn_mode       = MODE_LEARN_SINGLE;
char    conf_file_name[100];
int     timeout_timer    = 0;
int     learn_set_group_mode  = LEARN_SET_GROUP_DB;

void show_usage_info(const char* errormessage)
{
        printf("\n");
	printf("Rosetta Trainer V%s\n", PROGRAM_VERSION);
        printf("(c) 2003 kernel concepts\n\n");
        printf("Usage: rosettatrain <options>\n\nOptions:\n");
        printf("  -p               Do not use precalculations\n");
        printf("  -P               Calculate and save precalculations\n");
	printf("  -c <filename>    Use config file <filename> [default = rosetta.conf]");
	printf("\n");
        printf("%s\n", errormessage);
        exit(1);
}

int
main (int argc, char *argv[])
{
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
                        case 'P':       precalc(); save_precalc(PACKAGE_DATA_DIR"rosetta/rosetta.pc");
                                        break;
                        // configuration file
                        case 'c':       strncpy(conf_file_name, optarg, 99);
                                        break;
                        case '?':
                        default:        show_usage_info("");
                                        break;
                }
        }	
	
	memset(&stroke_database, 0, sizeof(stroke_database));
	memset(&configuration, 0, sizeof(configuration));
	keycount = 0;
	
	sprintf(temp_filename, "%s/.rosetta/", getenv("HOME"));
	
	if (access(temp_filename, F_OK) != 0) 
		mkdir(temp_filename, 0755);
	
	sprintf(temp_filename, "%s/.rosetta/rosetta.conf", getenv("HOME"));
	if (access(temp_filename, F_OK) == 0) strcpy(conf_filename, temp_filename); else
	if (access("/etc/rosetta.conf", F_OK) == 0)       strcpy(conf_filename, "/etc/rosetta.conf"); else
		strcpy(conf_filename, "rosetta.conf");
	
	sprintf(temp_filename, "%s/.rosetta/rosetta.db", getenv("HOME"));
	if (access(temp_filename, F_OK) == 0)   strcpy(db_filename, temp_filename); else
	if (access(PACKAGE_DATA_DIR"rosetta/rosetta.db", F_OK) == 0)   strcpy(db_filename, PACKAGE_DATA_DIR"rosetta/rosetta.db"); else
		strcpy(db_filename, "rosetta.db");
	
	if (!load_configfile(conf_filename)) show_usage_info("");
	
	mode = MODE_LEARNING;
	if (!load_stroke_database(db_filename, &stroke_database))
	{
		generate_emty_stroke_database_from_config(&stroke_database);
		printf("Neet to learn from scratch.\n");
	}

	if (flag_use_precalc) 
		if (!load_precalc(PACKAGE_DATA_DIR"rosetta/rosetta.pc"))
		{
			printf("Precalculations will be calculated and saved.\n");
			precalc();
			save_precalc(PACKAGE_DATA_DIR"rosetta/rosetta.pc");
		}
	
	//system("stty cbreak");
	printf("Rosetta Trainer V%s\n", PROGRAM_VERSION);
        printf("(c) 2003 kernel concepts\n\n");

	init ();	
	
	setlocale (LC_ALL, "");
        bindtextdomain (GTK_PACKAGE, PACKAGE_LOCALE_DIR);
        bind_textdomain_codeset (GTK_PACKAGE, "UTF-8");
        textdomain (GTK_PACKAGE);

        if (gpe_application_init (&argc, &argv) == FALSE)	// instead of gtk_init();
                exit (1);

	window = create_mainwindow();
		
	gtk_widget_show_all (window);

	draw(TRUE);
	draw_from_db();
	setupKeyboardVariables(gdk_x11_get_default_xdisplay());			
	gtk_main ();
	
	return 0;
}
