// 12 may 2019
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
	n = uiprivVsnprintf(buf, 256, fmt, ap);
	va_end(ap);
	if (n < 0)
		uiprivReportError(internalErrorPrefix, "internal error string has encoding error", internalErrorSuffix, true);
	if (n >= 256)
		uiprivReportError(internalErrorPrefix, "internal error string too long", internalErrorSuffix, true);
	uiprivReportError(internalErrorPrefix, buf, internalErrorSuffix, true);
}

#define programmerErrorPrefix "libui programmer error"
// TODO add debugging advice?
#define programmerErrorSuffix "This likely means you are using libui incorrectly. Check your source code and try again. If you have received this warning in error, contact the libui authors."

static uiprivTestHookReportProgrammerErrorFunc reportProgrammerErrorTestHook = NULL;
static void *reportProgrammerErrorTestHookData = NULL;

void uiprivTestHookReportProgrammerError(uiprivTestHookReportProgrammerErrorFunc f, void *data)
{
	reportProgrammerErrorTestHook = f;
	reportProgrammerErrorTestHookData = data;
}

void uiprivProgrammerError(const char *fmt, ...)
{
	va_list ap;
	int n;
	char buf[256];

	va_start(ap, fmt);
	n = uiprivVsnprintf(buf, 256, fmt, ap);
	if (n < 0)
		uiprivInternalError("programmer error has encoding error");
	if (n >= 256)
		uiprivInternalError("programmer error string too long (%d)", n);
	va_end(ap);
	if (reportProgrammerErrorTestHook != NULL) {
		(*reportProgrammerErrorTestHook)(buf, reportProgrammerErrorTestHookData);
		return;
	}
	uiprivReportError(programmerErrorPrefix, buf, programmerErrorSuffix, false);
}
