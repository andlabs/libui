// 27 february 2018

#include <stdarg.h>
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
	int Verbose;
};

extern void testingSetRun(testingSet *set, const struct testingOptions *options, int *anyRun, int *anyFailed);

typedef struct testingT testingT;
#define testingTLogf(t, ...) \
	testingprivExpand(testingprivTLogfThen((void), t, __VA_ARGS__))
#define testingTLogvf(t, format, ap) \
	testingprivTLogvfThen((void), t, format, ap)
#define testingTErrorf(t, ...) \
	testingprivExpand(testingprivTLogfThen(testingTFail, t, __VA_ARGS__))
#define testingTErrorvf(t, format, ap) \
	testingprivTLogvfThen(testingTFail, t, format, ap)
#define testingTFatalf(t, ...) \
	testingprivExpand(testingprivTLogfThen(testingTFailNow, t, __VA_ARGS__))
#define testingTFatalvf(t, format, ap) \
	testingprivTLogvfThen(testingTFailNow, t, format, ap)
#define testingTSkipf(t, ...) \
	testingprivExpand(testingprivTLogfThen(testingTSkipNow, t, __VA_ARGS__))
#define testingTSkipvf(t, format, ap) \
	testingprivTLogvfThen(testingTSkipNow, t, format, ap)
extern void testingTFail(testingT *t);
extern void testingTFailNow(testingT *t);
extern void testingTSkipNow(testingT *t);
extern void testingTDefer(testingT *t, void (*f)(testingT *t, void *data), void *data);
extern void testingTRun(testingT *t, const char *subname, void (*subfunc)(testingT *t, void *data), void *data);

extern void testingprivSetRegisterTest(testingSet **pset, const char *, void (*)(testingT *, void *), void *, const char *, long);
// see https://stackoverflow.com/questions/32399191/va-args-expansion-using-msvc
#define testingprivExpand(x) x
#define testingprivTLogfThen(then, t, ...) ((testingprivTLogfFull(t, __FILE__, __LINE__, __VA_ARGS__)), (then(t)))
#define testingprivTLogvfThen(then, t, format, ap) ((testingprivTLogvfFull(t, __FILE__, __LINE__, format, ap)), (then(t)))
extern void testingprivTLogfFull(testingT *, const char *, long, const char *, ...);
extern void testingprivTLogvfFull(testingT *, const char *, long, const char *, va_list);
