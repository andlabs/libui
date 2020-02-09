// 12 may 2019
#include "uipriv.h"
#include "testhooks.h"

#define maxErrorBuf 256
#define conststrlen(s) ((sizeof (s) / sizeof (char)) - 1)
#define errorBufSize(prefix, suffix) (conststrlen(prefix) + maxErrorBuf + conststrlen(suffix) + 1)

#define internalErrorPrefix "libui internal error"
// TODO add debugging advice?
#define internalErrorSuffix "This likely means there is a bug in libui itself. Contact the libui authors."
#define internalErrorBufSize errorBufSize(internalErrorPrefix, internalErrorSuffix)

void uiprivInternalError(const char *fmt, ...)
{
	va_list ap;
	char buf[internalErrorBufSize];
	int n;

	va_start(ap, fmt);
	n = uiprivVsnprintf(buf, internalErrorBufSize, fmt, ap);
	va_end(ap);
	if (n < 0)
		uiprivReportError(internalErrorPrefix, "internal error string has encoding error", internalErrorSuffix, true);
	if (n >= internalErrorBufSize)
		uiprivReportError(internalErrorPrefix, "internal error string too long", internalErrorSuffix, true);
	uiprivReportError(internalErrorPrefix, buf, internalErrorSuffix, true);
}

#define programmerErrorPrefix "libui programmer error"
// TODO add debugging advice?
#define programmerErrorSuffix "This likely means you are using libui incorrectly. Check your source code and try again. If you have received this warning in error, contact the libui authors."
#define programmerErrorBufSize errorBufSize(programmerErrorPrefix, programmerErrorSuffix)

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
	char buf[programmerErrorBufSize];
	int n;

	va_start(ap, fmt);
	n = uiprivVsnprintf(buf, programmerErrorBufSize, fmt, ap);
	if (n < 0)
		uiprivInternalError("programmer error has encoding error");
	if (n >= programmerErrorBufSize)
		uiprivInternalError("programmer error string too long (%d)", n);
	va_end(ap);
	if (reportProgrammerErrorTestHook != NULL) {
		(*reportProgrammerErrorTestHook)(buf, reportProgrammerErrorTestHookData);
		return;
	}
	uiprivReportError(programmerErrorPrefix, buf, programmerErrorSuffix, false);
}
