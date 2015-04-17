// 7 april 2015
#include "uipriv_unix.h"

struct button {
	uiButton b;
	GtkWidget *widget;
	GtkButton *button;
	void (*onClicked)(uiButton *, void *);
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

static char *buttonText(uiButton *bb)
{
	struct button *b = (struct button *) bb;

	return g_strdup(gtk_button_get_label(b->button));
}

static void buttonSetText(uiButton *bb, const char *text)
{
	struct button *b = (struct button *) bb;

	gtk_button_set_label(b->button, text);
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

	b = uiNew(struct button);

	uiUnixNewControl(uiControl(b), GTK_TYPE_BUTTON,
		FALSE, FALSE,
		"label", text,
		NULL);

	b->widget = WIDGET(b);
	b->button = GTK_BUTTON(b->widget);

	g_signal_connect(b->widget, "clicked", G_CALLBACK(onClicked), b);
	g_signal_connect(b->widget, "destroy", G_CALLBACK(onDestroy), b);
	b->onClicked = defaultOnClicked;

	uiButton(b)->Text = buttonText;
	uiButton(b)->SetText = buttonSetText;
	uiButton(b)->OnClicked = buttonOnClicked;

	return uiButton(b);
}
