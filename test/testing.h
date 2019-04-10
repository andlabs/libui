// 27 february 2018

// TODO
// - https://blogs.msdn.microsoft.com/oldnewthing/20181107-00/?p=100155 https://blogs.msdn.microsoft.com/oldnewthing/20181108-00/?p=100165 https://blogs.msdn.microsoft.com/oldnewthing/20181109-00/?p=100175
// - also in the above: note the unspecified order of data in the sub-segments...

#include <stdarg.h>

#define testingprivMkScaffold(name) \
	static inline void testingprivScaffold ## name(testingT *t) { name(t); }

// references:
// - https://gitlab.gnome.org/GNOME/glib/blob/master/glib/gconstructor.h
// - https://gitlab.gnome.org/GNOME/glib/blob/master/gio/glib-compile-resources.c
// - https://msdn.microsoft.com/en-us/library/bb918180.aspx
#if defined(__GNUC__)
#define testingprivMkCtor(name, reg) \
	__attribute__((constructor)) static void testingprivCtor ## name(void) { reg(#name, testingprivScaffold ## name); }
#elif defined(_MSC_VER)
#define testingprivMkCtorPrototype(name, reg) \
	static int name(void) testingprivCtor ## name(void) { reg(#name, testingprivScaffold ## name); return 0; } \
	__pragma(section(".CRT$XCU",read)) \
	__declspec(allocate(".CRT$XCU")) static int (*testingprivCtorPtr ## name)(void) = testingprivCtor ## name;
#elif defined(__SUNPRO_C)
#define testingprivMkCtor(name, reg) \
	_Pragma("init(testingprivCtor" #name ")") static void testingprivCtor ## name(void) { reg(#name, testingprivScaffold ## name); }
#else
#error unknown compiler for making constructors in C; cannot continue
#endif

#define testingTest(Name) \
	void Test ## Name(testingT *t); \
	testingprivMkScaffold(Test ## Name) \
	testingprivMkCtor(Test ## Name, testingprivRegisterTest) \
	void Test ## Name(testingT *t)

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

// TODO should __LINE__ arguments use intmax_t or uintmax_t instead of int?
extern void testingprivRegisterTest(const char *, void (*)(testingT *));
// see https://stackoverflow.com/questions/32399191/va-args-expansion-using-msvc
#define testingprivExpand(x) x
#define testingprivTLogfThen(then, t, ...) ((testingprivTLogfFull(t, __FILE__, __LINE__, __VA_ARGS__)), (then(t)))
#define testingprivTLogvfThen(then, t, format, ap) ((testingprivTLogvfFull(t, __FILE__, __LINE__, format, ap)), (then(t)))
extern void testingprivTLogfFull(testingT *, const char *, int, const char *, ...);
extern void testingprivTLogvfFull(testingT *, const char *, int, const char *, va_list);
