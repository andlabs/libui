// 19 april 2019
#include "uipriv.h"

enum {
	stateUninitialized,
	stateBeforeMain,
	stateInMain,
	stateQuitting,
	stateAfterMain,
	stateError,
};

static int state = stateUninitialized;

#define initialized() (state != stateUninitialized && state != stateError)

bool testHookInitShouldFail = false;

void uiprivTestHookSetInitShouldFailArtificially(bool shouldFail)
{
	testHookInitShouldFail = shouldFail;
}

bool uiInit(void *options, uiInitError *err)
{
	if (state != stateUninitialized) {
		uiprivProgrammerErrorMultipleCalls(uiprivFunc);
		state = stateError;
		return false;
	}
	if (options != NULL) {
		uiprivProgrammerErrorBadInitOptions(uiprivFunc);
		state = stateError;
		return false;
	}
	if (err == NULL) {
		uiprivProgrammerErrorNullPointer("uiInitError", uiprivFunc);
		state = stateError;
		return false;
	}
	if (err->Size != sizeof (uiInitError)) {
		uiprivProgrammerErrorWrongStructSize(err->Size, "uiInitError", uiprivFunc);
		state = stateError;
		return false;
	}

	if (testHookInitShouldFail) {
		state = stateError;
		return uiprivInitReturnErrorf(err, "general failure");
	}
	if (!uiprivSysInit(options, err)) {
		state = stateError;
		return false;
	}
	state = stateBeforeMain;
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

void uiMain(void)
{
	if (!uiprivCheckInitializedAndThread())
		return;
	if (state != stateBeforeMain) {
		uiprivProgrammerErrorMultipleCalls(uiprivFunc);
		return;
	}
	state = stateInMain;
	uiprivSysMain();
	state = stateAfterMain;
}

void uiQuit(void)
{
	if (!uiprivCheckInitializedAndThread())
		return;
	if (state == stateQuitting || state == stateAfterMain) {
		uiprivProgrammerErrorMultipleCalls(uiprivFunc);
		return;
	}
	if (state != stateInMain) {
		// the above handle the other states, so stateBeforeMain is what's left
		uiprivProgrammerErrorQuitBeforeMain(uiprivFunc);
		return;
	}
	state = stateQuitting;
	uiprivSysQuit();
}

void uiQueueMain(void (*f)(void *data), void *data)
{
	if (!initialized()) {
		uiprivProgrammerErrorNotInitialized(uiprivFunc);
		return;
	}
	uiprivSysQueueMain(f, data);
}

bool uiprivCheckInitializedAndThreadImpl(const char *func)
{
	// While it does seem risky to not lock this, if this changes during the execution of this function it means only that it was changed from a different thread, and since it can only change from false to true, an error will be reported anyway.
	if (!initialized()) {
		uiprivProgrammerErrorNotInitialized(func);
		return false;
	}
	if (!uiprivSysCheckThread()) {
		uiprivProgrammerErrorWrongThread(func);
		return false;
	}
	return true;
}
