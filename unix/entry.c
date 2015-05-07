// 8 april 2015
#include "uipriv_unix.h"

struct entry {
	uiEntry e;
	GtkWidget *widget;
	GtkEntry *entry;
	void (*onChanged)(uiEntry *, void *);
	void *onChangedData;
};

static void onChanged(GtkEditable *editable, gpointer data)
{
	struct entry *e = (struct entry *) data;

	(*(e->onChanged))(uiEntry(e), e->onChangedData);
}

static void defaultOnChanged(uiEntry *e, void *data)
{
	// do nothing
}

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

static void entryOnChanged(uiEntry *ee, void (*f)(uiEntry *, void *), void *data)
{
	struct entry *e = (struct entry *) ee;

	e->onChanged = f;
	e->onChangedData = data;
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

	g_signal_connect(e->widget, "changed", G_CALLBACK(onChanged), e);
	e->onChanged = defaultOnChanged;

	uiEntry(e)->Text = entryText;
	uiEntry(e)->SetText = entrySetText;
	uiEntry(e)->OnChanged = entryOnChanged;

	return uiEntry(e);
}
