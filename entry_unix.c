// 8 april 2015
#include "uipriv_unix.h"

struct entry {
	uiControl *c;
};

#define E(x) ((struct entry *) (x))

static void onDestroy(GtkWidget *widget, gpointer data)
{
	struct entry *e = (struct entry *) data;

	uiFree(e);
}

uiControl *uiNewEntry(void)
{
	struct entry *e;
	GtkWidget *widget;

	e = uiNew(struct entry);

	e->c = uiUnixNewControl(GTK_TYPE_ENTRY,
		FALSE, FALSE, e,
		NULL);

	widget = GTK_WIDGET(uiControlHandle(e->c));
	g_signal_connect(widget, "destroy", G_CALLBACK(onDestroy), e);

	return e->c;
}
