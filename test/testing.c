// 27 february 2018
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include "testing.h"

#define testingprivNew(T) ((T *) malloc(sizeof (T)))

struct defer {
	void (*f)(testingT *, void *);
	void *data;
	struct defer *next;
};

struct testingT {
	const char *name;
	void (*f)(testingT *);
	int failed;
	int skipped;
	int returned;
	jmp_buf returnNowBuf;
	struct defer *defers;
	int defersRun;
	testingT *next;
};

static testingT *tests = NULL;
static testingT *testsTail = NULL;

static testingT *newTest(const char *name, void (*f)(testingT *), testingT *prev)
{
	testingT *t;

	t = testingprivNew(testingT);
	t->name = name;
	t->f = f;
	t->failed = 0;
	t->skipped = 0;
	t->returned = 0;
	t->defers = NULL;
	t->defersRun = 0;
	t->next = NULL;
	if (prev != NULL)
		prev->next = t;
	return t;
}

void testingprivRegisterTest(const char *name, void (*f)(testingT *))
{
	testingT *t;

	t = newTest(name, f, testsTail);
	testsTail = t;
	if (tests == NULL)
		tests = t;
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

static void runTestSet(testingT *t, int *anyFailed)
{
	const char *status;

	for (; t != NULL; t = t->next) {
		printf("=== RUN   %s\n", t->name);
		if (setjmp(t->returnNowBuf) == 0)
			(*(t->f))(t);
		t->returned = 1;
		runDefers(t);
		status = "PASS";
		if (t->failed) {
			status = "FAIL";
			*anyFailed = 1;
		} else if (t->skipped)
			// note that failed overrides skipped
			status = "SKIP";
		printf("--- %s: %s (%s)\n", status, t->name, "TODO");
	}
}

int testingMain(void)
{
	int anyFailed;

	// TODO see if this should run if all tests are skipped
	if (tests == NULL) {
		fprintf(stderr, "warning: no tests to run\n");
		// imitate Go here (TODO confirm this)
		return 0;
	}

	anyFailed = 0;
	runTestSet(tests, &anyFailed);
	if (anyFailed) {
		printf("FAIL\n");
		return 1;
	}
	printf("PASS\n");
	return 0;
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
	printf("\t%s:%ld: ", file, line);
	// TODO split into lines separated by \n\t\t and trimming trailing empty lines
	vprintf(format, ap);
	printf("\n");
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

void testingprivTFailNow(testingT *t)
{
	testingTFail(t);
	returnNow(t);
}

void testingprivTSkipNow(testingT *t)
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
