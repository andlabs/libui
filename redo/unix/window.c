// 11 june 2015
#include "uipriv_unix.h"

// TODO ban uiControl methods that don't apply

struct window {
	uiWindow w;

	GtkWidget *widget;
	GtkContainer *container;
	GtkWindow *window;

	GtkWidget *vboxWidget;
	GtkContainer *vboxContainer;
	GtkBox *vbox;

	GtkWidget *menubar;

	uiControl *child;

	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
	int margined;
	void (*baseCommitDestroy)(uiControl *c);
};

uiDefineControlType(uiWindow, uiTypeWindow, struct window)

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
	return 0;
}

static void windowCommitDestroy(uiControl *c)
{
	struct window *w = (struct window *) c;

	// first hide ourselves
	gtk_widget_hide(w->widget);
	// now destroy the child
	uiControlSetParent(w->child, NULL);
	uiControlDestroy(w->child);
	// now destroy the menus, if any
	if (w->menubar != NULL)
		freeMenubar(w->menubar);
	// now destroy ourselves
	// this will also free the vbox
	(*(w->baseCommitDestroy))(uiControl(w));
}

static uintptr_t windowHandle(uiControl *c)
{
	struct window *w = (struct window *) c;

	return (uintptr_t) (w->widget);
}

static void windowCommitShow(uiControl *c)
{
	struct window *w = (struct window *) c;

	// don't use gtk_widget_show_all() as that will show all children, regardless of user settings
	// don't use gtk_widget_show(); that doesn't bring to front or give keyboard focus
	// (gtk_window_present() does call gtk_widget_show() though)
	gtk_window_present(w->window);
}

static void windowContainerUpdateState(uiControl *c)
{
	struct window *w = (struct window *) c;

	if (w->child != NULL)
		uiControlUpdateState(w->child);
}

static char *windowTitle(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	return uiUnixStrdupText(gtk_window_get_title(w->window));
}

static void windowSetTitle(uiWindow *ww, const char *title)
{
	struct window *w = (struct window *) ww;

	gtk_window_set_title(w->window, title);
	// don't queue resize; the caption isn't part of what affects layout and sizing of the client area (it'll be ellipsized if too long)
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

	if (w->child != NULL)
		uiControlSetParent(w->child, NULL);
	w->child = child;
	if (w->child != NULL) {
//TODO		uiControlSetParent(w->child, w->child);
		gtk_widget_set_hexpand(GTK_WIDGET(uiControlHandle(w->child)), TRUE);
		gtk_widget_set_halign(GTK_WIDGET(uiControlHandle(w->child)), GTK_ALIGN_FILL);
		gtk_widget_set_vexpand(GTK_WIDGET(uiControlHandle(w->child)), TRUE);
		gtk_widget_set_valign(GTK_WIDGET(uiControlHandle(w->child)), GTK_ALIGN_FILL);
		gtk_container_add(w->vboxContainer, GTK_WIDGET(uiControlHandle(w->child)));
		uiControlQueueResize(w->child);
	}
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
	// TODO
	uiControlQueueResize(uiControl(w));
}

static void windowResizeChild(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	if (w->child == NULL)
		return;
	PUT_CODE_HERE;
}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	struct window *w;
	GtkWidget *holderWidget;

	w = (struct window *) uiNewControl(uiTypeWindow());

	w->widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	w->container = GTK_CONTAINER(w->widget);
	w->window = GTK_WINDOW(w->widget);

	uiUnixMakeSingleWidgetControl(uiControl(w), w->widget);

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

	// and connect our OnClosing() event
	g_signal_connect(w->widget, "delete-event", G_CALLBACK(onClosing), w);

	w->onClosing = defaultOnClosing;

	uiControl(w)->Handle = windowHandle;
	w->baseCommitDestroy = uiControl(w)->CommitDestroy;
	uiControl(w)->CommitDestroy = windowCommitDestroy;
	uiControl(w)->CommitShow = windowCommitShow;
	uiControl(w)->ContainerUpdateState = windowContainerUpdateState;

	uiWindow(w)->Title = windowTitle;
	uiWindow(w)->SetTitle = windowSetTitle;
	uiWindow(w)->OnClosing = windowOnClosing;
	uiWindow(w)->SetChild = windowSetChild;
	uiWindow(w)->Margined = windowMargined;
	uiWindow(w)->SetMargined = windowSetMargined;
	uiWindow(w)->ResizeChild = windowResizeChild;

	return uiWindow(w);
}
