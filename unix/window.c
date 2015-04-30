// 28 april 2015
#include "uipriv_unix.h"

struct window {
	uiWindow w;

	GtkWidget *widget;
	GtkContainer *container;
	GtkWindow *window;

	GtkWidget *vboxWidget;
	GtkContainer *vboxContainer;
	GtkBox *vbox;

	GtkWidget *menubar;

	uiContainer *bin;

	int hidden;

	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
	int margined;
};

static gboolean onClosing(GtkWidget *win, GdkEvent *e, gpointer data)
{
	struct window *w = (struct window *) data;

	// manually destroy the window ourselves; don't let the delete-event handler do it
	if ((*(w->onClosing))(uiWindow(w), w->onClosingData))
		uiControlDestroy(uiControl(w));
	// don't continue to the default delete-event handler; we destroyed the window by now
	return TRUE;
}

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 1;
}

static void windowDestroy(uiControl *c)
{
	struct window *w = (struct window *) c;

	// first hide ourselves
	gtk_widget_hide(w->widget);
	// now destroy the bin
	// the bin has no parent, so we can just call uiControlDestroy()
	uiControlDestroy(uiControl(w->bin));
	// now destroy the menus
	freeMenubar(w->menubar);
	// now destroy ourselves
	// this will also free the vbox
	gtk_widget_destroy(w->widget);
	uiFree(w);
}

static uintptr_t windowHandle(uiControl *c)
{
	struct window *w = (struct window *) c;

	return (uintptr_t) (w->window);
}

static void windowSetParent(uiControl *c, uiContainer *parent)
{
	complain("attempt to give the uiWindow at %p a parent", c);
}

static void windowPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	complain("attempt to get the preferred size of the uiWindow at %p", c);
}

static void windowResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	complain("attempt to resize the uiWindow at %p", c);
}

static int windowVisible(uiControl *c)
{
	struct window *w = (struct window *) c;

	return !w->hidden;
}

static void windowShow(uiControl *c)
{
	struct window *w = (struct window *) c;

	// don't use gtk_widget_show_all() as that will show all children, regardless of user settings
	gtk_widget_show(w->widget);
	w->hidden = 0;
}

static void windowHide(uiControl *c)
{
	struct window *w = (struct window *) c;

	gtk_widget_hide(w->widget);
	w->hidden = 1;
}

static void windowEnable(uiControl *c)
{
	struct window *w = (struct window *) c;

	gtk_widget_set_sensitive(w->widget, TRUE);
}

static void windowDisable(uiControl *c)
{
	struct window *w = (struct window *) c;

	gtk_widget_set_sensitive(w->widget, FALSE);
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

static void windowOnClosing(uiWindow *ww, int (*f)(uiWindow *, void *), void *data)
{
	struct window *w = (struct window *) ww;

	w->onClosing = f;
	w->onClosingData = data;
}

static void windowSetChild(uiWindow *ww, uiControl *child)
{
	struct window *w = (struct window *) ww;

	binSetMainControl(w->bin, child);
}

static int windowMargined(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	return w->margined;
}

static void windowSetMargined(uiWindow *ww, int margined)
{
	struct window *w = (struct window *) ww;

	w->margined = margined;
	if (w->margined)
		binSetMargins(w->bin, gtkXMargin, gtkYMargin, gtkXMargin, gtkYMargin);
	else
		binSetMargins(w->bin, 0, 0, 0, 0);
}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	struct window *w;
	GtkWidget *binWidget;

	w = uiNew(struct window);

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

	w->bin = newBin();
	binWidget = GTK_WIDGET(uiControlHandle(uiControl(w->bin)));
	gtk_widget_set_hexpand(binWidget, TRUE);
	gtk_widget_set_halign(binWidget, GTK_ALIGN_FILL);
	gtk_widget_set_vexpand(binWidget, TRUE);
	gtk_widget_set_valign(binWidget, GTK_ALIGN_FILL);
	binSetParent(w->bin, (uintptr_t) (w->vboxContainer));

	// show everything in the vbox, but not the GtkWindow itself
	gtk_widget_show_all(w->vboxWidget);

	// and connect our OnClosing() event
	g_signal_connect(w->widget, "delete-event", G_CALLBACK(onClosing), w);
	w->onClosing = defaultOnClosing;

	uiControl(w)->Destroy = windowDestroy;
	uiControl(w)->Handle = windowHandle;
	uiControl(w)->SetParent = windowSetParent;
	uiControl(w)->PreferredSize = windowPreferredSize;
	uiControl(w)->Resize = windowResize;
	uiControl(w)->Visible = windowVisible;
	uiControl(w)->Show = windowShow;
	uiControl(w)->Hide = windowHide;
	uiControl(w)->Enable = windowEnable;
	uiControl(w)->Disable = windowDisable;

	uiWindow(w)->Title = windowTitle;
	uiWindow(w)->SetTitle = windowSetTitle;
	uiWindow(w)->OnClosing = windowOnClosing;
	uiWindow(w)->SetChild = windowSetChild;
	uiWindow(w)->Margined = windowMargined;
	uiWindow(w)->SetMargined = windowSetMargined;

	return uiWindow(w);
}
