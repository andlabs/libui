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

#define sharedbitsPrefix testingpriv
#include "../../sharedbits/alloc_impl.h"
#include "../../sharedbits/array_impl.h"
#undef sharedbitsPrefix

#define sharedbitsPrefix testingprivImpl
#define sharedbitsStatic static
#include "../../sharedbits/strsafe_impl.h"
#undef sharedbitsStatic
#undef sharedbitsPrefix

int testingprivVsnprintf(char *s, size_t n, const char *format, va_list ap)
{
	int ret;

	ret = testingprivImplVsnprintf(s, n, format, ap);
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
	size_t n;

	n = strlen(s);
	t = (char *) testingprivAlloc((n + 1) * sizeof (char), "char[]");
	strncpy(t, s, n + 1);
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

void testingprivOutbufVprintfIndented(testingprivOutbuf *o, const char *fmt, va_list ap)
{
	char *buf;
	char *lineStart, *lineEnd;
	const char *indent;

	buf = testingprivVsmprintf(fmt, ap);

	lineStart = buf;
	indent = "";
	for (;;) {
		lineEnd = strchr(lineStart, '\n');
		if (lineEnd == NULL)
			break;
		*lineEnd = '\0';
		testingprivOutbufPrintf(o, "%s%s\n", indent, lineStart);
		lineStart = lineEnd + 1;
		indent = "    ";
	}
	// and print the last line fragment, if any
	if (*lineStart != '\0')
		testingprivOutbufPrintf(o, "%s%s", indent, lineStart);
	testingprivFree(buf);
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
	bool hasTrailingBlankLine;
	size_t trailingBlankLinePos = 0;		// silence incorrect MSVC warning
	char *lineStart, *lineEnd;

	buf = src->buf.buf;
	n = src->buf.len;
	if (n == 0)
		// nothing to write
		return;

	// strip trailing blank lines, if any
	hasTrailingBlankLine = false;
	if (buf[n - 1] == '\n') {
		hasTrailingBlankLine = true;
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

const char *testingprivOutbufString(testingprivOutbuf *o)
{
	if (o->buf.buf == NULL)
		return "";
	return o->buf.buf;
}
