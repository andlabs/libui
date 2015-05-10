// 8 april 2015
#include "uipriv_unix.h"

struct entry {
	uiEntry e;
	GtkWidget *widget;
	GtkEntry *entry;
	GtkEditable *editable;
	void (*onChanged)(uiEntry *, void *);
	void *onChangedData;
	gulong onChangedSignal;
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

	// we need to inhibit sending of ::changed because this WILL send a ::changed otherwise
	g_signal_handler_block(e->editable, e->onChangedSignal);
	gtk_entry_set_text(e->entry, text);
	g_signal_handler_unblock(e->editable, e->onChangedSignal);
}

static void entryOnChanged(uiEntry *ee, void (*f)(uiEntry *, void *), void *data)
{
	struct entry *e = (struct entry *) ee;

	e->onChanged = f;
	e->onChangedData = data;
}

static int entryReadOnly(uiEntry *ee)
{
	struct entry *e = (struct entry *) ee;

	return gtk_editable_get_editable(e->editable) == FALSE;
}

static void entrySetReadOnly(uiEntry *ee, int readonly)
{
	struct entry *e = (struct entry *) ee;
	gboolean editable;

	editable = TRUE;
	if (readonly)
		editable = FALSE;
	gtk_editable_set_editable(e->editable, editable);
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
	e->editable = GTK_EDITABLE(e->widget);

	e->onChangedSignal = g_signal_connect(e->widget, "changed", G_CALLBACK(onChanged), e);
	e->onChanged = defaultOnChanged;

	uiEntry(e)->Text = entryText;
	uiEntry(e)->SetText = entrySetText;
	uiEntry(e)->OnChanged = entryOnChanged;
	uiEntry(e)->ReadOnly = entryReadOnly;
	uiEntry(e)->SetReadOnly = entrySetReadOnly;

	return uiEntry(e);
}
