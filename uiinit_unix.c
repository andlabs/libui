// 6 april 2015
#include "ui_unix.h"

struct uiInitError {
	GError *err;
};

uiInitError *uiInit(uiInitOptions *o)
{
	uiInitError *e;

	e = g_new0(uiInitError, 1);
	if (gtk_init_with_args(NULL, NULL, NULL, NULL, NULL, &(e->err)) == FALSE)
		return e;
	uiInitErrorFree(e);
	return NULL;
}

const char *uiInitErrorMessage(uiInitError *e)
{
	return e->err->message;
}

void uiInitErrorFree(uiInitError *e)
{
	g_free(e);
}
