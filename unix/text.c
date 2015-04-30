// 9 april 2015
#include "uipriv_unix.h"

// TODO rename to uiUnixStrdupText()
char *strdupText(const char *t)
{
	return g_strdup(t);
}

void uiFreeText(char *t)
{
	g_free(t);
}
