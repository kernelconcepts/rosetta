#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gpe/init.h>
#include <gpe/pixmaps.h>
//#include <gpe/render.h>
#include <gpe/smallbox.h>
#include <gpe/errorbox.h>
#include <gpe/question.h>

#include "rosetta.h"
#include "gui_learn.h"
#include "callbacks.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  gtk_object_set_data_full (GTK_OBJECT (component), name, \
    gtk_widget_ref (widget), (GtkDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  gtk_object_set_data (GTK_OBJECT (component), name, widget)

static struct gpe_icon icons[] = {
	{"learn", PREFIX "/share/pixmaps/penwrite24.png"},
	{"icon", PREFIX "/share/pixmaps/rosetta.png"},
	{NULL, NULL}
};

GtkWidget *MainWindow;
GdkPixbuf *MainWindow_icon_pixbuf;
GtkWidget *swMain;
GtkWidget *vpMain;
GtkWidget *vbMain;
GtkWidget *tbMain;
GtkWidget *imgInput;
GtkWidget *hsSeperator;
GtkWidget *nbMain;
GtkWidget *vbTraining;
GtkWidget *lblActualChar;
GtkWidget *hs2;
GtkWidget *hbCharacter;
GtkWidget *vbDescription;
GtkWidget *lblKeyName;
GtkWidget *lblChar;
GtkWidget *lblKeySym;
GtkWidget *lblKS;
GtkWidget *lblGroup;
GtkWidget *lblGrp;
GtkWidget *imgShow;
GtkWidget *vs1;
GtkWidget *hbLearnOptions;
GtkWidget *lblTrain;
GtkWidget *swTextinput;
GtkWidget *twTextinput;
GtkWidget *lblTest;
GtkWidget *sbStatus;

GtkWidget *window;
GdkPixmap *pixmap;
GdkPixmap *pixmap_show;
GtkWidget *imgInput_EventBox;


GtkWidget *
create_mainwindow (void)
{
	char      text[30];
	GtkWidget *learn;
	
        if (gpe_load_icons (icons) == FALSE)
                exit (1);	
	
	// Mainwindow with scrollbox / viewport and the button-bar

	MainWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_widget_realize (MainWindow);
	gtk_window_set_title (GTK_WINDOW (MainWindow), _("Rosetta Trainer"));
	gtk_window_set_default_size (GTK_WINDOW (MainWindow), 240, 300);
	gpe_set_window_icon(MainWindow, "icon");

	swMain = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (swMain);
	gtk_container_add (GTK_CONTAINER (MainWindow), swMain);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swMain),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	vpMain = gtk_viewport_new (NULL, NULL);
	gtk_widget_show (vpMain);
	gtk_container_add (GTK_CONTAINER (swMain), vpMain);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (vpMain), GTK_SHADOW_NONE);

	vbMain = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbMain);
	gtk_container_add (GTK_CONTAINER (vpMain), vbMain);

	// inserting the toolbar	
        tbMain = gtk_toolbar_new ();
        gtk_toolbar_set_orientation (GTK_TOOLBAR (tbMain), GTK_ORIENTATION_HORIZONTAL);
        gtk_toolbar_set_style (GTK_TOOLBAR (tbMain), GTK_TOOLBAR_ICONS);

        gtk_box_pack_start (GTK_BOX (vbMain), tbMain, FALSE, FALSE, 0);
        gtk_widget_show (tbMain);

	gtk_toolbar_insert_stock (GTK_TOOLBAR (tbMain), GTK_STOCK_GOTO_FIRST,_("First"),
                _("First"), (GtkSignalFunc) on_btnFirst_clicked, NULL, -1);

	gtk_toolbar_insert_stock (GTK_TOOLBAR (tbMain), GTK_STOCK_GO_BACK,_("Previous"),
                _("Previous"), (GtkSignalFunc) on_btnPrevious_clicked, NULL, -1);

	gtk_toolbar_insert_stock (GTK_TOOLBAR (tbMain), GTK_STOCK_GO_FORWARD,_("Next"),
                _("Next"), (GtkSignalFunc) on_btnNext_clicked, NULL, -1);

	gtk_toolbar_insert_stock (GTK_TOOLBAR (tbMain), GTK_STOCK_GOTO_LAST,_("Last"),
                _("Last"), (GtkSignalFunc) on_btnLast_clicked, NULL, -1);

	gtk_toolbar_append_space (GTK_TOOLBAR (tbMain));
	
	learn = gtk_image_new_from_pixbuf(gpe_find_icon ("learn"));
	gtk_toolbar_append_item (GTK_TOOLBAR (tbMain), _("Learn"),
		_("Learn"), _("Learn"), learn, (GtkSignalFunc) on_btnLearn_clicked, NULL);
		
	gtk_toolbar_append_space (GTK_TOOLBAR (tbMain));

	gtk_toolbar_insert_stock (GTK_TOOLBAR (tbMain), GTK_STOCK_QUIT,_("Exit"),
                _("Exit"), (GtkSignalFunc) on_window_destroy, NULL, -1);
	
	// end of toolbar
	
	// Creating now the Image with the input pixmap
	pixmap = gdk_pixmap_new (MainWindow->window, WIDTH, HEIGHT, -1);
	imgInput = gtk_image_new_from_pixmap (pixmap, NULL);
	gtk_widget_show(imgInput);
	imgInput_EventBox = gtk_event_box_new ();
	gtk_container_add (GTK_CONTAINER (imgInput_EventBox), imgInput);
	gtk_container_add (GTK_CONTAINER (vbMain), imgInput_EventBox);
	gtk_widget_set_size_request (imgInput, WIDTH, HEIGHT);
	gtk_misc_set_alignment (GTK_MISC (imgInput), 0, 0);	
	// Creation of the input image ends here

	hsSeperator = gtk_hseparator_new ();
	gtk_widget_show (hsSeperator);
	gtk_box_pack_start (GTK_BOX (vbMain), hsSeperator, FALSE, TRUE, 0);

	nbMain = gtk_notebook_new ();
	gtk_widget_show (nbMain);
	gtk_box_pack_start (GTK_BOX (vbMain), nbMain, TRUE, TRUE, 0);

	vbTraining = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbTraining);
	gtk_container_add (GTK_CONTAINER (nbMain), vbTraining);
	gtk_container_set_border_width (GTK_CONTAINER (vbTraining), 5);

	sprintf(text, "<b>%s</b>", _("Actual character / symbol"));
	lblActualChar = gtk_label_new (NULL);
	gtk_label_set_markup(GTK_LABEL(lblActualChar), text);
	gtk_widget_show (lblActualChar);
	gtk_box_pack_start (GTK_BOX (vbTraining), lblActualChar, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lblActualChar), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lblActualChar), 0, 0);
	gtk_misc_set_padding (GTK_MISC (lblActualChar), 0, 5);

	hs2 = gtk_hseparator_new ();
	gtk_widget_show (hs2);
	gtk_box_pack_start (GTK_BOX (vbTraining), hs2, FALSE, TRUE, 0);

	hbCharacter = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbCharacter);
	gtk_box_pack_start (GTK_BOX (vbTraining), hbCharacter, TRUE, TRUE, 0);

	vbDescription = gtk_vbox_new (TRUE, 0);
	gtk_widget_show (vbDescription);
	gtk_box_pack_start (GTK_BOX (hbCharacter), vbDescription, FALSE,
			    FALSE, 0);

	sprintf(text, "<i>%s</i>", _("Character:"));
	lblKeyName = gtk_label_new (NULL);
	gtk_label_set_markup(GTK_LABEL(lblKeyName), text);	
	gtk_widget_show (lblKeyName);
	gtk_box_pack_start (GTK_BOX (vbDescription), lblKeyName, FALSE, FALSE,
			    0);
	gtk_label_set_justify (GTK_LABEL (lblKeyName), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lblKeyName), 0, 0);

	lblChar = gtk_label_new ("Space");
	gtk_widget_show (lblChar);
	gtk_box_pack_start (GTK_BOX (vbDescription), lblChar, FALSE, FALSE,
			    0);
	gtk_label_set_justify (GTK_LABEL (lblChar), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lblChar), 0, 0);
	gtk_misc_set_padding (GTK_MISC (lblChar), 15, 0);

	sprintf(text, "<i>%s</i>", _("KeySym:"));
	lblKeySym = gtk_label_new (NULL);
	gtk_label_set_markup(GTK_LABEL(lblKeySym), text);	
	gtk_widget_show (lblKeySym);
	gtk_box_pack_start (GTK_BOX (vbDescription), lblKeySym, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lblKeySym), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lblKeySym), 0, 0);

	lblKS = gtk_label_new ("0x0000");
	gtk_widget_show (lblKS);
	gtk_box_pack_start (GTK_BOX (vbDescription), lblKS, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lblKS), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lblKS), 0, 0);
	gtk_misc_set_padding (GTK_MISC (lblKS), 15, 0);

	sprintf(text, "<i>%s</i>", _("Group:"));
	lblGroup = gtk_label_new (NULL);
	gtk_label_set_markup(GTK_LABEL(lblGroup), text);	
	gtk_widget_show (lblGroup);
	gtk_box_pack_start (GTK_BOX (vbDescription), lblGroup, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lblGroup), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lblGroup), 0, 0);

	lblGrp = gtk_label_new ("Lower");
	gtk_widget_show (lblGrp);
	gtk_box_pack_start (GTK_BOX (vbDescription), lblGrp, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lblGrp), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lblGrp), 0, 0);
	gtk_misc_set_padding (GTK_MISC (lblGrp), 15, 0);

	pixmap_show = gdk_pixmap_new (MainWindow->window, NORM_WIDTH, NORM_HEIGHT, -1);
	gdk_draw_rectangle(pixmap_show, MainWindow->style->white_gc, TRUE, 0, 0, NORM_WIDTH, NORM_HEIGHT); 
	imgShow = gtk_image_new_from_pixmap (pixmap_show, NULL);
	gtk_widget_show (imgShow);
	gtk_box_pack_end (GTK_BOX (hbCharacter), imgShow, FALSE, TRUE, 0);	
	gtk_widget_set_size_request (imgShow, NORM_WIDTH, NORM_HEIGHT);

	vs1 = gtk_vseparator_new ();
	gtk_widget_show (vs1);
	gtk_box_pack_end (GTK_BOX (hbCharacter), vs1, FALSE, TRUE, 4);

	hbLearnOptions = gtk_hbox_new (TRUE, 0);
	gtk_widget_show (hbLearnOptions);
	gtk_box_pack_start (GTK_BOX (vbTraining), hbLearnOptions, TRUE, TRUE, 0);

	lblTrain = gtk_label_new (_("Training"));
	gtk_widget_show (lblTrain);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (nbMain),
		gtk_notebook_get_nth_page (GTK_NOTEBOOK(nbMain), 0), lblTrain);
	gtk_label_set_justify (GTK_LABEL (lblTrain), GTK_JUSTIFY_LEFT);

	swTextinput = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (swTextinput);
	gtk_container_add (GTK_CONTAINER (nbMain), swTextinput);

	twTextinput = gtk_text_view_new ();
	gtk_widget_show (twTextinput);
	gtk_container_add (GTK_CONTAINER (swTextinput), twTextinput);

	lblTest = gtk_label_new (_("Test"));
	gtk_widget_show (lblTest);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (nbMain),
		gtk_notebook_get_nth_page (GTK_NOTEBOOK(nbMain), 1), lblTest);
	gtk_label_set_justify (GTK_LABEL (lblTest), GTK_JUSTIFY_LEFT);
	
	sbStatus = gtk_statusbar_new ();
	gtk_widget_show (sbStatus);
	gtk_box_pack_start (GTK_BOX (vbMain), sbStatus, FALSE, FALSE, 0);
	gtk_statusbar_push(GTK_STATUSBAR(sbStatus), 0,
			    _("Write a char to overwrite saved pattern."));
	
	gtk_signal_connect (GTK_OBJECT (MainWindow), "destroy",
			    GTK_SIGNAL_FUNC (on_window_destroy), NULL);

	gtk_signal_connect (GTK_OBJECT (imgInput_EventBox),
			    "button-press-event",
			    GTK_SIGNAL_FUNC (button_press_event), NULL);
	gtk_signal_connect (GTK_OBJECT (imgInput_EventBox),
			    "button-release-event",
			    GTK_SIGNAL_FUNC (button_release_event), NULL);
	gtk_signal_connect (GTK_OBJECT (imgInput_EventBox),
			    "motion-notify-event",
			    GTK_SIGNAL_FUNC (motion_notify), NULL);
	gtk_signal_connect_after (GTK_OBJECT (nbMain), 
			    "switch_page",
			    GTK_SIGNAL_FUNC (on_nbMain_switch_page), NULL);
		    
	gtk_widget_add_events (GTK_WIDGET (imgInput_EventBox),
			       GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
			       | GDK_POINTER_MOTION_MASK);	

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT_NO_REF (MainWindow, MainWindow, "MainWindow");
	GLADE_HOOKUP_OBJECT (MainWindow, swMain, "swMain");
	GLADE_HOOKUP_OBJECT (MainWindow, vpMain, "vpMain");
	GLADE_HOOKUP_OBJECT (MainWindow, vbMain, "vbMain");
	GLADE_HOOKUP_OBJECT (MainWindow, tbMain, "tbMain");
	GLADE_HOOKUP_OBJECT (MainWindow, imgInput, "imgInput");
	GLADE_HOOKUP_OBJECT (MainWindow, hsSeperator, "hsSeperator");
	GLADE_HOOKUP_OBJECT (MainWindow, nbMain, "nbMain");
	GLADE_HOOKUP_OBJECT (MainWindow, vbTraining, "vbTraining");
	GLADE_HOOKUP_OBJECT (MainWindow, lblActualChar, "lblActualChar");
	GLADE_HOOKUP_OBJECT (MainWindow, hs2, "hs2");
	GLADE_HOOKUP_OBJECT (MainWindow, hbCharacter, "hbCharacter");
	GLADE_HOOKUP_OBJECT (MainWindow, vbDescription, "vbDescription");
	GLADE_HOOKUP_OBJECT (MainWindow, lblKeyName, "lblKeyName");
	GLADE_HOOKUP_OBJECT (MainWindow, lblChar, "lblChar");
	GLADE_HOOKUP_OBJECT (MainWindow, lblKeySym, "lblKeySym");
	GLADE_HOOKUP_OBJECT (MainWindow, lblKS, "lblKS");
	GLADE_HOOKUP_OBJECT (MainWindow, lblGroup, "lblGroup");
	GLADE_HOOKUP_OBJECT (MainWindow, lblGrp, "lblGrp");

	GLADE_HOOKUP_OBJECT (MainWindow, imgShow, "imgShow");
	GLADE_HOOKUP_OBJECT (MainWindow, vs1, "vs1");
	GLADE_HOOKUP_OBJECT (MainWindow, hbLearnOptions, "hbLearnOptions");
	GLADE_HOOKUP_OBJECT (MainWindow, lblTrain, "lblTrain");
	GLADE_HOOKUP_OBJECT (MainWindow, swTextinput, "swTextinput");
	GLADE_HOOKUP_OBJECT (MainWindow, twTextinput, "twTextinput");
	GLADE_HOOKUP_OBJECT (MainWindow, lblTest, "lblTest");
	GLADE_HOOKUP_OBJECT (MainWindow, sbStatus, "sbStatus");

	return MainWindow;
}
