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

#include "rosetta.h"
#include "callbacks.h"
#include "libvirtkeys.h"
#include "gui_learn.h"


#define       BAR_HEIGHT 18

GdkColor      black    = {0, 0, 0, 0};
GdkColor      white    = {0, 65535, 65535, 65535};
GdkColor      gray     = {0, 57045, 57045, 57045};
GdkColor      darkgray = {0, 51400, 51400, 51400};
GdkColor      red      = {0, 65535, 0, 0 };
GdkColor      green    = {0, 0, 65535, 0 };
GdkColor      blue     = {0, 0, 0, 65535 };
GdkColor      purple   = {0, 65535, 0, 65535 };
GdkColor      fontbg   = {0, 23040, 46080, 51400};
GdkColor      fontfg   = {0, 15360, 30720, 51400};
 
t_data_struct input_data;
t_charlist    analized_data;
t_database    stroke_database;
t_config      configuration;

int           window_width = WIDTH;
int           window_height = HEIGHT;

int           keycount;
int           start_time;
int           timeout_handler = 0;
int           clear   = FALSE;
int           pendown = FALSE;

void init(void)
{
	memset(&input_data, 0, sizeof(input_data));
	memset(&analized_data, 0, sizeof(analized_data));
	
	input_data.lcount = 0;
	input_data.timeout = WRITE_TIMEOUT;	
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

void
redraw (void)
{
	gtk_widget_queue_draw (imgInput);
}

void
redraw_show (void)
{
	gtk_widget_queue_draw (imgShow);
}

void draw(int mode)
{
	int lcount;
	int pcount;

	GdkGC   *gc;
	GdkFont *font;
	
	clear = TRUE;
	
	gc = gdk_gc_new (GDK_DRAWABLE(pixmap));
	
	font = gdk_font_load("*helvetica-medium-r-*-*-12*");
	if (font == (GdkFont *)NULL)
	{
		fprintf(stderr,"Cannot get font: *helvetica-medium-r-*-*-12*.\n");
		font = NULL;
		font = gdk_font_load("*13*");
		if (font == (GdkFont *)NULL)
		{
			fprintf(stderr,"Cannot get font: *13*.\n");
			font = NULL;
		}
	}
	
	gdk_gc_set_rgb_fg_color(gc, &white);
	gdk_gc_set_line_attributes (gc, 1, GDK_LINE_SOLID, GDK_CAP_ROUND,
					   GDK_JOIN_MITER);	
	
	gdk_draw_rectangle (pixmap, gc, TRUE, 0, 0, WIDTH, HEIGHT);

	gdk_gc_set_rgb_fg_color(gc, &gray);
	gdk_gc_set_rgb_bg_color(gc, &gray);

	gdk_draw_rectangle(pixmap, gc, TRUE, 0, 0, WIDTH, BAR_HEIGHT);

	gdk_gc_set_rgb_fg_color(gc, &darkgray);

	gdk_gc_set_line_attributes (gc, 1, GDK_LINE_ON_OFF_DASH, GDK_CAP_ROUND,
					   GDK_JOIN_MITER);	
	/* inner lines */
	
	gdk_draw_line (pixmap, gc, 0, BAR_HEIGHT, WIDTH, BAR_HEIGHT);
	gdk_draw_line (pixmap, gc, 0,
		   (HEIGHT - BAR_HEIGHT) / 3 + BAR_HEIGHT,
		   WIDTH,
		   (HEIGHT - BAR_HEIGHT) / 3 + BAR_HEIGHT);
	
	gdk_draw_line (pixmap, gc, 0,
		   (HEIGHT - BAR_HEIGHT) * 2 / 3 + BAR_HEIGHT,
		   WIDTH,
		   (HEIGHT - BAR_HEIGHT) * 2 / 3 + BAR_HEIGHT);

	if (gtk_notebook_get_current_page(GTK_NOTEBOOK(nbMain)) == 0)
	{
		gdk_gc_set_rgb_fg_color(gc, &fontbg);
	
		gdk_draw_string(pixmap, font, gc, WIDTH/2 - gdk_string_width(font, "Write character to learn...") / 2 + 1, BAR_HEIGHT - gdk_string_height(font, "Write character to learn...")/2 + 2, "Write character to learn...");
		
		gdk_gc_set_rgb_fg_color(gc, &fontfg);
		
		gdk_draw_string(pixmap, font, gc, WIDTH/2 - gdk_string_width(font, "Write character to learn...") / 2, BAR_HEIGHT - gdk_string_height(font, "Write character to learn...")/2 + 1, "Write character to learn...");
		
	} else
	{
		gdk_draw_line (pixmap, gc, WIDTH / 3,
			   0, WIDTH / 3, HEIGHT);
		gdk_draw_line (pixmap, gc, (WIDTH / 3) * 2,
			   0, (WIDTH / 3) * 2, HEIGHT);
	
	
		gdk_gc_set_rgb_fg_color(gc, &fontbg);
	
		gdk_draw_string(pixmap, font, gc, 5+1, BAR_HEIGHT - gdk_string_height(font, "abc")/2+1, "abc");
	
		gdk_draw_string(pixmap, font, gc, WIDTH/2 - gdk_string_width(font, "ABC")/2 + 1, BAR_HEIGHT - gdk_string_height(font, "ABC")/2 + 1, "ABC");
	
		gdk_draw_string(pixmap, font, gc, WIDTH - gdk_string_width(font, "Symbol") - 5 + 1, BAR_HEIGHT - gdk_string_height(font, "Symbol")/2 + 2, "Symbol");
		
		gdk_gc_set_rgb_fg_color(gc, &fontfg);
		
		gdk_draw_string(pixmap, font, gc, 5, BAR_HEIGHT - gdk_string_height(font, "abc")/2, "abc");
	
		gdk_draw_string(pixmap, font, gc, WIDTH/2 - gdk_string_width(font, "ABC")/2, BAR_HEIGHT - gdk_string_height(font, "ABC")/2, "ABC");
	
		gdk_draw_string(pixmap, font, gc, WIDTH - gdk_string_width(font, "Symbol") - 5, BAR_HEIGHT - gdk_string_height(font, "Symbol")/2+1, "Symbol");
	}
	
	gdk_gc_set_line_attributes (gc, 3, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_MITER);	

	if (mode) 
		gdk_gc_set_rgb_fg_color(gc, &black);
	else 	
		gdk_gc_set_rgb_fg_color(gc, &gray);
	
	for (lcount = 0; lcount < input_data.lcount; lcount++)
		for (pcount = 1; pcount < input_data.lines[lcount].pcount; pcount++)
			gdk_draw_line(pixmap, gc, 
			   input_data.lines[lcount].points[pcount-1].x,
			   input_data.lines[lcount].points[pcount-1].y, 
			   input_data.lines[lcount].points[pcount].x,
			   input_data.lines[lcount].points[pcount].y); 
		
	g_object_unref (G_OBJECT (gc));
	redraw();		
	clear = FALSE;
}

void draw_from_db(void)
{
	int     scount;
	int     pcount;
	char    text[20];

	GdkGC   *gc;
	
	clear = TRUE;
	
	gc = gdk_gc_new (GDK_DRAWABLE(pixmap_show));
	
	gdk_gc_set_rgb_fg_color(gc, &white);
	gdk_draw_rectangle (pixmap_show, gc, TRUE, 0, 0, NORM_WIDTH, NORM_HEIGHT);

	gdk_gc_set_line_attributes (gc, 3, GDK_LINE_SOLID, GDK_CAP_ROUND,
					   GDK_JOIN_MITER);	

	gdk_gc_set_rgb_fg_color(gc, &black);

	for (scount = 0; scount < stroke_database.chars[keycount].strokecount; scount++)
		for (pcount = 1; pcount < TEST_POINT_COUNT; pcount++)
			gdk_draw_line(pixmap_show, gc, 
			   stroke_database.chars[keycount].strokes[scount].points[pcount-1].x,
			   stroke_database.chars[keycount].strokes[scount].points[pcount-1].y, 
			   stroke_database.chars[keycount].strokes[scount].points[pcount].x,
			   stroke_database.chars[keycount].strokes[scount].points[pcount].y); 
		
	g_object_unref (G_OBJECT (gc));
	redraw_show();	

	// Now set correct text for the labels
	
	for (scount = 0; scount < configuration.keycount; scount++)
		if (stroke_database.chars[keycount].symbol == XStringToKeysym(configuration.keys[scount].symbol[0]))		
			gtk_label_set_text(GTK_LABEL(lblChar), configuration.keys[scount].text[0]);
			
	//gtk_label_set_text(GTK_LABEL(lblChar), XKeysymToString(stroke_database.chars[keycount].symbol));
	sprintf(text, "%04x", (uint) stroke_database.chars[keycount].symbol);		
	gtk_label_set_text(GTK_LABEL(lblKS), text);
	switch ((int)stroke_database.chars[keycount].group)
	{
		case    grp_lowercase:
		        gtk_label_set_text(GTK_LABEL(lblGrp), _("Lower case"));
		        break;

		case    grp_uppercase:
		        gtk_label_set_text(GTK_LABEL(lblGrp), _("Upper case"));
		        break;
		
		case    grp_number:
		        gtk_label_set_text(GTK_LABEL(lblGrp), _("Number"));
		        break;

		case    grp_special:
		        gtk_label_set_text(GTK_LABEL(lblGrp), _("Special"));
		        break;

		case    grp_function:
		        gtk_label_set_text(GTK_LABEL(lblGrp), _("Function keys"));
		        break;
	}
	
	clear = FALSE;
}

int write_timeout (gpointer data)
{
	KeySym read_symbols[MAX_LINES+1];
#ifdef DEBUG
	int    elapsed_time;
#endif
	int    count;
	int    symbolcount;
	
	if (input_data.lcount > 0) 
	{
		analize_strokes(&input_data, &analized_data);
			
		if (mode == MODE_LEARNING)
		{	
			clear = TRUE;				
//			analized_data.chars[0].symbol = XStringToKeysym(configuration.keys[keycount].symbol[0]);
			analized_data.chars[0].symbol = stroke_database.chars[keycount].symbol;
			analized_data.chars[0].group  = stroke_database.chars[keycount].group;
			set_stroke_database(&analized_data.chars[0], &stroke_database, keycount);
			input_data.lcount = 0;
			draw(TRUE);
			if ((mode == MODE_LEARNING) && (learn_mode == MODE_LEARN_CONT))
			{
				keycount++;				
				if (keycount >= stroke_database.charcount) keycount = 0;
			}
			draw_from_db();
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
			if (gtk_notebook_get_current_page(GTK_NOTEBOOK(nbMain)) == 1)
			{
				gtk_widget_grab_focus (twTextinput);	
				for (count = 0; count < symbolcount; count++)
				{
					send_keypress(read_symbols[count]);
				}
			}
			draw(FALSE);
			clear = TRUE;
		}
		init();
	}
	timeout_handler = 0;
	return(FALSE);
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
		gdk_gc_set_line_attributes (window->style->black_gc, 3,
				    GDK_LINE_SOLID, GDK_CAP_ROUND,
				    GDK_JOIN_MITER);	

		gdk_draw_line (pixmap, window->style->black_gc,
			       input_data.lines[input_data.lcount - 1].points[input_data.lines[input_data.lcount - 1].pcount - 2].x,
			       input_data.lines[input_data.lcount - 1].points[input_data.lines[input_data.lcount - 1].pcount - 2].y, x, y);
		
		gdk_gc_set_line_attributes (window->style->black_gc, 1, 
				    GDK_LINE_SOLID, GDK_CAP_ROUND, 
				    GDK_JOIN_MITER);		

		redraw();
	}
}

