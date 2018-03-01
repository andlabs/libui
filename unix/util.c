// 18 april 2015
#include "uipriv_unix.h"

void setMargined(GtkContainer *c, int margined)
{
	if (margined)
		gtk_container_set_border_width(c, gtkXMargin);
	else
		gtk_container_set_border_width(c, 0);
}

int uiOpenURL(const char *url)
{
	return gtk_show_uri(NULL, url, GDK_CURRENT_TIME, NULL) == TRUE;
}
