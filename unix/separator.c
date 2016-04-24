// 11 june 2015
#include "uipriv_unix.h"

struct uiSeparator {
	uiUnixControl c;
	GtkWidget *widget;
	GtkSeparator *separator;
};

uiUnixDefineControl(
	uiSeparator							// type name
)

uiSeparator *uiNewHorizontalSeparator(void)
{
	uiSeparator *s;

	s = (uiSeparator *) uiNewControl(uiSeparatorType());

	s->widget = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	s->separator = GTK_SEPARATOR(s->widget);

	uiUnixFinishNewControl(s, uiSeparator);

	return s;
}
