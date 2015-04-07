// 7 april 2015
#include "uipriv_unix.h"

struct button {
	uiControl *c;
	void (*onClicked)(uiControl *, void *);
	void *onClickedData;
};

#define B(x) ((struct button *) (x))

static void onClicked(GtkButton *b, gpointer data)
{
	(*(B(data)->onClicked))(B(data)->c, B(data)->onClickedData);
}

static void defaultOnClicked(uiControl *c, void *data)
{
	// do nothing
}

// TODO destruction
uiControl *uiNewButton(const char *text)
{
	struct button *b;
	GParameter props[1];
	GtkWidget *widget;

	b = uiNew(struct button);

	props[0].name = "label";
	g_value_init(&(props[0].value), G_TYPE_STRING);
	g_value_set_string(&(props[0].value), text);
	b->c = uiUnixNewControl(GTK_TYPE_BUTTON,
		1, props,
		FALSE, FALSE, FALSE, b);
	g_value_unset(&(props[0].value));		// thanks to gregier in irc.gimp.net/#gtk+

	widget = GTK_WIDGET(uiControlHandle(b->c));
	g_signal_connect(widget, "clicked", G_CALLBACK(onClicked), b);

	b->onClicked = defaultOnClicked;

	return b->c;
}

// TODO text

void uiButtonOnClicked(uiControl *c, void (*f)(uiControl *, void *), void *data)
{
	struct button *b;

	b = (struct button *) uiUnixControlData(c);
	b->onClicked = f;
	b->onClickedData = data;
}
