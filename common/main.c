// 19 april 2019
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

bool uiprivInitReturnErrorf(uiInitError *err, const char *fmt, ...)
{
	int n;
	va_list ap;

	va_start(ap, fmt);
	n = uiprivVsnprintf(err->Message, 256, fmt, ap);
	va_end(ap);
	if (n < 0) {
		uiprivInternalError("encoding error returning initialization error; this means something is very very wrong with libui itself");
		abort();		// TODO handle this scenario more gracefully
	}
	if (n >= 256) {
		// the formatted message is too long; truncate it
		err->Message[252] = '.';
		err->Message[253] = '.';
		err->Message[254] = '.';
		err->Message[255] = '\0';
	}
	return false;
}

void uiQueueMain(void (*f)(void *data), void *data)
{
	if (!initialized) {
		uiprivProgrammerErrorNotInitialized(uiprivFunc);
		return;
	}
	uiprivSysQueueMain(f, data);
}

bool uiprivCheckInitializedAndThreadImpl(const char *func)
{
	// While it does seem risky to not lock this, if this changes during the execution of this function it means only that it was changed from a different thread, and since it can only change from false to true, an error will be reported anyway.
	if (!initialized) {
		uiprivProgrammerErrorNotInitialized(func);
		return false;
	}
	if (!uiprivSysCheckThread()) {
		uiprivProgrammerErrorWrongThread(func);
		return false;
	}
	return true;
}
