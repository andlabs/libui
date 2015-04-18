// 8 april 2015
#include "uipriv_unix.h"

struct entry {
	uiEntry e;
	GtkWidget *widget;
	GtkEntry *entry;
};

static void onDestroy(GtkWidget *widget, gpointer data)
{
	struct entry *e = (struct entry *) data;

	if (options.debugLogLifetimes)
		fprintf(stderr, "GtkWidget::destroy entry %p\n", e);
	uiFree(e);
}

static char *entryText(uiEntry *ee)
{
	struct entry *e = (struct entry *) ee;

	return g_strdup(gtk_entry_get_text(e->entry));
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
	if (options.debugLogLifetimes)
		fprintf(stderr, "uiNewEntry() %p\n", e);

	uiUnixNewControl(uiControl(e), GTK_TYPE_ENTRY,
		FALSE, FALSE,
		NULL);

	e->widget = WIDGET(e);
	e->entry = GTK_ENTRY(e->widget);

	g_signal_connect(e->widget, "destroy", G_CALLBACK(onDestroy), e);

	uiEntry(e)->Text = entryText;
	uiEntry(e)->SetText = entrySetText;

	return uiEntry(e);
}
