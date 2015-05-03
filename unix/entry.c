// 8 april 2015
#include "uipriv_unix.h"

struct entry {
	uiEntry e;
	GtkWidget *widget;
	GtkEntry *entry;
};

static void onDestroy(void *data)
{
	struct entry *e = (struct entry *) data;

	uiFree(e);
}

static char *entryText(uiEntry *ee)
{
	struct entry *e = (struct entry *) ee;

	return uiUnixStrdupText(gtk_entry_get_text(e->entry));
}

static void entrySetText(uiEntry *ee, const char *text)
{
	struct entry *e = (struct entry *) ee;

	gtk_entry_set_text(e->entry, text);
}

uiEntry *uiNewEntry(void)
{
	struct entry *e;

	e = uiNew(struct entry);

	uiUnixMakeControl(uiControl(e), GTK_TYPE_ENTRY,
		FALSE, FALSE, onDestroy, e,
		NULL);

	e->widget = GTK_WIDGET(uiControlHandle(uiControl(e)));
	e->entry = GTK_ENTRY(e->widget);

	uiEntry(e)->Text = entryText;
	uiEntry(e)->SetText = entrySetText;

	return uiEntry(e);
}
