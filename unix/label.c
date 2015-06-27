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

	return uiUnixStrdupText(gtk_label_get_text(l->label));
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

	l->widget = GTK_WIDGET(uiControlHandle(uiControl(l)));
	l->label = GTK_LABEL(l->widget);

	uiLabel(l)->Text = labelText;
	uiLabel(l)->SetText = labelSetText;

	return uiLabel(l);
}
