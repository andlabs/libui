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

	uiControl *child;
	GtkWidget *childbox;
	int margined;

	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
};

static void onDestroy(uiWindow *);

uiUnixDefineControlWithOnDestroy(
	uiWindow,							// type name
	uiWindowType,							// type function
	onDestroy(this);						// on destroy
)

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

static void onDestroy(uiWindow *w)
{
	// first hide ourselves
	gtk_widget_hide(w->widget);
	// now destroy the child
	uiControlSetParent(w->child, NULL);
	uiControlDestroy(w->child);
	// now destroy the menus, if any
	if (w->menubar != NULL)
		freeMenubar(w->menubar);
	gtk_widget_destroy(w->vboxWidget);
}

static void windowCommitShow(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	// don't use gtk_widget_show_all() as that will show all children, regardless of user settings
	// don't use gtk_widget_show(); that doesn't bring to front or give keyboard focus
	// (gtk_window_present() does call gtk_widget_show() though)
	gtk_window_present(w->window);
}

static void windowContainerUpdateState(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	if (w->child != NULL)
		controlUpdateState(w->child);
}

char *uiWindowTitle(uiWindow *w)
{
	return uiUnixStrdupText(gtk_window_get_title(w->window));
}

void uiWindowSetTitle(uiWindow *w, const char *title)
{
	gtk_window_set_title(w->window, title);
	// don't queue resize; the caption isn't part of what affects layout and sizing of the client area (it'll be ellipsized if too long)
}

void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *, void *), void *data)
{
	w->onClosing = f;
	w->onClosingData = data;
}

void uiWindowSetChild(uiWindow *w, uiControl *child)
{
	if (w->child != NULL) {
		gtk_container_remove(GTK_CONTAINER(w->childbox),
			GTK_WIDGET(uiControlHandle(w->child)));
		uiControlSetParent(w->child, NULL);
	}
	w->child = child;
	if (w->child != NULL) {
		uiControlSetParent(w->child, uiControl(w));
		gtk_container_add(GTK_CONTAINER(w->childbox),
			GTK_WIDGET(uiControlHandle(w->child)));
	}
}

int uiWindowMargined(uiWindow *w)
{
	return w->margined;
}

void uiWindowSetMargined(uiWindow *w, int margined)
{
	w->margined = margined;
	setMargined(GTK_CONTAINER(w->childbox), w->margined);
}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	uiWindow *w;

	w = (uiWindow *) uiNewControl(uiWindowType());

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

	w->childbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_hexpand(w->childbox, TRUE);
	gtk_widget_set_halign(w->childbox, GTK_ALIGN_FILL);
	gtk_widget_set_vexpand(w->childbox, TRUE);
	gtk_widget_set_valign(w->childbox, GTK_ALIGN_FILL);
	gtk_container_add(w->vboxContainer, w->childbox);

	// show everything in the vbox, but not the GtkWindow itself
	gtk_widget_show_all(w->vboxWidget);

	// and connect our OnClosing() event
	g_signal_connect(w->widget, "delete-event", G_CALLBACK(onClosing), w);
	uiWindowOnClosing(w, defaultOnClosing, NULL);

	uiUnixFinishNewControl(w, uiWindow);
	uiControl(w)->CommitShow = windowCommitShow;
	uiControl(w)->ContainerUpdateState = windowContainerUpdateState;

	return w;
}
