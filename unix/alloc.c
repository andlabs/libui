// 7 april 2015
#include <stdio.h>
#include "uipriv_unix.h"

void *uiAlloc(size_t size)
{
	void *out;

	out = g_malloc0(size);
	return out;
}

void *uiRealloc(void *p, size_t size)
{
	void *out;

	if (p == NULL)
		return uiAlloc(size);
	// TODO fill with 0s
	out = g_realloc(p, size);
	return out;
}

void uiFree(void *p)
{
	g_free(p);
}
