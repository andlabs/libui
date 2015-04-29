// 9 april 2015
#include "uipriv_unix.h"

char *strdupText(const char *t)
{
	return g_strdup(t);
}

void uiFreeText(char *t)
{
	g_free(t);
}
