// 8 april 2015
#include "uipriv_unix.h"

struct entry {
	uiEntry e;
};

static void onDestroy(GtkWidget *widget, gpointer data)
{
	struct entry *e = (struct entry *) data;

	uiFree(e);
}

#define ENTRY(e) GTK_ENTRY(uiControlHandle(uiControl(e)))

static char *getText(uiEntry *e)
{
	return g_strdup(gtk_entry_get_text(ENTRY(e)));
}

static void uiEntrySetText(uiEntry *e, const char *text)
{
	gtk_entry_set_text(ENTRY(e), text);
}

uiControl *uiNewEntry(void)
{
	struct entry *e;
	GtkWidget *widget;

	e = uiNew(struct entry);

	uiUnixNewControl(uiControl(e), GTK_TYPE_ENTRY,
		FALSE, FALSE,
		NULL);

	widget = GTK_WIDGET(ENTRY(e));
	g_signal_connect(widget, "destroy", G_CALLBACK(onDestroy), e);

	uiEntry(e)->Text = getText;
	uiEntry(e)->SetText = setText;

	return uiEntry(e);
}
