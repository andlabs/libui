// 27 february 2018
// TODO get rid of the need for this (it temporarily silences noise so I can find actual build issues)
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include "timer.h"
#include "testing.h"
#include "testingpriv.h"

// a struct outbuf of NULL writes directly to stdout
struct outbuf {
	char *buf;
	size_t len;
	size_t cap;
};

#define nOutbufGrow 32

static void outbufCopyStr(struct outbuf *o, const char *str, size_t len)
{
	size_t grow;

	if (len == 0)
		return;
	if (o == NULL) {
		printf("%s", str);
		return;
	}
	grow = len + 1;
	if (grow < nOutbufGrow)
		grow = nOutbufGrow;
	if ((o->len + grow) >= o->cap) {
		size_t prevcap;

		prevcap = o->cap;
		o->cap += grow;
		o->buf = testingprivResizeArray(o->buf, char, prevcap, o->cap);
	}
	memmove(o->buf + o->len, str, len * sizeof (char));
	o->len += len;
}

#define outbufAddNewline(o) outbufCopyStr(o, "\n", 1)

static void outbufAddIndent(struct outbuf *o, int n)
{
	for (; n != 0; n--)
		outbufCopyStr(o, "    ", 4);
}

static void outbufVprintf(struct outbuf *o, int indent, const char *format, va_list ap)
{
	va_list ap2;
	char *buf;
	int n;
	char *lineStart, *lineEnd;
	int firstLine = 1;

	va_copy(ap2, ap);
	n = testingprivVsnprintf(NULL, 0, format, ap2);
	// TODO handle n < 0 case
	va_end(ap2);
	buf = testingprivNewArray(char, n + 1);
	testingprivVsnprintf(buf, n + 1, format, ap);

	// strip trailing blank lines
	while (buf[n - 1] == '\n')
		n--;
	buf[n] = '\0';

	lineStart = buf;
	for (;;) {
		lineEnd = strchr(lineStart, '\n');
		if (lineEnd == NULL)			// last line
			break;
		*lineEnd = '\0';
		outbufAddIndent(o, indent);
		outbufCopyStr(o, lineStart, lineEnd - lineStart);
		outbufAddNewline(o);
		lineStart = lineEnd + 1;
		if (firstLine) {
			// subsequent lines are indented twice
			indent++;
			firstLine = 0;
		}
	}
	// print the last line
	outbufAddIndent(o, indent);
	outbufCopyStr(o, lineStart, strlen(lineStart));
	outbufAddNewline(o);

	testingprivFree(buf);
}

static void outbufPrintf(struct outbuf *o, int indent, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	outbufVprintf(o, indent, format, ap);
	va_end(ap);
}

static void outbufCopy(struct outbuf *o, const struct outbuf *from)
{
	outbufCopyStr(o, from->buf, from->len);
}

struct defer {
	void (*f)(testingT *, void *);
	void *data;
	struct defer *next;
};

#ifdef _MSC_VER
// Microsoft defines jmp_buf with a __declspec(align()), and for whatever reason, they have a warning that triggers when you use that for any reason, and that warning is enabled with /W4
// Silence the warning; it's harmless.
#pragma warning(push)
#pragma warning(disable: 4324)
#endif

struct testingT {
	// set at test creation time
	const char *name;
	char *computedName;
	void (*f)(testingT *);
	const char *file;
	long line;

	// test status
	int failed;
	int skipped;
	int returned;
	jmp_buf returnNowBuf;

	// deferred functions
	struct defer *defers;
	int defersRun;

