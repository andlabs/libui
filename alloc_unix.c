// 7 april 2015
#include <stdio.h>
#include "uipriv_unix.h"

void *uiAlloc(size_t size, const char *type)
{
	void *out;

	out = g_malloc0(size);
#ifdef uiLogAllocations
	fprintf(stderr, "%p alloc %s\n", out, type);
#endif
	return out;
}

void *uiRealloc(void *p, size_t size, const char *type)
{
	void *out;

	if (p == NULL)
		return uiAlloc(size, type);
	// TODO fill with 0s
	out = g_realloc(p, size);
#ifdef uiLogAllocations
	fprintf(stderr, "%p realloc %p\n", p, out);
#endif
	return out;
}

void uiFree(void *p)
{
	g_free(p);
#ifdef uiLogAllocations
	fprintf(stderr, "%p free\n", p);
#endif
}
