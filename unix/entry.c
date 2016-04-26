// 11 june 2015
#include "uipriv_unix.h"

struct uiEntry {
	uiUnixControl c;
	GtkWidget *widget;
	GtkEntry *entry;
	GtkEditable *editable;
	void (*onChanged)(uiEntry *, void *);
	void *onChangedData;
	gulong onChangedSignal;
};

uiUnixControlAllDefaults(uiEntry)

static void onChanged(GtkEditable *editable, gpointer data)
{
	uiEntry *e = uiEntry(data);

	(*(e->onChanged))(e, e->onChangedData);
}

static void defaultOnChanged(uiEntry *e, void *data)
{
	// do nothing
}

char *uiEntryText(uiEntry *e)
{
	return uiUnixStrdupText(gtk_entry_get_text(e->entry));
}

void uiEntrySetText(uiEntry *e, const char *text)
{
	// we need to inhibit sending of ::changed because this WILL send a ::changed otherwise
	g_signal_handler_block(e->editable, e->onChangedSignal);
	gtk_entry_set_text(e->entry, text);
	g_signal_handler_unblock(e->editable, e->onChangedSignal);
	// don't queue the control for resize; entry sizes are independent of their contents
}

void uiEntryOnChanged(uiEntry *e, void (*f)(uiEntry *, void *), void *data)
{
	e->onChanged = f;
	e->onChangedData = data;
}

int uiEntryReadOnly(uiEntry *e)
{
	return gtk_editable_get_editable(e->editable) == FALSE;
}

void uiEntrySetReadOnly(uiEntry *e, int readonly)
{
	gboolean editable;

	editable = TRUE;
	if (readonly)
		editable = FALSE;
	gtk_editable_set_editable(e->editable, editable);
}

uiEntry *uiNewEntry(void)
{
	uiEntry *e;

	uiUnixNewControl(uiEntry, e);

	e->widget = gtk_entry_new();
	e->entry = GTK_ENTRY(e->widget);
	e->editable = GTK_EDITABLE(e->widget);

	e->onChangedSignal = g_signal_connect(e->widget, "changed", G_CALLBACK(onChanged), e);
	uiEntryOnChanged(e, defaultOnChanged, NULL);

	return e;
}
