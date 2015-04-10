// 6 april 2015
#include "uipriv_unix.h"

struct uiInitError {
	GError *err;
};

uiInitOptions options;

uiInitError *uiInit(uiInitOptions *o)
{
	uiInitError *err;

	options = *o;
	err = uiNew(uiInitError);
	if (gtk_init_with_args(NULL, NULL, NULL, NULL, NULL, &(err->err)) == FALSE)
		return err;
	uiFree(err);
	return NULL;
}

const char *uiInitErrorMessage(uiInitError *err)
{
	return err->err->message;
}

void uiInitErrorFree(uiInitError *err)
{
	g_error_free(err->err);
	uiFree(err);
}
