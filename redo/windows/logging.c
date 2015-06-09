// 9 june 2015
#include "uipriv_windows.h"

// We can't use the private heap for these functions, since they are allowed to be run before uiInit() or after uiUninit().

static void nomemlog(void)
{
	OutputDebugStringW(L"[libui] memory exhausted logging message");
	DebugBreak();
	abort();			// just in case
}

void uiLog(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	uiLogv(format, ap);
	va_end(ap);
}

void uiLogv(const char *format, va_list ap)
{
	va_list ap2;
	int n;
	char *buf;

	va_copy(ap2, ap);
	n = _vscprintf(format, ap2);
	va_end(ap2);
	// "[libui] message\0" == (8 + n + 1)
	buf = (char *) malloc((8 + n + 1)
	if (buf == NULL)
		nomemlog();
	buf[0] = '[';
	buf[1] = 'l';
	buf[2] = 'i';
	buf[3] = 'b';
	buf[4] = 'u';
	buf[5] = 'i';
	buf[6] = ']';
	buf[7] = ' ';
	vsnprintf_s(buf + 8, n + 1, n, format, ap);
	// breaking the rules: we can't reliably tell that the encoding of debugging messages is UTF-8 like we could with everything else :/
	OutputDebugStringA(buf);
	free(buf);
}

void complain(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	uiLogv(format, av);
	va_end(ap);
	DebugBreak();
	abort();		// just in case
}
