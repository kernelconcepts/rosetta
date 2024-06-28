#ifndef _callbacks_h_
#define _callbacks_h_



extern t_data_struct input_data;
extern t_charlist    analized_data;
extern t_database    stroke_database;
extern t_config      configuration;

void init(void);
void draw(int mode);
void draw_from_db(void);
void on_btnFirst_clicked (GtkButton *button, gpointer user_data);
void on_btnPrevious_clicked (GtkButton *button, gpointer user_data);
void on_btnNext_clicked (GtkButton *button, gpointer user_data);
void on_btnLast_clicked (GtkButton *button, gpointer user_data);
void on_btnLearn_clicked (GtkButton *button, gpointer user_data);
void motion_notify (GtkWidget * widget, GdkEventMotion * motion, gpointer * data);
void button_press_event (GtkWidget * widget, GdkEventButton * button, gpointer * data);
void button_release_event (GtkWidget * widget, GdkEventButton * button, gpointer * data);
void on_nbMain_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, gint page_num, gpointer user_data);
void on_window_destroy (GtkObject * object, gpointer user_data);

#endif
