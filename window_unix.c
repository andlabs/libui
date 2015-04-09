// 6 april 2015
#include "uipriv_unix.h"

struct uiWindow {
	GtkWidget *widget;
	GtkWidget *container;
	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
};

static void onDestroy(GtkWidget *widget, gpointer data)
{
	uiWindow *w = (uiWindow *) data;

	uiFree(w);
}

uiWindow *uiNewWindow(char *title, int width, int height)
{
	uiWindow *w;

	w = uiNew(uiWindow);
	w->widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(w->widget), title);
	gtk_window_resize(GTK_WINDOW(w->widget), width, height);
	g_signal_connect(w->widget, "destroy", G_CALLBACK(onDestroy), w);
	w->container = newContainer();
	gtk_container_add(GTK_CONTAINER(w->widget), w->container);
	return w;
}

void uiWindowDestroy(uiWindow *w)
{
	gtk_widget_destroy(w->widget);
}

uintptr_t uiWindowHandle(uiWindow *w)
{
	return (uintptr_t) (w->widget);
}

char *uiWindowTitle(uiWindow *w)
{
	return g_strdup(gtk_window_get_title(GTK_WINDOW(w->widget)));
}

void uiWindowSetTitle(uiWindow *w, const char *title)
{
	gtk_window_set_title(GTK_WINDOW(w->widget), title);
}

void uiWindowShow(uiWindow *w)
{
	gtk_widget_show_all(w->widget);
}

void uiWindowHide(uiWindow *w)
{
	gtk_widget_hide(w->widget);
}

static gboolean onClosing(GtkWidget *win, GdkEvent *e, gpointer data)
{
	uiWindow *w = (uiWindow *) data;

	// return exact values just in case
	if ((*(w->onClosing))(w, w->onClosingData))
		return FALSE;
	return TRUE;
}

void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *, void *), void *data)
{
	w->onClosing = f;
	w->onClosingData = data;
	g_signal_connect(w->widget, "delete-event", G_CALLBACK(onClosing), w);
}

void uiWindowSetChild(uiWindow *w, uiControl *c)
{
	uiContainer(w->container)->child = c;
	(*(c->setParent))(c, (uintptr_t) (w->container));
}

// TODO margined

void uiWindowSetMargined(uiWindow *w, int margined)
{
	uiContainer(w->container)->margined = margined;
	updateParent((uintptr_t) (w->container));
}
