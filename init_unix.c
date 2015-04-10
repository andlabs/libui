// 6 april 2015
#include "uipriv_unix.h"

uiInitOptions options;

const char *uiInit(uiInitOptions *o)
{
	GError *err = NULL;
	const char *msg;

	options = *o;
	if (gtk_init_with_args(NULL, NULL, NULL, NULL, NULL, &err) == FALSE) {
		msg = g_strdup(err->message);
		g_error_free(err);
		return msg;
	}
	return NULL;
}

void uiFreeInitError(const char *err)
{
	g_free((gpointer) err);
}
