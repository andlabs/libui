// 19 april 2019
#include <string.h>
#include "ui.h"
#include "uipriv.h"

static int initialized = 0;

int uiprivInitCheckParams(void *options, uiInitError *err)
{
	if (err == NULL)
		return 0;
	if (err->Size != sizeof (uiInitError))
		return 0;

	if (initialized)
		return uiprivInitReturnError(err, "xxxxxxxxx");

	if (options != NULL)
		return uiprivInitReturnError(err, "xxxxxxxx");

	return 1;
}

int uiprivInitReturnError(uiInitError *err, const char *msg)
{
	strncpy(err->Message, msg, 256);
	if (err->Message[255] != '\0')
		// TODO there should be some check to make sure this doesn't happen
		err->Message[255] = '\0';
	return 0;
}

void uiprivMarkInitialized(void)
{
	initialized = 1;
}
