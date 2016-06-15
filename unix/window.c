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

	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
	void (*onPositionChanged)(uiWindow *, void *);
	void *onPositionChangedData;
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

static void defaultOnPositionChanged(uiWindow *w, void *data)
{
	// do nothing
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

// TODO allow specifying either as NULL on all platforms
void uiWindowPosition(uiWindow *w, int *x, int *y)
{
	gint rx, ry;

	gtk_window_get_position(w->window, &rx, &ry);
	*x = rx;
	*y = ry;
}

void uiWindowSetPosition(uiWindow *w, int x, int y)
{
	gtk_window_move(w->window, x, y);
}

// TODO after calling this I have to call get_position() a few times before it actually works
void uiWindowCenter(uiWindow *w)
{
	gint x, y;
	GtkAllocation winalloc;
	GdkWindow *gdkwin;
	GdkScreen *screen;
	GdkRectangle workarea;

	gtk_widget_get_allocation(w->widget, &winalloc);
	gdkwin = gtk_widget_get_window(w->widget);
	screen = gdk_window_get_screen(gdkwin);
	gdk_screen_get_monitor_workarea(screen,
		gdk_screen_get_monitor_at_window(screen, gdkwin),
		&workarea);

	x = (workarea.width - winalloc.width) / 2;
	y = (workarea.height - winalloc.height) / 2;
	// TODO move up slightly? see what Mutter or GNOME Shell does?
	gtk_window_move(w->window, x, y);
}

// TODO find a signal to connect to
void uiWindowOnPositionChanged(uiWindow *w, void (*f)(uiWindow *, void *), void *data)
{
	w->onPositionChanged = f;
	w->onPositionChangedData = data;
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

	// set the vbox as the GtkWindow child
	gtk_container_add(w->container, w->vboxWidget);

	if (hasMenubar) {
		w->menubar = makeMenubar(uiWindow(w));
		gtk_container_add(w->vboxContainer, w->menubar);
	}

	// show everything in the vbox, but not the GtkWindow itself
	gtk_widget_show_all(w->vboxWidget);

	// and connect our events
	g_signal_connect(w->widget, "delete-event", G_CALLBACK(onClosing), w);
	uiWindowOnClosing(w, defaultOnClosing, NULL);
	uiWindowOnPositionChanged(w, defaultOnPositionChanged, NULL);

	// normally it's SetParent() that does this, but we can't call SetParent() on a uiWindow
	// TODO we really need to clean this up
	g_object_ref(w->widget);

	return w;
}
