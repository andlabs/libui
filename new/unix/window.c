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

static gboolean onClosing(GtkWidget *win, GdkEvent *e, gpointer data)
{
	struct window *w = (struct window *) data;

	// manually destroy the window ourselves; don't let the delete-event handler do it
	if ((*(w->onClosing))(uiWindow(w), w->onClosingData))
		uiWindowDestroy(uiWindow(w));
	// don't continue to the default delete-event handler; we destroyed the window by now
	return TRUE;
}

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 1;
}

static void windowDestroy(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	// first, hide the window to avoid flicker
	gtk_widget_hide(w->widget);
	// next, remove the uiOSContainer from the vbox
	gtk_container_remove(w->vboxcontainer, GTK_WIDGET(uiOSContainerHandle(w->content)));
	// next, destroy the uiOSContainer, which will destroy its child widget
	uiOSContainerDestroy(w->content);
	// TODO menus
	// next, destroy the GtkWindow itself, which will destroy the vbox, menus, etc.
	gtk_widget_destroy(w->widget);
	// finally, free ourselves
	uiFree(w);
}

static uintptr_t windowHandle(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	return (uintptr_t) (w->widget);
}

static char *windowTitle(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	return strdupText(gtk_window_get_title(w->window));
}

static void windowSetTitle(uiWindow *ww, const char *title)
{
	struct window *w = (struct window *) ww;

	gtk_window_set_title(w->window, title);
}

static void windowShow(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	// don't use gtk_widget_show_all(); that will override user hidden settings
	gtk_widget_show(w->widget);
}

static void windowHide(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	gtk_widget_hide(w->widget);
}

static void windowOnClosing(uiWindow *ww, int (*f)(uiWindow *, void *), void *data)
{
	struct window *w = (struct window *) ww;

	w->onClosing = f;
	w->onClosingData = data;
}

static void windowSetChild(uiWindow *ww, uiControl *c)
{
	struct window *w = (struct window *) ww;

	// TODO make the update implicit
	uiOSContainerSetMainControl(w->content, c);
	uiOSContainerUpdate(w->content);
}

static int windowMargined(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	return w->margined;
}

static void windowSetMargined(uiWindow *ww, int margined)
{
	struct window *w = (struct window *) ww;

	// TODO make the update implicit
	w->margined = margined;
	if (w->margined)
		uiOSContainerSetMargins(w->content, gtkXMargin, gtkYMargin, gtkXMargin, gtkYMargin);
	else
		uiOSContainerSetMargins(w->content, 0, 0, 0, 0);
	uiOSContainerUpdate(w->content);
}

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
	gtk_widget_set_valign(GTK_WIDGET(w->content), GTK_ALIGN_FILL);

	// show everything in the vbox, but not the GtkWindow itself
	gtk_widget_show_all(w->vboxwidget);

	// and connect our OnClosing() event
	g_signal_connect(w->widget, "delete-event", G_CALLBACK(onClosing), w);
	w->onClosing = defaultOnClosing;

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
