// 6 april 2015
#include "ui_unix.h"

struct uiWindow {
	GtkWidget *widget;
	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
};

uiWindow *uiNewWindow(char *title, int width, int height)
{
	uiWindow *w;

	w = g_new0(uiWindow, 1);
	w->widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(w->widget), title);
	gtk_window_resize(GTK_WINDOW(w->widget), width, height);
	return w;
}

void uiWindowDestroy(uiWindow *w)
{
	gtk_widget_destroy(w->widget);
	g_free(w);
}

uintptr_t uiWindowHandle(uiWindow *w)
{
	return (uintptr_t) (w->widget);
}

// TODO titles

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
