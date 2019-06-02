// 27 february 2018

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#define testingprivImplName(basename) testingprivImpl ## basename

#define testingprivScaffoldName(basename) testingprivScaffold ## basename
#define testingprivMkScaffold(basename, argtype, argname) \
	static void testingprivScaffoldName(basename)(argtype *argname, void *data) { testingprivImplName(basename)(argname); }

// references:
// - https://gitlab.gnome.org/GNOME/glib/blob/master/glib/gconstructor.h
// - https://gitlab.gnome.org/GNOME/glib/blob/master/gio/glib-compile-resources.c
// - https://msdn.microsoft.com/en-us/library/bb918180.aspx
#define testingprivCtorName(basename) testingprivCtor ## basename
#define testingprivCtorPtrName(basename) testingprivCtorPtr ## basename
#if defined(__GNUC__)
#define testingprivMkCtor(basename, pset) \
	__attribute__((constructor)) static void testingprivCtorName(basename)(void) { testingprivSetRegisterTest(pset, #basename, testingprivScaffoldName(basename), NULL, __FILE__, __LINE__); }
#elif defined(_MSC_VER)
#define testingprivMkCtor(basename, pset) \
	static int testingprivCtorName(basename)(void) { testingprivSetRegisterTest(pset, #basename, testingprivScaffoldName(basename), NULL, __FILE__, __LINE__); return 0; } \
	__pragma(section(".CRT$XCU",read)) \
	__declspec(allocate(".CRT$XCU")) static int (*testingprivCtorPtrName(basename))(void) = testingprivCtorName(basename);
#else
#error unknown compiler for making constructors in C; cannot continue
#endif

#define testingprivMk(basename, argtype, argname, pset) \
	void testingprivImplName(basename)(argtype *argname); \
	testingprivMkScaffold(basename, argtype, argname) \
	testingprivMkCtor(basename, pset) \
	void testingprivImplName(basename)(argtype *argname)

#define testingTest(Name) \
	testingprivMk(Test ## Name, testingT, t, NULL)
#define testingTestInSet(Set, Name) \
	testingprivMk(Test ## Name, testingT, t, &Set)

typedef struct testingSet testingSet;
typedef struct testingOptions testingOptions;

struct testingOptions {
	bool Verbose;
};

extern void testingSetRun(testingSet *set, const struct testingOptions *options, bool *anyRun, bool *anyFailed);

typedef struct testingT testingT;

#define testingTLogf(t, ...) \
	testingprivTLogfFullThen(t, NULL, __FILE__, __LINE__, __VA_ARGS__)
#define testingTLogvf(t, format, ap) \
	testingprivTLogvfFullThen(t, NULL, __FILE__, __LINE__, format, ap)
#define testingTLogfFull(t, file, line, ...) \
	testingprivTLogfFullThen(t, NULL, file, line, __VA_ARGS__)
#define testingTLogvfFull(t, file, line, format, ap) \
	testingprivTLogvfFullThen(t, NULL, file, line, format, ap)

#define testingTErrorf(t, ...) \
	testingprivTLogfFullThen(t, testingTFail, __FILE__, __LINE__, __VA_ARGS__)
#define testingTErrorvf(t, format, ap) \
	testingprivTLogvfFullThen(t, testingTFail, __FILE__, __LINE__, format, ap)
#define testingTErrorfFull(t, file, line, ...) \
	testingprivTLogfFullThen(t, testingTFail, file, line, __VA_ARGS__)
#define testingTErrorvfFull(t, file, line, format, ap) \
	testingprivTLogvfFullThen(t, testingTFail, file, line, format, ap)

#define testingTFatalf(t, ...) \
	testingprivTLogfFullThen(t, testingTFailNow, __FILE__, __LINE__, __VA_ARGS__)
#define testingTFatalvf(t, format, ap) \
	testingprivTLogvfFullThen(t, testingTFailNow, __FILE__, __LINE__, format, ap)
#define testingTFatalfFull(t, file, line, ...) \
	testingprivTLogfFullThen(t, testingTFailNow, file, line, __VA_ARGS__)
#define testingTFatalvfFull(t, file, line, format, ap) \
	testingprivTLogvfFullThen(t, testingTFailNow, file, line, format, ap)

#define testingTSkipf(t, ...) \
	testingprivTLogfFullThen(t, testingTSkipNow, __FILE__, __LINE__, __VA_ARGS__)
#define testingTSkipvf(t, format, ap) \
	testingprivTLogvfFullThen(t, testingTSkipNow, __FILE__, __LINE__, format, ap)
#define testingTSkipfFull(t, file, line, ...) \
	testingprivTLogfFullThen(t, testingTSkipNow, file, line, __VA_ARGS__)
#define testingTSkipvfFull(t, file, line, format, ap) \
	testingprivTLogvfFullThen(t, testingTSkipNow, file, line, format, ap)

extern void testingTFail(testingT *t);
extern void testingTFailNow(testingT *t);
extern void testingTSkipNow(testingT *t);

extern void testingTDefer(testingT *t, void (*f)(testingT *t, void *data), void *data);
extern void testingTRun(testingT *t, const char *subname, void (*subfunc)(testingT *t, void *data), void *data);

extern void testingprivSetRegisterTest(testingSet **pset, const char *, void (*)(testingT *, void *), void *, const char *, long);
#include "../../sharedbits/printfwarn_header.h"
sharedbitsPrintfFunc(
	extern void testingprivTLogfFullThen(testingT *, void (*)(testingT *), const char *, long, const char *, ...),
	5, 6);
#undef sharedbitsPrintfFunc
extern void testingprivTLogvfFullThen(testingT *, void (*)(testingT *), const char *, long, const char *, va_list);
