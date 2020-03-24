// 11 june 2015
#include "uipriv_unix.h"
#include "keyboard.h"

struct uiEntry {
	uiUnixControl c;
	GtkWidget *widget;
	GtkEntry *entry;
	GtkEditable *editable;
	void (*onChanged)(uiEntry *, void *);
	void *onChangedData;
	gulong onChangedSignal;
	int (*onKeyEvent)(uiEntry *, uiAreaKeyEvent *);
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

static gboolean onKeyEvent(GtkEditable *editable, GdkEventKey *event, gpointer data)
{
	uiEntry *e = (uiEntry*)(data);
	uiAreaKeyEvent ke;
	if (fillUiKeyEvent(&ke, event))
		return (*(e->onKeyEvent))(e, &ke);
	return GDK_EVENT_PROPAGATE;
}

static int defaultOnKeyEvent(uiEntry *e, uiAreaKeyEvent *uke)
{
	// do nothing
	// return GDK_EVENT_STOP; // to stop further key handling
	return GDK_EVENT_PROPAGATE;
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

void uiEntryOnKeyEvent(uiEntry *e, int (*f)(uiEntry *, uiAreaKeyEvent *))
{
	e->onKeyEvent = f;
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

static uiEntry *finishNewEntry(GtkWidget *w, const gchar *signal)
{
	uiEntry *e;

	uiUnixNewControl(uiEntry, e);

	e->widget = w;
	e->entry = GTK_ENTRY(e->widget);
	e->editable = GTK_EDITABLE(e->widget);

	e->onChangedSignal = g_signal_connect(e->widget, signal, G_CALLBACK(onChanged), e);
	uiEntryOnChanged(e, defaultOnChanged, NULL);

	gtk_widget_add_events(e->widget, GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
	g_signal_connect(e->widget, "key_press_event", G_CALLBACK(onKeyEvent), e);
	g_signal_connect(e->widget, "key_release_event", G_CALLBACK(onKeyEvent), e);
	uiEntryOnKeyEvent(e, defaultOnKeyEvent);

	return e;
}

uiEntry *uiNewEntry(void)
{
	return finishNewEntry(gtk_entry_new(), "changed");
}

uiEntry *uiNewPasswordEntry(void)
{
	GtkWidget *e;

	e = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(e), FALSE);
	return finishNewEntry(e, "changed");
}

// TODO make it use a separate function to be type-safe
uiEntry *uiNewSearchEntry(void)
{
	return finishNewEntry(gtk_search_entry_new(), "search-changed");
}
