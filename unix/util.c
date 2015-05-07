// 18 april 2015
#include "uipriv_unix.h"

void complain(const char *fmt, ...)
{
	va_list ap;
	char *msg;

	va_start(ap, fmt);
	msg = g_strdup_vprintf(fmt, ap);
	va_end(ap);
	g_error("[libui] %s\n", msg);
}
