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

	nBytesRemaining = (arr->len - pos) * arr->elemsize;
	nBytesAdded = n * arr->elemsize;
	new = ((uint8_t *) (arr->buf)) + (pos * arr->elemsize);
	old = new + nBytesAdded;
	memmove(old, new, nBytesRemaining);
	memset(new, 0, nBytesAdded);

	arr->len += n;
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

char *testingprivVsmprintf(const char *fmt, va_list ap)
{
	char *s;
	va_list ap2;
	int n;

	va_copy(ap2, ap);
	n = testingprivVsnprintf(NULL, 0, fmt, ap2);
	va_end(ap2);
	s = (char *) testingprivAlloc((n + 1) * sizeof (char), "char[]");
	testingprivVsnprintf(s, n + 1, fmt, ap);
	return s;
}

char *testingprivSmprintf(const char *fmt, ...)
{
	char *s;
	va_list ap;

	va_start(ap, fmt);
	s = testingprivVsmprintf(fmt, ap);
	va_end(ap);
	return s;
}

struct testingprivOutbuf {
	testingprivArray buf;
};

testingprivOutbuf *testingprivNewOutbuf(void)
{
	testingprivOutbuf *o;

	o = testingprivNew(testingprivOutbuf);
	testingprivArrayInit(o->buf, char, 32, "testing output buffer");
	return o;
}

void testingprivOutbufFree(testingprivOutbuf *o)
{
	testingprivArrayFree(o->buf);
	testingprivFree(o);
}

void testingprivOutbufVprintf(testingprivOutbuf *o, const char *fmt, va_list ap)
{
	char *dest;
	va_list ap2;
	int n;

	if (o == NULL) {
		vprintf(fmt, ap);
		return;
	}
	va_copy(ap2, ap);
	n = testingprivVsnprintf(NULL, 0, fmt, ap2);
	va_end(ap2);
	// To conserve memory, we only allocate the terminating NUL once.
	if (o->buf.len == 0)
		dest = (char *) testingprivArrayAppend(&(o->buf), n + 1);
	else {
		dest = (char *) testingprivArrayAppend(&(o->buf), n);
		dest--;
	}
	testingprivVsnprintf(dest, n + 1, fmt, ap);
}

void testingprivOutbufPrintf(testingprivOutbuf *o, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	testingprivOutbufVprintf(o, fmt, ap);
	va_end(ap);
}

// TODO right now this assumes the last character in o before calling this is a newline
void testingprivOutbufAppendOutbuf(testingprivOutbuf *o, testingprivOutbuf *src)
{
	char *buf;
	size_t n;
	int hasTrailingBlankLine;
	size_t trailingBlankLinePos;
	char *lineStart, *lineEnd;

	buf = src->buf.buf;
	n = src->buf.len;
	if (n == 0)
		// nothing to write
		return;

	// strip trailing blank lines, if any
	hasTrailingBlankLine = 0;
	if (buf[n - 1] == '\n') {
		hasTrailingBlankLine = 1;
		while (n > 0 && buf[n - 1] == '\n')
			n--;
		if (n == 0) {
			// the buffer only has blank lines, so just add a single newline and be done with it
			// TODO verify that this is the correct behavior
			testingprivOutbufPrintf(o, "\n");
			return;
		}
		trailingBlankLinePos = n;
		buf[trailingBlankLinePos] = '\0';
	}

	lineStart = buf;
	for (;;) {
		lineEnd = strchr(lineStart, '\n');
		if (lineEnd == NULL)			// last line
			break;
		*lineEnd = '\0';
		testingprivOutbufPrintf(o, "    %s\n", lineStart);
		// be sure to restore src to its original state
		*lineEnd = '\n';
		lineStart = lineEnd + 1;
	}
	// print the last line, if any
	if (*lineStart != '\0')
		testingprivOutbufPrintf(o, "    %s\n", lineStart);

	// restore src to its original state
	if (hasTrailingBlankLine)
		buf[trailingBlankLinePos] = '\n';
}
