// 12 may 2019
// TODO get rid of the need for this (it temporarily silences noise so I can find actual build issues)
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdarg.h>
#include <stdio.h>
#include "ui.h"
#include "uipriv.h"
#include "testhooks.h"

#define internalErrorPrefix "libui internal error"
// TODO add debugging advice?
#define internalErrorSuffix "This likely means there is a bug in libui itself. Contact the libui authors."

void uiprivInternalError(const char *fmt, ...)
{
	va_list ap;
	char buf[256];
	int n;

	va_start(ap, fmt);
	n = vsnprintf(buf, 256, fmt, ap);
	va_end(ap);
	if (n < 0)
		uiprivReportError(internalErrorPrefix, "internal error string has encoding error", internalErrorSuffix, true);
	if (n >= 256)
		uiprivReportError(internalErrorPrefix, "internal error string too long", internalErrorSuffix, true);
	uiprivReportError(internalErrorPrefix, buf, internalErrorSuffix, true);
}

static const char *messages[uiprivNumProgrammerErrors] = {
	[uiprivProgrammerErrorNotInitialized] = "attempt to call %s() before uiInit()",
	[uiprivProgrammerErrorWrongThread] = "attempt to call %s() on a thread other than the GUI thread",
	[uiprivProgrammerErrorWrongStructSize] = "wrong size %" uiprivSizetPrintf " for %s",
	[uiprivProgrammerErrorIndexOutOfRange] = "index %d out of range in %s()",
	[uiprivProgrammerErrorNullPointer] = "invalid null pointer for %s passed into %s()",
	[uiprivProgrammerErrorIntIDNotFound] = "%s identifier %d not found in %s()",
	[uiprivProgrammerErrorBadSenderForEvent] = "attempt to use a %s sender with a %s event in %s()",
	[uiprivProgrammerErrorChangingEventDuringFire] = "attempt to change a uiEvent with %s() while it is firing",
	[uiprivProgrammerErrorRecursiveEventFire] = "attempt to fire a uiEvent while it is already being fired",
};

static void prepareProgrammerError(char *buf, int size, unsigned int which, va_list ap)
{
	int n;

	if (which >= uiprivNumProgrammerErrors)
		uiprivInternalError("bad programmer error value %u", which);
	if (messages[which] == NULL)
		uiprivInternalError("programmer error %u has no message", which);
	n = vsnprintf(buf, size, messages[which], ap);
	if (n < 0)
		uiprivInternalError("programmer error string for %u has encoding error", which);
	if (n >= size)
		uiprivInternalError("programmer error string for %u too long (%d)", which, n);
}

#define programmerErrorPrefix "libui programmer error"
// TODO add debugging advice?
#define programmerErrorSuffix "This likely means you are using libui incorrectly. Check your source code and try again. If you have received this warning in error, contact the libui authors."

static uiprivTestHookReportProgrammerErrorFunc reportProgrammerErrorTestHook = uiprivReportError;

void uiprivTestHookReportProgrammerError(uiprivTestHookReportProgrammerErrorFunc f)
{
	if (f == NULL)
		f = uiprivReportError;
	reportProgrammerErrorTestHook = f;
}

void uiprivProgrammerError(unsigned int which, ...)
{
	va_list ap;
	char buf[256];

	va_start(ap, which);
	prepareProgrammerError(buf, 256, which, ap);
	va_end(ap);
	(*reportProgrammerErrorTestHook)(programmerErrorPrefix, buf, programmerErrorSuffix, false);
}
