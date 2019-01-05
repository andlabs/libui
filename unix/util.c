// 18 april 2015
#include "uipriv_unix.h"

void uiprivSetMargined(GtkContainer *c, int margined)
{
	if (margined)
		gtk_container_set_border_width(c, uiprivGTKXMargin);
	else
		gtk_container_set_border_width(c, 0);
}
