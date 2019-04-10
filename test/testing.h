// 27 february 2018

// TODO
// - https://blogs.msdn.microsoft.com/oldnewthing/20181107-00/?p=100155 https://blogs.msdn.microsoft.com/oldnewthing/20181108-00/?p=100165 https://blogs.msdn.microsoft.com/oldnewthing/20181109-00/?p=100175
// - also in the above: note the unspecified order of data in the sub-segments...

#include <stdarg.h>

#define testingprivFullName(basename) testingprivImpl ## basename

#define testingprivScaffoldName(basename) testingprivScaffold ## basename
#define testingprivMkScaffold(basename, argtype, argname) \
	static inline void testingprivScaffoldName(basename)(argtype *argname) { testingprivFullName(basename)(argname); }

// references:
// - https://gitlab.gnome.org/GNOME/glib/blob/master/glib/gconstructor.h
// - https://gitlab.gnome.org/GNOME/glib/blob/master/gio/glib-compile-resources.c
// - https://msdn.microsoft.com/en-us/library/bb918180.aspx
#define testingprivCtorName(basename) testingprivCtor ## basename
#define testingprivCtorPtrName(basename) testingprivCtorPtr ## basename
#if defined(__GNUC__)
#define testingprivMkCtor(basename, regfunc) \
	__attribute__((constructor)) static void testingprivCtorName(basename)(void) { regfunc(#basename, testingprivScaffoldName(basename)); }
#elif defined(_MSC_VER)
#define testingprivMkCtor(basename, reg) \
	static int testingprivCtorName(basename)(void) { regfunc(#basename, testingprivScaffoldName(basename)); return 0; } \
	__pragma(section(".CRT$XCU",read)) \
	__declspec(allocate(".CRT$XCU")) static int (*testingprivCtorPtrName(basename))(void) = testingprivCtorName(basename);
#else
#error unknown compiler for making constructors in C; cannot continue
#endif

#define testingprivMk(basename, argtype, argname, regfunc) \
	void testingprivImplName(basename)(argtype *argname); \
	testingprivMkScaffold(basename, argtype, argname) \
	testingprivMkCtor(basename, regfunc) \
	void testingprivImplName(basename)(argtype *argname)

#define testingTest(Name) \
	testingprivMk(Test ## Name, testingT, t, testingprivRegisterTest)

extern int testingMain(void);

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
// TODO should the defered function also have t passed to it?
extern void testingTDefer(testingT *t, void (*f)(void *data), void *data);

extern void testingprivRegisterTest(const char *, void (*)(testingT *));
extern void testingprivRegisterManualTest(const char *, void (*)(testingT *));
// see https://stackoverflow.com/questions/32399191/va-args-expansion-using-msvc
#define testingprivExpand(x) x
#define testingprivTLogfThen(then, t, ...) ((testingprivTLogfFull(t, __FILE__, __LINE__, __VA_ARGS__)), (then(t)))
#define testingprivTLogvfThen(then, t, format, ap) ((testingprivTLogvfFull(t, __FILE__, __LINE__, format, ap)), (then(t)))
extern void testingprivTLogfFull(testingT *, const char *, long, const char *, ...);
extern void testingprivTLogvfFull(testingT *, const char *, long, const char *, va_list);
