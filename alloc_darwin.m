// 4 december 2014
#include "ui_darwin.h"

void *uiAlloc(size_t size)
{
	void *out;

	out = malloc(size);
	NSAssert(out != NULL, @"out of memory in uiAlloc()");
	memset(out, 0, size);
	return out;
}

void *uiRealloc(void *p, size_t size)
{
	void *out;

	if (p == NULL)
		return uiAlloc(size);
	out = realloc(p, size);
	NSAssert(out != NULL, @"out of memory in uiRealloc()");
	// TODO zero the extra memory
	return out;
}

void uiFree(void *p)
{
	if (p == NULL)
		return;
	free(p);
}
