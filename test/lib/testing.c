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
	int verbose;
	testingprivOutbuf *outbuf;
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

struct testingSet {
	testingprivArray tests;
};

static testingSet mainTests = { testingprivArrayStaticInit(testingT, 32, "testingT[]") };

void testingprivSetRegisterTest(testingSet **pset, const char *name, void (*f)(testingT *), const char *file, long line)
{
	testingSet *set;
	testingT *t;

	set = &mainTests;
	if (pset != NULL) {
		set = *pset;
		if (set == NULL) {
			set = testingprivNew(testingSet);
			testingprivArrayInit(set->tests, testingT, 32, "testingT[]");
			*pset = set;
		}
	}
	t = (testingT *) testingprivArrayAppend(&(set->tests), 1);
	initTest(t, name, f, file, line);
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

static int testingprivTRun(testingT *t, const testingOptions *options, testingprivOutbuf *parentbuf)
{
	const char *status;
	timerTime start, end;
	char timerstr[timerDurationStringLen];
	int printStatus;

	if (options->Verbose)
		testingprivOutbufPrintf(parentbuf, "=== RUN   %s\n", t->name);
	t->verbose = options->Verbose;
	t->outbuf = testingprivNewOutbuf();

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
	} else if (t->skipped)
		// note that failed overrides skipped
		status = "SKIP";
	timerDurationString(timerTimeSub(end, start), timerstr);
	if (printStatus) {
		testingprivOutbufPrintf(parentbuf, "--- %s: %s (%s)\n", status, t->name, timerstr);
		testingprivOutbufAppendOutbuf(parentbuf, t->outbuf);
	}

	testingprivOutbufFree(t->outbuf);
	t->outbuf = NULL;
	return t->failed;
}

static void testingprivSetRun(testingSet *set, const testingOptions *options, testingprivOutbuf *outbuf, int *anyFailed)
{
	size_t i;
	testingT *t;

	testingprivArrayQsort(&(set->tests), testcmp);
	t = (testingT *) (set->tests.buf);
	for (i = 0; i < set->tests.len; i++) {
		if (!testingprivTRun(t, options, outbuf))
			*anyFailed = 1;
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
	if (set->tests.len == 0)
		return;
	testingprivSetRun(set, options, NULL, anyFailed);
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
	// TODO extract filename from file
	testingprivOutbufPrintf(t->outbuf, "%s:%ld: ", file, line);
	testingprivOutbufPrintf(t->outbuf, format, ap);
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
