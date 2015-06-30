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

uiSizing *uiUnixNewSizing(void)
{
	uiSizing *d;

	d = uiNew(uiSizing);
	d->XPadding = gtkXPadding;
	d->YPadding = gtkYPadding;
	d->Sys = uiNew(uiSizingSys);
	return d;
}

void uiFreeSizing(uiSizing *d)
{
	uiFree(d->Sys);
	uiFree(d);
}
