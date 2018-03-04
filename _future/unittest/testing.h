// 27 february 2018

#ifndef testingprivIncludeGuard_testing_h
#define testingprivIncludeGuard_testing_h

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
#error sorry, TODO requires either C99 or C++11; cannot continue
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
#define testingprivMkScaffold(name) \
	static inline void testingprivScaffold ## name(testingT *t) \
	{ \
		bool failedNow = false, skippedNow = false; \
		try { name(t); } \
		catch (testingprivFailNowException e) { failedNow = true; } \
		catch (testingprivSkipNowException e) { skippedNow = true; } \
		/* TODO see if we should catch other exceptions too */ \
		/* don't call these in the catch blocks as they call longjmp() */ \
		if (failedNow) testingprivTDoFailNow(t); \
		if (skippedNow) testingprivTDoSkipNow(t); \
	}
#else
#define testingprivMkScaffold(name) \
	static inline void testingprivScaffold ## name(testingT *t) { name(t); }
#endif

// references:
// - https://gitlab.gnome.org/GNOME/glib/blob/master/glib/gconstructor.h
// - https://gitlab.gnome.org/GNOME/glib/blob/master/gio/glib-compile-resources.c
// - https://msdn.microsoft.com/en-us/library/bb918180.aspx
#if defined(__cplusplus)
#define testingprivMkCtor(name, reg) \
	static reg ## Class testingprivCtor ## name(#name, testingprivScaffold ## name);
#elif defined(__GNUC__)
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
#ifdef __cplusplus
#define testingTFailNow(t) (throw testingprivFailNowException())
#define testingTSkipNow(t) (throw testingprivSkipNowException())
#else
#define testingTFailNow(t) (testingprivTDoFailNow(t))
#define testingTSkipNow(t) (testingprivTDoSkipNow(t))
#endif
// TODO should the defered function also have t passed to it?
extern void testingTDefer(testingT *t, void (*f)(void *data), void *data);

// TODO IEEE 754 helpers
// references:
// - https://www.sourceware.org/ml/libc-alpha/2009-04/msg00005.html
// - https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
// - https://stackoverflow.com/questions/5085533/is-a-c-preprocessor-identical-to-a-c-preprocessor

// TODO should __LINE__ arguments use intmax_t or uintmax_t instead of int?
extern void testingprivRegisterTest(const char *, void (*)(testingT *));
// see https://stackoverflow.com/questions/32399191/va-args-expansion-using-msvc
#define testingprivExpand(x) x
#define testingprivTLogfThen(then, t, ...) ((testingprivTLogfFull(t, __FILE__, __LINE__, __VA_ARGS__)), (then(t)))
#define testingprivTLogvfThen(then, t, format, ap) ((testingprivTLogvfFull(t, __FILE__, __LINE__, format, ap)), (then(t)))
extern void testingprivTLogfFull(testingT *, const char *, int, const char *, ...);
extern void testingprivTLogvfFull(testingT *, const char *, int, const char *, va_list);
extern void testingprivTDoFailNow(testingT *);
extern void testingprivTDoSkipNow(testingT *);

#ifdef __cplusplus
}
namespace {
	class testingprivFailNowException {};
	class testingprivSkipNowException {};
	class testingprivRegisterTestClass {
	public:
		testingprivRegisterTestClass(const char *name, void (*f)(testingT *)) { testingprivRegisterTest(name, f); }
	};
}
#endif

#endif
