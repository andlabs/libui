// 19 may 2019
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "testing.h"
#include "testingpriv.h"

void testingprivInternalError(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "** testing internal error: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "; aborting\n");
	va_end(ap);
	abort();
}

void *testingprivAlloc(size_t n, const char *what)
{
	void *p;

	p = malloc(n);
	if (p == NULL)
		testingprivInternalError("memory exhausted allocating %s", what);
	memset(p, 0, n);
	return p;
}

void *testingprivRealloc(void *p, size_t old, size_t new, const char *what)
{
	p = realloc(p, new);
	if (p == NULL)
		testingprivInternalError("memory exhausted reallocating %s", what);
	if (new > old)
		memset(((uint8_t *) p) + old, 0, new - old);
	return p;
}

void testingprivFree(void *p)
{
	free(p);
}

void *testingprivArrayAppend(testingprivArray *arr, size_t n)
{
	return testingprivArrayInsertAt(arr, arr->len, n);
}

void *testingprivArrayInsertAt(testingprivArray *arr, size_t pos, size_t n)
{
	uint8_t *old, *new;
	size_t nBytesAdded, nBytesRemaining;

	if ((arr->len + n) >= arr->cap) {
		size_t nGrow;

		// always grow by a perfect multiple of arr->nGrow
		nGrow = n + (arr->nGrow - (n % arr->nGrow));
		arr->buf = testingprivRealloc(arr->buf,
			arr->cap * arr->elemsize,
			(arr->cap + nGrow) * arr->elemsize,
			arr->what);
		arr->cap += nGrow;
	}
	arr->len += n;

	nBytesRemaining = (arr->len - pos) * arr->elemsize;
	nBytesAdded = n * arr->elemsize;
	new = ((uint8_t *) (arr->buf)) + (pos * arr->elemsize);
	old = new + nBytesAdded;
	memmove(old, new, nBytesRemaining);
	memset(new, 0, nBytesAdded);
	return new;
}

void testingprivArrayDelete(testingprivArray *arr, size_t pos, size_t n)
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

void testingprivArrayDeleteItem(testingprivArray *arr, void *p, size_t n)
{
	uint8_t *p8, *buf8;

	p8 = (uint8_t *) p;
	buf8 = (uint8_t *) (arr->buf);
	// TODO write this in a way that doesn't mix ptrdiff_t and size_t
	testingprivArrayDelete(arr, (p8 - buf8) / arr->elemsize, n);
}

void *testingprivArrayBsearch(const testingprivArray *arr, const void *key, int (*compare)(const void *, const void *))
{
	return bsearch(key, arr->buf, arr->len, arr->elemsize, compare);
}

void testingprivArrayQsort(testingprivArray *arr, int (*compare)(const void *, const void *))
{
	qsort(arr->buf, arr->len, arr->elemsize, compare);
}

int testingprivVsnprintf(char *s, size_t n, const char *format, va_list ap)
{
	int ret;

	ret = vsnprintf(s, n, format, ap);
	if (ret < 0)
		testingprivInternalError("encoding error in vsnprintf(); this likely means your call to testingTLogf() and the like is invalid");
	return ret;
}

int testingprivSnprintf(char *s, size_t n, const char *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = testingprivVsnprintf(s, n, format, ap);
	va_end(ap);
	return ret;
}

char *testingprivStrdup(const char *s)
{
	char *t;

	t = (char *) testingprivAlloc((strlen(s) + 1) * sizeof (char), "char[]");
	strcpy(t, s);
	return t;
}
