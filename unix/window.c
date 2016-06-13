// 11 june 2015
#include "uipriv_unix.h"

struct uiWindow {
	uiUnixControl c;

	GtkWidget *widget;
	GtkContainer *container;
	GtkWindow *window;

	GtkWidget *vboxWidget;
	GtkContainer *vboxContainer;
	GtkBox *vbox;

	GtkWidget *menubar;

	struct child *child;
	int margined;
	int fullscreen_enabled;

	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
};

static gboolean onClosing(GtkWidget *win, GdkEvent *e, gpointer data)
{
	uiWindow *w = uiWindow(data);

	// manually destroy the window ourselves; don't let the delete-event handler do it
	if ((*(w->onClosing))(w, w->onClosingData))
		uiControlDestroy(uiControl(w));
	// don't continue to the default delete-event handler; we destroyed the window by now
	return TRUE;
}

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 0;
}

static void uiWindowDestroy(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	// first hide ourselves
	gtk_widget_hide(w->widget);
	// now destroy the child
	if (w->child != NULL)
		childDestroy(w->child);
	// now destroy the menus, if any
	if (w->menubar != NULL)
		freeMenubar(w->menubar);
	gtk_widget_destroy(w->vboxWidget);
	// and finally free ourselves
	g_object_unref(w->widget);
	uiFreeControl(uiControl(w));
}

uiUnixControlDefaultHandle(uiWindow)

uiControl *uiWindowParent(uiControl *c)
{
	return NULL;
}

void uiWindowSetParent(uiControl *c, uiControl *parent)
{
	uiUserBugCannotSetParentOnToplevel("uiWindow");
}

static int uiWindowToplevel(uiControl *c)
{
	return 1;
}

uiUnixControlDefaultVisible(uiWindow)

static void uiWindowShow(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	// don't use gtk_widget_show_all() as that will show all children, regardless of user settings
	// don't use gtk_widget_show(); that doesn't bring to front or give keyboard focus
	// (gtk_window_present() does call gtk_widget_show() though)
	gtk_window_present(w->window);
}

uiUnixControlDefaultHide(uiWindow)
uiUnixControlDefaultEnabled(uiWindow)
uiUnixControlDefaultEnable(uiWindow)
uiUnixControlDefaultDisable(uiWindow)
// TODO?
uiUnixControlDefaultSetContainer(uiWindow)

char *uiWindowTitle(uiWindow *w)
{
	return uiUnixStrdupText(gtk_window_get_title(w->window));
}

void uiWindowSetTitle(uiWindow *w, const char *title)
{
	gtk_window_set_title(w->window, title);
}

void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *, void *), void *data)
{
	w->onClosing = f;
	w->onClosingData = data;
}

void uiWindowSetChild(uiWindow *w, uiControl *child)
{
	if (w->child != NULL)
		childRemove(w->child);
	w->child = newChildWithBox(child, uiControl(w), w->vboxContainer, w->margined);
	if (w->child != NULL) {
		gtk_widget_set_hexpand(childBox(w->child), TRUE);
		gtk_widget_set_halign(childBox(w->child), GTK_ALIGN_FILL);
		gtk_widget_set_vexpand(childBox(w->child), TRUE);
		gtk_widget_set_valign(childBox(w->child), GTK_ALIGN_FILL);
	}
}

int uiWindowMargined(uiWindow *w)
{
	return w->margined;
}

void uiWindowSetMargined(uiWindow *w, int margined)
{
	w->margined = margined;
	if (w->child != NULL)
		childSetMargined(w->child, w->margined);
}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	uiWindow *w;

	uiUnixNewControl(uiWindow, w);

	w->widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	w->container = GTK_CONTAINER(w->widget);
	w->window = GTK_WINDOW(w->widget);

	gtk_window_set_title(w->window, title);
	gtk_window_resize(w->window, width, height);

	w->vboxWidget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	w->vboxContainer = GTK_CONTAINER(w->vboxWidget);
	w->vbox = GTK_BOX(w->vboxWidget);
	w->fullscreen_enabled = 0;

	// set the vbox as the GtkWindow child
	gtk_container_add(w->container, w->vboxWidget);

	if (hasMenubar) {
		w->menubar = makeMenubar(uiWindow(w));
		gtk_container_add(w->vboxContainer, w->menubar);
	}

	// show everything in the vbox, but not the GtkWindow itself
	gtk_widget_show_all(w->vboxWidget);

	// and connect our OnClosing() event
	g_signal_connect(w->widget, "delete-event", G_CALLBACK(onClosing), w);
	uiWindowOnClosing(w, defaultOnClosing, NULL);

	// normally it's SetParent() that does this, but we can't call SetParent() on a uiWindow
	// TODO we really need to clean this up
	g_object_ref(w->widget);

	return w;
}

void uiWindowSize(uiWindow *w, int *width, int *height)
{
	gtk_window_get_size(GTK_WINDOW(w->widget), width, height);
}

void uiWindowSetSize(uiWindow *w, int width, int height)
{
	gtk_window_resize(GTK_WINDOW(w->widget), width, height);
}

void uiWindowPosition(uiWindow *w, int *x, int *y)
{
	gtk_window_get_position(GTK_WINDOW(w->widget), x, y);
}

void uiWindowSetPosition(uiWindow *w, int x, int y)
{
	gtk_window_move(GTK_WINDOW(w->widget), x, y);
}

void uiWindowCenter(uiWindow *w)
{
	GdkWindow *root = gtk_widget_get_root_window(GTK_WIDGET(w->widget));
	gint width, height, rwidth, rheight;
	gtk_window_get_size(GTK_WINDOW(w->widget), &width, &height);
	gdk_window_get_geometry(root, NULL, NULL, &rwidth, &rheight);
	gtk_window_move(GTK_WINDOW(w->widget), (rwidth-width)/2, (rheight-height)/2);
}

int uiWindowFullscreen(uiWindow *w)
{
	return w->fullscreen_enabled;
}

void uiWindowToggleFullscreen(uiWindow *w)
{
	w->fullscreen_enabled = !w->fullscreen_enabled;
	if(w->fullscreen_enabled)
		gtk_window_fullscreen(GTK_WINDOW(w->widget));
	else
		gtk_window_unfullscreen(GTK_WINDOW(w->widget));
}
