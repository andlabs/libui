// 19 april 2019
#include <string.h>
#include "ui.h"
#include "uipriv.h"

static int initialized = 0;

#define errAlreadyInitialized "libui already initialized"
#define errOptionsMustBeNULL "options parameter to uiInit() must be NULL"

static const char *commonInitErrors[] = {
	errAlreadyInitialized,
	errOptionsMustBeNULL,
	NULL,
};

static int checkInitErrorLengths(uiInitError *err, const char *initErrors[])
{
	const char **p;

	if (initErrors == NULL)
		return 1;
	for (p = initErrors; *p != NULL; p++)
		if (strlen(*p) > 255) {
			strcpy(err->Message, "[INTERNAL] uiInit() error too long: ");
			strncat(err->Message, *p, 32);
			strcat(err->Message, "...");
			return 0;
		}
	return 1;
}

int uiprivInitCheckParams(void *options, uiInitError *err, const char *initErrors[])
{
	if (err == NULL)
		return 0;
	if (err->Size != sizeof (uiInitError))
		return 0;

	if (!checkInitErrorLengths(err, commonInitErrors))
		return 0;
	if (!checkInitErrorLengths(err, initErrors))
		return 0;

	if (initialized)
		return uiprivInitReturnError(err, errAlreadyInitialized);

	if (options != NULL)
		return uiprivInitReturnError(err, errOptionsMustBeNULL);

	return 1;
}

int uiprivInitReturnError(uiInitError *err, const char *msg)
{
	// checkInitErrorLengths() above ensures that err->Message[255] will always be '\0'
	strncpy(err->Message, msg, 256);
	return 0;
}

void uiprivMarkInitialized(void)
{
	initialized = 1;
}
