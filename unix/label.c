// 11 april 2015
#include "uipriv_unix.h"

struct label {
	uiLabel l;
	GtkWidget *widget;
	GtkLabel *label;
};

static void onDestroy(void *data)
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
		FALSE, FALSE, onDestroy, l,
		"label", text,
		"xalign", 0.0,		// note: must be a float constant, otherwise the ... will turn it into an int and we get segfaults on some platforms (thanks ebassi in irc.gimp.net/#gtk+)
		// TODO yalign 0?
		NULL);

	l->widget = GTK_WIDGET(uiControlHandle(uiControl(l)));
	l->label = GTK_LABEL(l->widget);

	uiLabel(l)->Text = labelText;
	uiLabel(l)->SetText = labelSetText;

	return uiLabel(l);
}
