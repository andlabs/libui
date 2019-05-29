// 19 april 2019
// TODO get rid of the need for this (it temporarily silences noise so I can find actual build issues)
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdarg.h>
#include <stdio.h>
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

static int checkInitErrorLengths(uiInitError *err, const char **initErrors)
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

int uiInit(void *options, uiInitError *err)
{
	if (err == NULL)
		return 0;
	if (err->Size != sizeof (uiInitError))
		return 0;

	if (!checkInitErrorLengths(err, commonInitErrors))
		return 0;
	if (!checkInitErrorLengths(err, uiprivSysInitErrors()))
		return 0;

	if (initialized)
		return uiprivInitReturnError(err, errAlreadyInitialized);

	if (options != NULL)
		return uiprivInitReturnError(err, errOptionsMustBeNULL);

	if (!uiprivSysInit(options, err))
		return 0;
	initialized = 1;
	return 1;
}

int uiprivInitReturnError(uiInitError *err, const char *msg)
{
	// checkInitErrorLengths() above ensures that err->Message[255] will always be '\0'
	strncpy(err->Message, msg, 256);
	return 0;
}

int uiprivInitReturnErrorf(uiInitError *err, const char *msg, ...)
{
	va_list ap;

	// checkInitErrorLengths() above ensures that err->Message[255] will always be '\0' assuming the formatted string in msg passed to checkInitErrorLengths() is valid
	va_start(ap, msg);
	vsnprintf(err->Message, 256, msg, ap);
	va_end(ap);
	return 0;
}

bool uiprivCheckInitializedAndThreadImpl(const char *func)
{
	// While it does seem risky to not lock this, if this changes during the execution of this function it means only that it was changed from a different thread, and since it can only change from false to true, an error will be reported anyway.
	if (!initialized) {
		uiprivProgrammerError(uiprivProgrammerErrorNotInitialized, func);
		return false;
	}
	if (!uiprivSysCheckThread()) {
		uiprivProgrammerError(uiprivProgrammerErrorWrongThread, func);
		return false;
	}
	return true;
}
