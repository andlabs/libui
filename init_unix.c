// 6 april 2015
#include "ui_unix.h"

struct uiInitError {
	GError *err;
};

uiInitError *uiInit(uiInitOptions *o)
{
	uiInitError *err;

	err = g_new0(uiInitError, 1);
	if (gtk_init_with_args(NULL, NULL, NULL, NULL, NULL, &(err->err)) == FALSE)
		return err;
	g_free(err);
	return NULL;
}

const char *uiInitErrorMessage(uiInitError *err)
{
	return err->err->message;
}

void uiInitErrorFree(uiInitError *err)
{
	g_error_free(err->err);
	g_free(err);
}