void on_btnFirst_clicked (GtkButton *button, gpointer user_data)
{
	if (gtk_notebook_get_current_page(GTK_NOTEBOOK(nbMain)) != 0)
	{
		gtk_notebook_set_page(GTK_NOTEBOOK(nbMain), 0);
	}
	gtk_statusbar_pop(GTK_STATUSBAR(sbStatus), 0);
	gtk_statusbar_push(GTK_STATUSBAR(sbStatus), 0, _("Write a char to overwrite saved pattern."));
	keycount = 0;
	learn_mode = MODE_LEARN_SINGLE;
	draw_from_db();
}

void on_btnPrevious_clicked (GtkButton *button, gpointer user_data)
{
	if (gtk_notebook_get_current_page(GTK_NOTEBOOK(nbMain)) != 0)
	{
		gtk_notebook_set_page(GTK_NOTEBOOK(nbMain), 0);
	}
	gtk_statusbar_pop(GTK_STATUSBAR(sbStatus), 0);
	gtk_statusbar_push(GTK_STATUSBAR(sbStatus), 0, _("Write a char to overwrite saved pattern."));
	if (keycount > 0) keycount --;
	learn_mode = MODE_LEARN_SINGLE;
	draw_from_db();
}

void on_btnNext_clicked (GtkButton *button, gpointer user_data)
{
	if (gtk_notebook_get_current_page(GTK_NOTEBOOK(nbMain)) != 0)
	{
		gtk_notebook_set_page(GTK_NOTEBOOK(nbMain), 0);
	}
	gtk_statusbar_pop(GTK_STATUSBAR(sbStatus), 0);
	gtk_statusbar_push(GTK_STATUSBAR(sbStatus), 0, _("Write a char to overwrite saved pattern."));
	if (keycount < stroke_database.charcount-1) keycount ++;
	learn_mode = MODE_LEARN_SINGLE;
	draw_from_db();
}

