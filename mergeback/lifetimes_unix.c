// 19 april 2015
#include "uipriv_unix.h"

static void destroyBlocker(GtkWidget *widget, gpointer data)
{
	complain("trying to destroy object at %p before destroy function called", data);
}

gulong blockDestruction(GtkWidget *widget, void *object)
{
	return g_signal_connect(widget, "destroy", G_CALLBACK(destroyBlocker), object);
}

void readyToDestroy(GtkWidget *widget, gulong block)
{
	g_signal_handler_disconnect(widget, block);
}
