// 11 april 2015
#include "uipriv_unix.h"

struct label {
	uiLabel l;
	GtkWidget *widget;
	GtkLabel *label;
};

static void onDestroy(GtkWidget *widget, gpointer data)
{
	struct label *l = (struct label *) data;

	uiFree(l);
}

static char *labelText(uiLabel *ll)
{
	struct label *l = (struct label *) ll;

	// TODO change g_strdup() to a wrapper function for export in ui_unix.h
	return g_strdup(gtk_label_get_text(l->label));
}

static void labelSetText(uiLabel *ll, const char *text)
{
	struct label *l = (struct label *) ll;

	gtk_label_set_text(l->label, text);
}

uiLabel *uiNewLabel(const char *text)
{
	struct label *l;

	l = uiNew(struct label);

	uiUnixNewControl(uiControl(l), GTK_TYPE_LABEL,
		FALSE, FALSE,
		"label", text,
		"xalign", 0.0,		// note: must be a float constant, otherwise the ... will turn it into an int and we get segfaults on some platforms (thanks ebassi in irc.gimp.net/#gtk+)
		// TODO yalign 0?
		NULL);

	l->widget = WIDGET(l);
	l->label = GTK_LABEL(l->widget);

	g_signal_connect(l->widget, "destroy", G_CALLBACK(onDestroy), l);

	uiLabel(l)->Text = labelText;
	uiLabel(l)->SetText = labelSetText;

	return uiLabel(l);
}
