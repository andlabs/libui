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

void *uiprivArrayAppend(uiprivArray *arr, size_t n)
{
	return uiprivArrayInsertAt(arr, arr->len, n);
}

void *uiprivArrayInsertAt(uiprivArray *arr, size_t pos, size_t n)
{
	uint8_t *old, *new;
	size_t nBytes;

	if ((arr->len + n) >= arr->cap) {
		size_t nGrow;

		// always grow by a perfect multiple of arr->nGrow
		nGrow = n + (arr->nGrow - (n % arr->nGrow));
		arr->buf = uiprivRealloc(arr->buf,
			arr->cap * arr->elemsize,
			(arr->cap + nGrow) * arr->elemsize,
			arr->what);
		arr->cap += nGrow;
	}
	arr->len += n;

	nBytes = n * arr->elemsize;
	new = ((uint8_t *) (arr->buf)) + (pos * arr->elemsize);
	old = new + nBytes;
	memmove(old, new, nBytes);
	memset(new, 0, nBytes);
	return new;
}

void uiprivArrayDelete(uiprivArray *arr, size_t pos, size_t n)
{
	uint8_t *src, *dest;
	size_t nBytesDeleted, nBytesRemaining;

	nBytesDeleted = n * arr->elemsize;
	nBytesRemaining = (arr->len - pos - n) * arr->elemsize;
	dest = ((uint8_t *) (arr->buf)) + (pos * arr->elemsize);
	src = dest + nBytesDeleted;
	memmove(dest, src, nBytesDeleted);
	memset(src, 0, nBytesRemaining);
	arr->len -= n;
}

void uiprivArrayDeleteItem(uiprivArray *arr, void *p, size_t n)
{
	uint8_t *p8, *buf8;

	p8 = (uint8_t *) p;
	buf8 = (uint8_t *) (arr->buf);
	// TODO write this in a way that doesn't mix ptrdiff_t and size_t
	uiprivArrayDelete(arr, (p8 - buf8) / arr->elemsize, n);
}

void *uiprivArrayBsearch(uiprivArray *arr, const void *key, int (*compare)(const void *, const void *))
{
	return bsearch(key, arr->buf, arr->len, arr->elemsize, compare);
}

void uiprivArrayQsort(uiprivArray *arr, int (*compare)(const void *, const void *))
{
	qsort(arr->buf, arr->len, arr->elemsize, compare);
}
