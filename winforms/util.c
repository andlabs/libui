// 6 april 2015
#include "unmanaged.h"
#include <stdio.h>
#include <stdlib.h>

void complain(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "[libui] ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	DebugBreak();
	abort();		// just in case
}
