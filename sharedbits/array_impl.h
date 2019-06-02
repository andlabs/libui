// 30 may 2019
// requires: array_header.h

#include "start.h"

void sharedbitsPrefixName(ArrayInitFull)(sharedbitsPrefixName(Array) *arr, size_t elemsize, size_t nGrow, const char *what)
{
	memset(arr, 0, sizeof (sharedbitsPrefixName(Array)));
	arr->elemsize = elemsize;
	arr->nGrow = nGrow;
	arr->what = what;
}

void sharedbitsPrefixName(ArrayFreeFull)(sharedbitsPrefixName(Array) *arr)
{
	sharedbitsPrefixName(Free)(arr->buf);
	memset(arr, 0, sizeof (sharedbitsPrefixName(Array)));
}

void *sharedbitsPrefixName(ArrayAppend)(sharedbitsPrefixName(Array) *arr, size_t n)
{
	return sharedbitsPrefixName(ArrayInsertAt)(arr, arr->len, n);
}

void *sharedbitsPrefixName(ArrayInsertAt)(sharedbitsPrefixName(Array) *arr, size_t pos, size_t n)
{
	uint8_t *old, *new;
	size_t nBytesAdded, nBytesRemaining;

	if ((arr->len + n) >= arr->cap) {
		size_t nGrow;

		// always grow by a perfect multiple of arr->nGrow
		nGrow = n + (arr->nGrow - (n % arr->nGrow));
		arr->buf = sharedbitsPrefixName(Realloc)(arr->buf,
			arr->cap * arr->elemsize,
			(arr->cap + nGrow) * arr->elemsize,
			arr->what);
		arr->cap += nGrow;
	}

	nBytesRemaining = (arr->len - pos) * arr->elemsize;
	nBytesAdded = n * arr->elemsize;
	new = ((uint8_t *) (arr->buf)) + (pos * arr->elemsize);
	old = new + nBytesAdded;
	memmove(old, new, nBytesRemaining);
	memset(new, 0, nBytesAdded);

	arr->len += n;
	return new;
}

void sharedbitsPrefixName(ArrayDelete)(sharedbitsPrefixName(Array) *arr, size_t pos, size_t n)
{
	uint8_t *src, *dest;
	size_t nBytesDeleted, nBytesRemaining;

	nBytesDeleted = n * arr->elemsize;
	nBytesRemaining = (arr->len - pos - n) * arr->elemsize;
	dest = ((uint8_t *) (arr->buf)) + (pos * arr->elemsize);
	src = dest + nBytesDeleted;
	memmove(dest, src, nBytesRemaining);
	src = dest + nBytesRemaining;
	memset(src, 0, nBytesDeleted);
	arr->len -= n;
}

void sharedbitsPrefixName(ArrayDeleteItem)(sharedbitsPrefixName(Array) *arr, void *p, size_t n)
{
	uint8_t *p8, *buf8;

	p8 = (uint8_t *) p;
	buf8 = (uint8_t *) (arr->buf);
	// TODO write this in a way that doesn't mix ptrdiff_t and size_t
	sharedbitsPrefixName(ArrayDelete)(arr, (p8 - buf8) / arr->elemsize, n);
}

void *sharedbitsPrefixName(ArrayBsearch)(const sharedbitsPrefixName(Array) *arr, const void *key, int (*compare)(const void *, const void *))
{
	if (arr->len == 0)
		return NULL;
	return bsearch(key, arr->buf, arr->len, arr->elemsize, compare);
}

void sharedbitsPrefixName(ArrayQsort)(sharedbitsPrefixName(Array) *arr, int (*compare)(const void *, const void *))
{
	if (arr->len != 0)
		qsort(arr->buf, arr->len, arr->elemsize, compare);
}

#include "end.h"
