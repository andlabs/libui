// 11 june 2015
#include "uipriv_unix.h"

struct label {
	uiLabel l;
	GtkWidget *widget;
	GtkMisc *misc;
	GtkLabel *label;
};

uiDefineControlType(uiLabel, uiTypeLabel, struct label)

static uintptr_t labelHandle(uiControl *c)
{
	struct label *l = (struct label *) c;

	return (uintptr_t) (l->widget);
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
	// changing the text might necessitate a change in the label's size
	uiControlQueueResize(uiControl(l));
}

uiLabel *uiNewLabel(const char *text)
{
	struct label *l;

	l = (struct label *) uiNewControl(uiTypeLabel());

	l->widget = gtk_label_new(text);
	l->misc = GTK_MISC(l->widget);
	l->label = GTK_LABEL(l->widget);
	uiUnixMakeSingleWidgetControl(uiControl(l), l->widget);

	gtk_misc_set_alignment(l->misc, 0, 0);

	uiControl(l)->Handle = labelHandle;

	uiLabel(l)->Text = labelText;
	uiLabel(l)->SetText = labelSetText;

	return uiLabel(l);
}
