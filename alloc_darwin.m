// 4 december 2014
#import <stdio.h>
#import "uipriv_darwin.h"

void *uiAlloc(size_t size, const char *type)
{
	void *out;

	out = malloc(size);
	if (out == NULL) {
		fprintf(stderr, "memory exhausted in uiAlloc() allocating %s\n", type);
		abort();
	}
	memset(out, 0, size);
	if (options.debugLogAllocations)
		fprintf(stderr, "%p alloc %s\n", out, type);
	return out;
}

void *uiRealloc(void *p, size_t size, const char *type)
{
	void *out;

	if (p == NULL)
		return uiAlloc(size, type);
	out = realloc(p, size);
	if (out == NULL) {
		fprintf(stderr, "memory exhausted in uiRealloc() reallocating %s\n", type);
		abort();
	}
	// TODO zero the extra memory
	if (options.debugLogAllocations)
		fprintf(stderr, "%p realloc %p\n", p, out);
	return out;
}

void uiFree(void *p)
{
	if (p == NULL)
		return;
	free(p);
	if (options.debugLogAllocations)
		fprintf(stderr, "%p free\n", p);
}
