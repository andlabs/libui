// 18 april 2015
#include "uipriv_unix.h"

void complain(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	// there's no g_errorv() in glib 2.32, so do it manually instead
	g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, fmt, ap);
	va_end(ap);
	abort();		// just in case
}
