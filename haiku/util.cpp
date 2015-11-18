// 7 april 2015
#include <cstdio>
#include <cstdlib>
#include "uipriv_haiku.hpp"
using namespace std;

void complain(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "[libui] ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	abort();
}
