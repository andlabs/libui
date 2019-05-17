// 16 may 2019
#include <stdlib.h>
#include <string.h>
#include "ui.h"
#include "uipriv.h"

void *uiprivAlloc(size_t n, const char *what)
{
	void *p;

	p = malloc(n);
	if (p == NULL)
		uiprivInternalError("memory exhausted allocating %s", what);
	memset(p, 0, n);
	return p;
}

void *uiprivRealloc(void *p, size_t old, size_t new, const char *what)
{
	p = realloc(p, new);
	if (p == NULL)
		uiprivInternalError("memory exhausted reallocating %s", what);
	if (new > old)
		memset(((uint8_t *) p) + old, 0, new - old);
	return p;
}

void uiprivFree(void *p)
{
	free(p);
}
