// 6 april 2015
#include "uipriv_unix.h"

int uiInit(void *options, uiInitError *err)
{
	GError *gerr = NULL;

	if (!uiprivInitCheckParams(options, err, NULL))
		return 0;

	if (gtk_init_with_args(NULL, NULL, NULL, NULL, NULL, &gerr) == FALSE) {
		strncpy(err->Message, gerr->message, 255);
		g_error_free(gerr);
		return 0;
	}

	uiprivMarkInitialized();
	return 1;
}

void uiUninit(void)
{
}
