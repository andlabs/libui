xx 27 february 2018
#ifndef TODO_TEST
#error TODO this is where libui itself goes
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__)
#define testingTest(Name) \
	void Test ## Name(testingT *t); \
	__attribute__((constructor)) static inline void testingprivCtorRegisterTest ## Name(void) { testingprivRegisterTest("Test" #Name, Test ## Name); } \
	void Test ## Name(testingT *t)
#else
#error unknown compiler; cannot continue
#endif

extern int testingMain(void);

typedef struct testingT testingT;
#define testingTErrorf(t, ...) testingprivTLogFull(t, __FILE__, __LINE__, __VA_ARGS__)
#define testingTErrorvf(t, format, ap) testingprivTLogFullv(t, __FILE__, __LINE__, format, ap)

extern void testingprivRegisterTest(const char *, void (*)(testi
ngT *));
extern void testingprivTLogFull(testingT *, const char *, int, const char *, ...);
extern void testingprivTLogFullv(testingT *, const char *, int, const char *, va_list);

#ifdef __cplusplus
}
#endif

testingTest(Name)
{
}

int main(void)
{
	return testingMain();
}

#include <stdio.h>
#include <stdlib.h>

#define testingprivNew(T) ((T *) malloc(sizeof (T)))

struct testingT {
	const char *name;
	void (*f)(testingT *);
	int failed;
	struct testingprivTest *next;
};

static testingT *tests = NULL;

void testingprivRegisterTest(const char *name, void (*f)(testingT *))
{
	testingT *t;

	t = testingprivNew(testingT)
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
		xx imitate Go here (TODO confirm this)
		return 0;
	}

	anyFailed = 0;
	for (t = tests; t != NULL; t = t->next) {
		printf("=== RUN   %s\n", t->name);
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

extern void testingprivTLogFull(testingT *t, const char *file, int line, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	testingprivTLogFullv(t, file, line, format, ap);
	va_end(ap);
}

void testingprivTLogFullv(testingT *t, const char *file, int line, const char *format, va_list ap)
{
}
