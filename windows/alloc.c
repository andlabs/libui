// 4 december 2014
#include "uipriv_windows.h"

// wrappers for allocator of choice
// panics on memory exhausted, undefined on heap corruption or other unreliably-detected malady (see http://stackoverflow.com/questions/28761680/is-there-a-windows-api-memory-allocator-deallocator-i-can-use-that-will-just-giv)
// new memory is set to zero
// passing NULL to tableRealloc() acts like tableAlloc()
// passing NULL to tableFree() is a no-op

void *uiAlloc(size_t size, const char *type)
{
	void *out;

	out = malloc(size);
	if (out == NULL) {
		fprintf(stderr, "memory exhausted in uiAlloc() allocating %s\n", type);
		abort();
	}
	ZeroMemory(out, size);
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
	return out;
}

void uiFree(void *p)
{
	if (p == NULL)
		return;
	free(p);
}
