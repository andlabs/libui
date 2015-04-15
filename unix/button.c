// 7 april 2015
#include "uipriv_unix.h"

struct button {
	void (*onClicked)(uiControl *, void *);
	void *onClickedData;
};

static void onClicked(GtkButton *button, gpointer data)
{
	uiControl *c = uiControl(data);
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

static char *getText(uiButton *b)
{
	return g_strdup(gtk_button_get_label(GTK_BUTTON(uiControlHandle(b.base))));
}

static void setText(uiButton *b, const char *text)
{
	gtk_button_set_label(GTK_BUTTON(uiControlHandle(b.base)), text);
}

static void setOnClicked(uiButton *b, void (*f)(uiControl *, void *), void *data)
{
	struct button *b = (struct button *) (b->base.data);

	b->onClicked = f;
	b->onClickedData = data;
}

uiControl *uiNewButton(const char *text)
{
	uiButton *b;
	struct button *bb;
	GtkWidget *widget;

	b = uiNew(uiButton);

	uiUnixNewControl(&(b.base), GTK_TYPE_BUTTON,
		FALSE, FALSE,
		"label", text,
		NULL);

	widget = GTK_WIDGET(uiControlHandle(&(b.base)));
	g_signal_connect(widget, "clicked", G_CALLBACK(onClicked), b);

	bb = uiNew(struct button);
	g_signal_connect(widget, "destroy", G_CALLBACK(onDestroy), bb);
	bb->onClicked = defaultOnClicked;
	b->priv.data = bb;

	b->Text = getText;
	b->SetText = setText;
	b->OnClicked = setOnClicked;

	return b;
}
