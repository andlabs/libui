// 8 april 2015
#include "uipriv_unix.h"

struct entry {
};

static void onDestroy(GtkWidget *widget, gpointer data)
{
	struct entry *e = (struct entry *) data;

	uiFree(e);
}

uiControl *uiNewEntry(void)
{
	uiControl *c;
	struct entry *e;
	GtkWidget *widget;

	c = uiUnixNewControl(GTK_TYPE_ENTRY,
		FALSE, FALSE,
		NULL);

	widget = GTK_WIDGET(uiControlHandle(c));

	e = uiNew(struct entry);
	g_signal_connect(widget, "destroy", G_CALLBACK(onDestroy), e);
	c->data = e;

	return c;
}

char *uiEntryText(uiControl *c)
{
	return g_strdup(gtk_entry_get_text(GTK_ENTRY(uiControlHandle(c))));
}

void uiEntrySetText(uiControl *c, const char *text)
{
	gtk_entry_set_text(GTK_ENTRY(uiControlHandle(c)), text);
}
