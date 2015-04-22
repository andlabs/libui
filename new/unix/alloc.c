// 7 april 2015
#include <stdio.h>
#include "uipriv_unix.h"

void *uiAlloc(size_t size, const char *type)
{
	void *out;

	out = g_malloc0(size);
	if (options.debugLogAllocations)
		fprintf(stderr, "%p alloc %s\n", out, type);
	return out;
}

void *uiRealloc(void *p, size_t size, const char *type)
{
	void *out;

	if (p == NULL)
		return uiAlloc(size, type);
	// TODO fill with 0s
	out = g_realloc(p, size);
	if (options.debugLogAllocations)
		fprintf(stderr, "%p realloc %p\n", p, out);
	return out;
}

void uiFree(void *p)
{
	g_free(p);
	if (options.debugLogAllocations)
		fprintf(stderr, "%p free\n", p);
}
