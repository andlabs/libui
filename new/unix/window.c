// 22 april 2015
#include "uipriv_unix.h"

struct window {
	uiWindow w;

	// the window itself, preconverted to the various GTK+ types
	GtkWidget *widget;
	GtkContainer *container;
	GtkWindow *window;

	// the main content widget of the GtkWindow
	GtkWidget *vboxwidget;
	GtkContainer *vboxcontainer;
	GtkBox *vbox;

	// the OS container for the uiWindow
	uiOSContainer *content;

	int margined;
};

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubars)
{
	struct window *w;

	w = uiNew(struct window);

	w->widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	w->container = GTK_CONTAINER(w->widget);
	w->window = GTK_WINDOW(w->widget);

	w->vboxwidget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	w->vboxcontainer = GTK_CONTAINER(w->vboxwidget);
	w->vbox = GTK_BOX(w->vboxwidget);

	// set the vbox as the GtkWindow child
	gtk_container_add(w->container, w->vboxwidget);

	// TODO menus

	// and add the OS container
	w->content = uiNewOSContainer((uintptr_t) (w->vboxcontainer));
	gtk_widget_set_hexpand(GTK_WIDGET(w->content), TRUE);
	gtk_widget_set_halign(GTK_WIDGET(w->content), GTK_ALIGN_FILL);
	gtk_widget_set_vexpand(GTK_WIDGET(w->content), TRUE);
	gtk_widget_set_valign(GTK_WIDGET(w->content), GTK_ALL_FILL);

	// show everything in the vbox, but not the GtkWindow itself
	gtk_widget_show_all(w->vboxwidget);

	// and connect our OnClosing() event
	g_signal_connect(w->TODO

	uiWindow(w)->Destroy = windowDestroy;
	uiWindow(w)->Handle = windowHandle;
	uiWindow(w)->Title = windowTitle;
	uiWindow(w)->SetTitle = windowSetTitle;
	uiWindow(w)->Show = windowShow;
	uiWindow(w)->Hide = windowHide;
	uiWindow(w)->OnClosing = windowOnClosing;
	uiWindow(w)->SetChild = windowSetChild;
	uiWindow(w)->Margined = windowMargined;
	uiWindow(w)->SetMargined = windowSetMargined;

	return uiWindow(w);
}
