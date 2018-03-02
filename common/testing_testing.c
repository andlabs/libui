// 27 february 2018
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include "testing.h"

#define testingprivNew(T) ((T *) malloc(sizeof (T)))

struct testingT {
	const char *name;
	void (*f)(testingT *);
	int failed;
	jmp_buf failNowBuf;
	testingT *next;
};

static testingT *tests = NULL;

void testingprivRegisterTest(const char *name, void (*f)(testingT *))
{
	testingT *t;

	t = testingprivNew(testingT);
	t->name = name;
	t->f = f;
	t->failed = 0;
	t->next = tests;
	tests = t;
}

int testingMain(void)
{
	testingT *t;
	int anyFailed;
	const char *status;

	if (tests == NULL) {
		fprintf(stderr, "warning: no tests to run\n");
		// imitate Go here (TODO confirm this)
		return 0;
	}

	anyFailed = 0;
	for (t = tests; t != NULL; t = t->next) {
		printf("=== RUN   %s\n", t->name);
		if (setjmp(t->failNowBuf) == 0)
			(*(t->f))(t);
		status = "PASS";
		if (t->failed) {
			status = "FAIL";
			anyFailed = 1;
		}
		printf("--- %s: %s (%s)\n", status, t->name, "TODO");
	}

	if (anyFailed) {
		printf("FAIL\n");
		return 1;
	}
	printf("PASS\n");
	return 0;
}

static void testingprivTDoLog(testingT *t, const char *file, int line, const char *format, va_list ap)
{
	// TODO extract filename from file
	printf("\t%s:%d: ", file, line);
	// TODO split into lines separated by \n\t\t and trimming trailing empty lines
	vprintf(format, ap);
	printf("\n");
}

void testingprivTErrorfFull(testingT *t, const char *file, int line, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	testingprivTErrorvfFull(t, file, line, format, ap);
	va_end(ap);
}

void testingprivTErrorvfFull(testingT *t, const char *file, int line, const char *format, va_list ap)
{
	testingprivTDoLog(t, file, line, format, ap);
	t->failed = 1;
}

void testingprivTDoFailNow(testingT *t)
{
	t->failed = 1;
	longjmp(t->failNowBuf, 1);
}
