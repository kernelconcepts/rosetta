#ifndef _gui_learn_h_
#define _gui_learn_h_

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

extern	GtkWidget *MainWindow;
extern	GdkPixbuf *MainWindow_icon_pixbuf;
extern	GtkWidget *swMain;
extern	GtkWidget *vpMain;
extern	GtkWidget *vbMain;
extern	GtkWidget *tbMain;
extern	GtkWidget *imgInput;
extern	GtkWidget *hsSeperator;
extern	GtkWidget *nbMain;
extern	GtkWidget *vbTraining;
extern	GtkWidget *lblActualChar;
extern	GtkWidget *hs2;
extern	GtkWidget *hbCharacter;
extern	GtkWidget *vbDescription;
extern	GtkWidget *lblKeyName;
extern	GtkWidget *lblChar;
extern	GtkWidget *lblKeySym;
extern	GtkWidget *lblKS;
extern	GtkWidget *lblGroup;
extern	GtkWidget *lblGrp;
extern	GtkWidget *imgShow;
extern	GtkWidget *vs1;
extern	GtkWidget *hbLearnOptions;
extern	GtkWidget *rbOverwrite;
extern	GSList    *rbOverwrite_group;
extern	GtkWidget *rbAdd;
extern	GtkWidget *lblTrain;
extern	GtkWidget *swTextinput;
extern	GtkWidget *twTextinput;
extern	GtkWidget *lblTest;
extern  GtkWidget *sbStatus;

extern	GtkWidget *window;
extern	GdkPixmap *pixmap;
extern	GdkPixmap *pixmap_show;
extern	GtkWidget *imgInput_EventBox;


GtkWidget *create_mainwindow (void);

#endif
