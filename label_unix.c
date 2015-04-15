// 11 april 2015
#include "uipriv_unix.h"

struct label {
};

static void onDestroy(GtkWidget *widget, gpointer data)
{
	struct label *l = (struct label *) data;

	uiFree(l);
}

uiControl *uiNewLabel(const char *text)
{
	uiControl *c;
	struct label *l;
	GtkWidget *widget;

	c = uiUnixNewControl(GTK_TYPE_LABEL,
		FALSE, FALSE,
		"label", text,
		// TODO TODO TODO TODO TODO
		// the presence of this property leads to a segfault on 32-bit linux builds AND 64-bit openbsd builds
		// is it just this property? or is it more than one property at all? and why?
		"xalign", 0,
		// TODO yalign 0?
		NULL);

	widget = GTK_WIDGET(uiControlHandle(c));

	l = uiNew(struct label);
	g_signal_connect(widget, "destroy", G_CALLBACK(onDestroy), l);
	c->data = l;

	return c;
}

char *uiLabelText(uiControl *c)
{
	// TODO change g_strdup() to a wrapper function for export in ui_unix.h
	return g_strdup(gtk_label_get_text(GTK_LABEL(uiControlHandle(c))));
}

void uiLabelSetText(uiControl *c, const char *text)
{
	gtk_label_set_text(GTK_LABEL(uiControlHandle(c)), text);
}
