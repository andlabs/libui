// 7 april 2015
#include "uipriv_unix.h"

struct button {
	void (*onClicked)(uiControl *, void *);
	void *onClickedData;
};

static void onClicked(GtkButton *button, gpointer data)
{
	uiControl *c = (uiControl *) data;
	struct button *b = (struct button *) (c->data);

	(*(b->onClicked))(c, b->onClickedData);
}

static void defaultOnClicked(uiControl *c, void *data)
{
	// do nothing
}

static void onDestroy(GtkWidget *widget, gpointer data)
{
	struct button *b = (struct button *) data;

	uiFree(b);
}

uiControl *uiNewButton(const char *text)
{
	uiControl *c;
	struct button *b;
	GtkWidget *widget;

	c = uiUnixNewControl(GTK_TYPE_BUTTON,
		FALSE, FALSE,
		"label", text,
		NULL);

	widget = GTK_WIDGET(uiControlHandle(c));
	g_signal_connect(widget, "clicked", G_CALLBACK(onClicked), c);

	b = uiNew(struct button);
	g_signal_connect(widget, "destroy", G_CALLBACK(onDestroy), b);
	b->onClicked = defaultOnClicked;
	c->data = b;

	return c;
}

char *uiButtonText(uiControl *c)
{
	return g_strdup(gtk_button_get_label(GTK_BUTTON(uiControlHandle(c))));
}

void uiButtonSetText(uiControl *c, const char *text)
{
	gtk_button_set_label(GTK_BUTTON(uiControlHandle(c)), text);
}

void uiButtonOnClicked(uiControl *c, void (*f)(uiControl *, void *), void *data)
{
	struct button *b = (struct button *) (c->data);

	b->onClicked = f;
	b->onClickedData = data;
}
