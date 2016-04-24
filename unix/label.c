// 11 june 2015
#include "uipriv_unix.h"

struct uiLabel {
	uiUnixControl c;
	GtkWidget *widget;
	GtkMisc *misc;
	GtkLabel *label;
};

uiUnixDefineControl(
	uiLabel								// type name
)

char *uiLabelText(uiLabel *l)
{
	return uiUnixStrdupText(gtk_label_get_text(l->label));
}

void uiLabelSetText(uiLabel *l, const char *text)
{
	gtk_label_set_text(l->label, text);
	// changing the text might necessitate a change in the label's size
	uiControlQueueResize(uiControl(l));
}

uiLabel *uiNewLabel(const char *text)
{
	uiLabel *l;

	l = (uiLabel *) uiNewControl(uiLabel);

	l->widget = gtk_label_new(text);
	l->misc = GTK_MISC(l->widget);
	l->label = GTK_LABEL(l->widget);

	gtk_misc_set_alignment(l->misc, 0, 0);

	uiUnixFinishNewControl(l, uiLabel);

	return l;
}
