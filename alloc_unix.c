// 7 april 2015
#include "uipriv_unix.h"

void *uiAlloc(size_t size)
{
	return g_malloc0(size);
}

void *uiRealloc(void *p, size_t size)
{
	// TODO fill with 0s
	return g_realloc(p, size);
}

void uiFree(void *p)
{
	g_free(p);
}
