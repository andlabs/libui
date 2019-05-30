// 19 april 2019
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "ui.h"
#include "uipriv.h"

static bool initialized = false;

bool uiInit(void *options, uiInitError *err)
{
	if (err == NULL)
		return false;
	if (err->Size != sizeof (uiInitError))
		return false;

	if (initialized)
		return uiprivInitReturnErrorf(err, "libui already initialized");

	if (options != NULL)
		return uiprivInitReturnErrorf(err, "options parameter to uiInit() must be NULL");

	if (!uiprivSysInit(options, err))
		return false;
	initialized = true;
	return true;
}

// TODO rename all msgs to fmt
bool uiprivInitReturnErrorf(uiInitError *err, const char *msg, ...)
{
	int n;
	va_list ap;

	va_start(ap, msg);
	n = vsnprintf(err->Message, 256, msg, ap);
	va_end(ap);
	if (n < 0)
		uiprivInternalError("encoding error returning initialization error; this means something is very very wrong with libui itself");
	if (n >= 256) {
		// the formatted message is too long; truncate it
		err->Message[252] = '.';
		err->Message[253] = '.';
		err->Message[254] = '.';
		err->Message[255] = '\0';
	}
	return false;
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
