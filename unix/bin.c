// 28 april 2015
#include "uipriv_unix.h"

int binHasOSParent(uiBin *b)
{
	GtkWidget *binWidget;

	binWidget = GTK_WIDGET(uiControlHandle(uiControl(b)));
	return gtk_widget_get_parent(binWidget) != NULL;
}

void binSetOSParent(uiBin *b, uintptr_t osParent)
{
	GtkWidget *binWidget;

	binWidget = GTK_WIDGET(uiControlHandle(uiControl(b)));
	gtk_container_add(GTK_CONTAINER(osParent), binWidget);
}

void binRemoveOSParent(uiBin *b)
{
	GtkWidget *binWidget;
	GtkWidget *oldparent;

	binWidget = GTK_WIDGET(uiControlHandle(uiControl(b)));
	oldparent = gtk_widget_get_parent(binWidget);
	gtk_container_remove(GTK_CONTAINER(oldparent), binWidget);
}

void binResizeRootAndUpdate(uiBin *b, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
	// not used on GTK+
}

void binTranslateMargins(uiBin *b, intmax_t *left, intmax_t *top, intmax_t *right, intmax_t *bottom, uiSizing *d)
{
	// not used on GTK+
}
