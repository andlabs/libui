// 30 may 2019
// requires: alloc_header.h

#include "start.h"

#include "printfwarn_header.h"
sharedbitsPrintfFunc(
	extern void sharedbitsPrefixName(InternalError)(const char *fmt, ...),
	1, 2);

void *sharedbitsPrefixName(Alloc)(size_t n, const char *what)
{
	void *p;

	p = malloc(n);
	if (p == NULL) {
		sharedbitsPrefixName(InternalError)("memory exhausted allocating %s", what);
		abort();		// TODO handle more gracefully somehow
	}
	memset(p, 0, n);
	return p;
}

void *sharedbitsPrefixName(Realloc)(void *p, size_t nOld, size_t nNew, const char *what)
{
	p = realloc(p, nNew);
	if (p == NULL) {
		sharedbitsPrefixName(InternalError)("memory exhausted reallocating %s", what);
		abort();		// TODO handle more gracefully somehow
	}
	if (nNew > nOld)
		memset(((uint8_t *) p) + nOld, 0, nNew - nOld);
	return p;
}

void sharedbitsPrefixName(Free)(void *p)
{
	free(p);
}

#include "end.h"
