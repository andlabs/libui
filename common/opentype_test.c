// 27 february 2018
#ifndef TODO_TEST
#error TODO this is where libui itself goes
#endif

#include <stdarg.h>

#undef testingprivBadLanguageVersion
#ifdef __cplusplus
// TODO https://stackoverflow.com/questions/2324658/how-to-determine-the-version-of-the-c-standard-used-by-the-compiler implies this won't do with C++0x-era compilers, and https://wiki.apache.org/stdcxx/C++0xCompilerSupport doesn't talk about va_copy() so a simple version check for the C99 preprocessor may be wrong...
// TODO what if __cplusplus is blank (maybe only in that case, since IIRC C++98 requires __cplusplus to have a value)?
#if __cplusplus < 201103L
#define testingprivBadLanguageVersion
#endif
#elif !defined(__STDC_VERSION__)
#define testingprivBadLanguageVersion
#elif __STDC_VERSION__ < 199901L
#define testingprivBadLanguageVersion
#endif
#ifdef testingprivBadLanguageVersion
#error sorry, TODO requires either C99 or TODO; cannot continue
#endif

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
#define testingTErrorf(t, ...) testingprivTErrorfFull(t, __FILE__, __LINE__, __VA_ARGS__)
#define testingTErrorvf(t, format, ap) testingprivTErrorvfFull(t, __FILE__, __LINE__, format, ap)

// TODO should __LINE__ arguments use intmax_t or uintmax_t instead of int?
extern void testingprivRegisterTest(const char *, void (*)(testingT *));
extern void testingprivTErrorfFull(testingT *, const char *, int, const char *, ...);
extern void testingprivTErrorvfFull(testingT *, const char *, int, const char *, va_list);

#ifdef __cplusplus
}
#endif

testingTest(Name)
{
	printf("in the test!\n");
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
