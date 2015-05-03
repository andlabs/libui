// 4 december 2014
#import <stdio.h>
#import "uipriv_darwin.h"

void *uiAlloc(size_t size)
{
	void *out;

	out = malloc(size);
	if (out == NULL) {
		fprintf(stderr, "memory exhausted in uiAlloc()\n");
		abort();
	}
	memset(out, 0, size);
	return out;
}

void *uiRealloc(void *p, size_t size)
{
	void *out;

	if (p == NULL)
		return uiAlloc(size);
	out = realloc(p, size);
	if (out == NULL) {
		fprintf(stderr, "memory exhausted in uiRealloc()\n");
		abort();
	}
	// TODO zero the extra memory
	return out;
}

void uiFree(void *p)
{
	if (p == NULL)
		return;
	free(p);
}