void on_btnLast_clicked (GtkButton *button, gpointer user_data)
{
	if (gtk_notebook_get_current_page(GTK_NOTEBOOK(nbMain)) != 0)
	{
		gtk_notebook_set_page(GTK_NOTEBOOK(nbMain), 0);
	}
	gtk_statusbar_pop(GTK_STATUSBAR(sbStatus), 0);
	gtk_statusbar_push(GTK_STATUSBAR(sbStatus), 0, _("Write a char to overwrite saved pattern."));
	keycount = stroke_database.charcount-1;
	learn_mode = MODE_LEARN_SINGLE;
	draw_from_db();
}
void on_btnLearn_clicked (GtkButton *button, gpointer user_data)
{
	if (gtk_notebook_get_current_page(GTK_NOTEBOOK(nbMain)) != 0)
	{
		gtk_notebook_set_page(GTK_NOTEBOOK(nbMain), 0);
	}
	gtk_statusbar_pop(GTK_STATUSBAR(sbStatus), 0);
	gtk_statusbar_push(GTK_STATUSBAR(sbStatus), 0, _("Automatic learning mode entered..."));
	learn_mode = MODE_LEARN_CONT;
	draw_from_db();
}

void
motion_notify (GtkWidget * widget, GdkEventMotion * motion, gpointer * data)
{
	if (pendown)
	{
		add_point (motion->x, motion->y);
	}
}

