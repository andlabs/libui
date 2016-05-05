// 4 september 2015
#include "dtp.h"

// #qo pkg-config: gtk+-3.0

int main(void)
{
	GtkWidget *mainwin;
	GtkWidget *box;
	GtkWidget *button;

	gtk_init(NULL, NULL);

	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(mainwin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(mainwin), 12);
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	gtk_container_add(GTK_CONTAINER(mainwin), box);
	button = newDTP();
	gtk_container_add(GTK_CONTAINER(box), button);
	button = newDP();
	gtk_container_add(GTK_CONTAINER(box), button);
	button = newTP();
	gtk_container_add(GTK_CONTAINER(box), button);
	gtk_container_add(GTK_CONTAINER(box), gtk_entry_new());

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
