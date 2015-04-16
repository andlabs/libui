// 7 april 2015
#include "uipriv_unix.h"

struct button {
	uiButton b;
	void (*onClicked)(uiButton *, void *);
	void *onClickedData;
};

static void clicked(GtkButton *button, gpointer data)
{
	struct button *b = (struct button *) data;

	(*(b->onClicked))(uiButton(b), b->onClickedData);
}

static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
}

static void destroy(GtkWidget *widget, gpointer data)
{
	struct button *b = (struct button *) data;

	uiFree(b);
}

#define BUTTON(b) GTK_BUTTON(widget(b))

static char *buttonText(uiButton *bb)
{
	return g_strdup(gtk_button_get_label(BUTTON(bb)));
}

static void buttonSetText(uiButton *bb, const char *text)
{
	gtk_button_set_label(BUTTON(bb), text);
}

static void buttonOnClicked(uiButton *bb, void (*f)(uiButton *, void *), void *data)
{
	struct button *b = (struct button *) bb;

	b->onClicked = f;
	b->onClickedData = data;
}

uiButton *uiNewButton(const char *text)
{
	struct button *b;
	GtkWidget *widget;

	b = uiNew(struct button);

	uiUnixNewControl(uiControl(b), GTK_TYPE_BUTTON,
		FALSE, FALSE,
		"label", text,
		NULL);

	widget = WIDGET(b);
	g_signal_connect(widget, "clicked", G_CALLBACK(clicked), b);
	g_signal_connect(widget, "destroy", G_CALLBACK(destroy), b);
	b->onClicked = defaultOnClicked;

	uiButton(b)->Text = buttonText;
	uiButton(b)->SetText = buttonSetText;
	uiButton(b)->OnClicked = buttonOnClicked;

	return uiButton(b);
}