void
button_press_event (GtkWidget * widget, GdkEventButton * button,
		    gpointer * data)
{	
	if (clear) draw(TRUE);
	if (timeout_handler != 0) 
	{
		gtk_timeout_remove(timeout_handler);
		timeout_handler = 0;
	}
	
	input_data.lcount++;
	input_data.lines[input_data.lcount - 1].pcount  = 0;
	input_data.lines[input_data.lcount - 1].dotcount = 0;
	input_data.lines[input_data.lcount - 1].multi   = FALSE;
	input_data.lines[input_data.lcount - 1].reference = 0xff;
	input_data.lines[input_data.lcount - 1].newword = FALSE;
	
	add_point (button->x, button->y);
	pendown = TRUE;
}

void
button_release_event (GtkWidget * widget, GdkEventButton * button,
		      gpointer * data)
{
	start_time = getTimeStamp();
	
	timeout_handler = gtk_timeout_add(WRITE_TIMEOUT, write_timeout, NULL);
	
	pendown = FALSE;	
}

void
on_nbMain_switch_page(GtkNotebook *notebook, GtkNotebookPage *page,
                      gint page_num, gpointer user_data)
{
	if (page_num == 0)
	{
		mode = MODE_LEARNING; 
		gtk_statusbar_pop(GTK_STATUSBAR(sbStatus), 0);
		gtk_statusbar_push(GTK_STATUSBAR(sbStatus), 0, _("Write text and try stroke database..."));
	}
	else    
	{
		mode = MODE_RECOGNITION;
		learn_mode = MODE_LEARN_SINGLE;
		gtk_statusbar_pop(GTK_STATUSBAR(sbStatus), 0);
		gtk_statusbar_push(GTK_STATUSBAR(sbStatus), 0, _("Write a char to overwrite saved pattern."));
	}
	draw(FALSE);
	draw_from_db();
}


void
on_window_destroy (GtkObject * object, gpointer user_data)
{
	GtkWidget *dialog;
	int       answer;
	char      temp_filename[100];
	
	dialog = gtk_message_dialog_new (GTK_WINDOW (window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			_("Shall I save the new stroke database?"));
	answer = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	if (answer == GTK_RESPONSE_YES)
	{
		sprintf(temp_filename, "%s/.rosetta/rosetta.db", getenv("HOME"));
		save_stroke_database(temp_filename, &stroke_database);							
	}
	gtk_exit (0);
}
