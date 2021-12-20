// 9 april 2015
#include "uipriv_unix.h"

char *uiUnixStrdupText(const char *t)
{
	return g_strdup(t);
}

void uiFreeText(char *t)
{
	g_free(t);
}

int uiprivStricmp(const char *a, const char *b)
{
	return g_ascii_strcasecmp(a, b);
}