	// output
	int indent;
	int verbose;
	struct outbuf output;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

static void initTest(testingT *t, const char *name, void (*f)(testingT *), const char *file, long line)
{
	memset(t, 0, sizeof (testingT));
	t->name = name;
	t->computedName = testingprivStrdup(name);
	t->f = f;
	t->file = file;
	t->line = line;
}

#define nGrow 32

struct testingSet {
	testingT *tests;
	size_t len;
	size_t cap;
};

static testingSet mainTests = { NULL, 0, 0 };

void testingprivSetRegisterTest(testingSet **pset, const char *name, void (*f)(testingT *), const char *file, long line)
{
	testingSet *set;

	set = &mainTests;
	if (pset != NULL) {
		set = *pset;
		if (set == NULL) {
			set = testingprivNew(testingSet);
			*pset = set;
		}
	}
	if (set->len == set->cap) {
		size_t prevcap;

		prevcap = set->cap;
		set->cap += nGrow;
		set->tests = testingprivResizeArray(set->tests, testingT, prevcap, set->cap);
	}
	initTest(set->tests + set->len, name, f, file, line);
	set->len++;
}

static int testcmp(const void *a, const void *b)
{
	const testingT *ta = (const testingT *) a;
	const testingT *tb = (const testingT *) b;
	int ret;

	ret = strcmp(ta->file, tb->file);
	if (ret != 0)
		return ret;
	if (ta->line < tb->line)
		return -1;
	if (ta->line > tb->line)
		return 1;
	return 0;
}

static void runDefers(testingT *t)
{
	struct defer *d;

	if (t->defersRun)
		return;
	t->defersRun = 1;
	for (d = t->defers; d != NULL; d = d->next)
		(*(d->f))(t, d->data);
}

static const testingOptions defaultOptions = {
	.Verbose = 0,
};

static void testingprivSetRun(testingSet *set, const testingOptions *options, int indent, int *anyFailed)
{
	size_t i;
	testingT *t;
	const char *status;
	timerTime start, end;
	char timerstr[timerDurationStringLen];
	int printStatus;

	qsort(set->tests, set->len, sizeof (testingT), testcmp);
	t = set->tests;
	for (i = 0; i < set->len; i++) {
		if (options->Verbose)
			outbufPrintf(NULL, indent, "=== RUN   %s", t->name);
		t->indent = indent + 1;
		t->verbose = options->Verbose;
		start = timerMonotonicNow();
		if (setjmp(t->returnNowBuf) == 0)
			(*(t->f))(t);
		end = timerMonotonicNow();
		t->returned = 1;
		runDefers(t);
		printStatus = t->verbose;
		status = "PASS";
		if (t->failed) {
			status = "FAIL";
			printStatus = 1;			// always print status on failure
			*anyFailed = 1;
		} else if (t->skipped)
			// note that failed overrides skipped
			status = "SKIP";
		timerDurationString(timerTimeSub(end, start), timerstr);
		if (printStatus) {
			outbufPrintf(NULL, indent, "--- %s: %s (%s)", status, t->name, timerstr);
			outbufCopy(NULL, &(t->output));
		}
		t++;
	}
}

void testingSetRun(testingSet *set, const struct testingOptions *options, int *anyRun, int *anyFailed)
{
	*anyRun = 0;
	*anyFailed = 0;
	if (set == NULL)
		set = &mainTests;
	if (options == NULL)
		options = &defaultOptions;
	if (set->len == 0)
		return;
	testingprivSetRun(set, options, 0, anyFailed);
	*anyRun = 1;
}

void testingprivTLogfFull(testingT *t, const char *file, long line, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	testingprivTLogvfFull(t, file, line, format, ap);
	va_end(ap);
}

void testingprivTLogvfFull(testingT *t, const char *file, long line, const char *format, va_list ap)
{
	va_list ap2;
	char *buf;
	int n, n2;

	// TODO extract filename from file
	n = testingprivSnprintf(NULL, 0, "%s:%ld: ", file, line);
	// TODO handle n < 0 case
	va_copy(ap2, ap);
	n2 = testingprivVsnprintf(NULL, 0, format, ap2);
	// TODO handle n2 < 0 case
	va_end(ap2);
	buf = testingprivNewArray(char, n + n2 + 1);
	testingprivSnprintf(buf, n + 1, "%s:%ld: ", file, line);
	testingprivVsnprintf(buf + n, n2 + 1, format, ap);
	outbufPrintf(&(t->output), t->indent, "%s", buf);
	testingprivFree(buf);
}

void testingTFail(testingT *t)
{
	t->failed = 1;
}

static void returnNow(testingT *t)
{
	if (!t->returned) {
		// set this now so a FailNow inside a Defer doesn't longjmp twice
		t->returned = 1;
		// run defers before calling longjmp() just to be safe
		runDefers(t);
		longjmp(t->returnNowBuf, 1);
	}
}

void testingTFailNow(testingT *t)
{
	testingTFail(t);
	returnNow(t);
}

void testingTSkipNow(testingT *t)
{
	t->skipped = 1;
	returnNow(t);
}

void testingTDefer(testingT *t, void (*f)(testingT *t, void *data), void *data)
{
	struct defer *d;

	d = testingprivNew(struct defer);
	d->f = f;
	d->data = data;
	// add to the head of the list so defers are run in reverse order of how they were added
	d->next = t->defers;
	t->defers = d;
}
