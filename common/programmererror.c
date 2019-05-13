// 12 may 2019
#include <stdarg.h>
#include <stdio.h>
#include "ui.h"
#include "uipriv.h"

static const char *messages[] = {
	[uiprivProgrammerErrorWrongStructSize] = "wrong size %zu for %s",
	[uiprivProgrammerErrorIndexOutOfRange] = "index %d out of range in %s()",
};

static void prepareProgrammerError(char *buf, int size, unsigned int which, va_list ap)
{
	int n;

	if (which >= uiprivNumProgrammerErrors) {
		// TODO
	}
	n = vsnprintf(buf, size, messages[which], ap);
	if (n < 0) {
		// TODO
	}
	if (n >= size) {
		// TODO
		buf[size - 4] = '.';
		buf[size - 3] = '.';
		buf[size - 2] = '.';
		buf[size - 1] = '\0';
	}
}

void uiprivProgrammerError(unsigned int which, ...)
{
	va_list ap;
	char buf[256];

	va_start(ap, which);
	prepareProgrammerError(buf, 256, which, ap);
	va_end(ap);
	uiprivSysProgrammerError(buf);
}
