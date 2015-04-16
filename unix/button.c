// 7 april 2015
#include "uipriv_unix.h"

struct button {
	uiButton b;
	void (*onClicked)(uiControl *, void *);
	void *onClickedData;
};

static void onClicked(GtkButton *button, gpointer data)
{
	struct button *b = (struct button *) data;

	(*(b->onClicked))(uiButton(b), b->onClickedData);
}

static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
}

static void onDestroy(GtkWidget *widget, gpointer data)
{
	struct button *b = (struct button *) data;

	uiFree(b);
}

#define BUTTON(b) GTK_BUTTON(uiControlHandle(uiControl(b)))

static char *getText(uiButton *b)
{
	return g_strdup(gtk_button_get_label(BUTTON(b)));
}

static void setText(uiButton *b, const char *text)
{
	gtk_button_set_label(BUTTON(b), text);
}

static void setOnClicked(uiButton *b, void (*f)(uiControl *, void *), void *data)
{
	struct button *b = (struct button *) b;

	b->onClicked = f;
	b->onClickedData = data;
}

uiControl *uiNewButton(const char *text)
{
	struct button *b;
	GtkWidget *widget;

	b = uiNew(b);

	uiUnixNewControl(uiControl(b), GTK_TYPE_BUTTON,
		FALSE, FALSE,
		"label", text,
		NULL);

	widget = GTK_WIDGET(BUTTON(b));
	g_signal_connect(widget, "clicked", G_CALLBACK(onClicked), b);
	g_signal_connect(widget, "destroy", G_CALLBACK(onDestroy), b);
	b->onClicked = defaultOnClicked;

	uiButton(b)->Text = getText;
	uiButton(b)->SetText = setText;
	uiButton(b)->OnClicked = setOnClicked;

	return b;
}
