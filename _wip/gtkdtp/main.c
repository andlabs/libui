// 4 september 2015
#include "dtp.h"

// #qo pkg-config: gtk+-3.0

int main(void)
{
	GtkWidget *mainwin;
	GtkWidget *dtp;

	gtk_init(NULL, NULL);

	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(mainwin), 12);
	g_signal_connect(mainwin, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	dtp = GTK_WIDGET(g_object_new(dateTimePickerWidget_get_type(), NULL));
	gtk_container_add(GTK_CONTAINER(mainwin), dtp);

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
