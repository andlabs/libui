// 6 april 2015
#include "uipriv_unix.h"

struct uiWindow {
	GtkWidget *widget;
	uiParent *content;
	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
	int margined;
};

static gboolean onClosing(GtkWidget *win, GdkEvent *e, gpointer data)
{
	uiWindow *w = (uiWindow *) data;

	// return exact values just in case
	if ((*(w->onClosing))(w, w->onClosingData))
		return FALSE;
	return TRUE;
}

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 1;
}

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
	g_signal_connect(w->widget, "delete-event", G_CALLBACK(onClosing), w);
	g_signal_connect(w->widget, "destroy", G_CALLBACK(onDestroy), w);
	w->content = uiNewParent((uintptr_t) (w->widget));
	w->onClosing = defaultOnClosing;
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
	// don't use gtk_widget_show_all(); that will override user hidden settings
	gtk_widget_show(w->widget);
}

void uiWindowHide(uiWindow *w)
{
	gtk_widget_hide(w->widget);
}

void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *, void *), void *data)
{
	w->onClosing = f;
	w->onClosingData = data;
}

void uiWindowSetChild(uiWindow *w, uiControl *c)
{
	uiParentSetChild(w->content, c);
	uiParentUpdate(w->content);
}

int uiWindowMargined(uiWindow *w)
{
	return w->margined;
}

void uiWindowSetMargined(uiWindow *w, int margined)
{
	w->margined = margined;
	if (w->margined)
		uiParentSetMargins(w->content, gtkXMargin, gtkYMargin, gtkXMargin, gtkYMargin);
	else
		uiParentSetMargins(w->content, 0, 0, 0, 0);
	uiParentUpdate(w->content);
}
