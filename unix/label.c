// 11 april 2015
#include "uipriv_unix.h"

struct label {
	uiLabel l;
};

static void onDestroy(GtkWidget *widget, gpointer data)
{
	struct label *l = (struct label *) data;

	uiFree(l);
}

#define LABEL(l) GTK_LABEL(uiControlHandle(uiControl(l)))

static char *getText(uiLabel *l)
{
	// TODO change g_strdup() to a wrapper function for export in ui_unix.h
	return g_strdup(gtk_label_get_text(LABEL(l)));
}

static void setText(uiLabel *l, const char *text)
{
	gtk_label_set_text(LABEL(l), text);
}

uiControl *uiNewLabel(const char *text)
{
	struct label *l;
	GtkWidget *widget;

	l = uiNew(struct label);

	uiUnixNewControl(uiControl(l), GTK_TYPE_LABEL,
		FALSE, FALSE,
		"label", text,
		"xalign", 0.0,		// note: must be a float constant, otherwise the ... will turn it into an int and we get segfaults on some platforms (thanks ebassi in irc.gimp.net/#gtk+)
		// TODO yalign 0?
		NULL);

	widget = GTK_WIDGET(LABEL(l));
	g_signal_connect(widget, "destroy", G_CALLBACK(onDestroy), l);

	uiLabel(l)->Text = getText;
	uiLabel(l)->SetText = setText;

	return uiLabel(l);
}
