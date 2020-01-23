// 27 february 2018
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ui.h"
#ifdef libuiOSHeader
#include libuiOSHeader
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define testingprivImplName(basename) testingprivImpl ## basename
#define testingprivScaffoldName(basename) testingprivScaffold ## basename

// references:
// - https://gitlab.gnome.org/GNOME/glib/blob/master/glib/gconstructor.h
// - https://gitlab.gnome.org/GNOME/glib/blob/master/gio/glib-compile-resources.c
// - https://msdn.microsoft.com/en-us/library/bb918180.aspx
#define testingprivCtorName(basename) testingprivCtor ## basename
#define testingprivCtorPtrName(basename) testingprivCtorPtr ## basename
#if defined(__GNUC__)
#define testingprivMkCtor(basename) \
	__attribute__((constructor)) static void testingprivCtorName(basename)(void) { testingprivRegisterTest(#basename, testingprivScaffoldName(basename)); }
#elif defined(_MSC_VER)
#define testingprivMkCtor(basename) \
	static int testingprivCtorName(basename)(void) { testingprivRegisterTest(#basename, testingprivScaffoldName(basename)); return 0; } \
	__pragma(section(".CRT$XCU",read)) \
	__declspec(allocate(".CRT$XCU")) static int (*testingprivCtorPtrName(basename))(void) = testingprivCtorName(basename);
#elif defined(__SUNPRO_C)
#define testingprivMkCtor(basename) \
	static void testingprivCtorName(basename)(void) { testingprivRegisterTest(#basename, testingprivScaffoldName(basename)); } \
	_Pragma(init(testingprivCtorName(basename)))
#else
#error unknown compiler for making constructors in C; cannot continue
#endif

#define Test(Name) \
	void testingprivImplName(Test ## Name)(void); \
	static void testingprivScaffoldName(Test ## Name)(void) \
	{ \
		uiInitError err; \
		memset(&err, 0, sizeof (uiInitError)); \
		err.Size = sizeof (uiInitError); \
		if (!uiInit(NULL, &err)) { \
			fprintf(stderr, "error initializing libui for Test" #Name ": %s\n", err.Message); \
			TestFailNow(); \
		} \
		testingprivImplName(Test ## Name)(); \
	} \
	testingprivMkCtor(Test ## Name) \
	void testingprivImplName(Test ## Name)(void)

#define TestNoInit(Name) \
	void testingprivImplName(Test ## Name)(void); \
	static void testingprivScaffoldName(Test ## Name)(void) \
	{ \
		testingprivImplName(Test ## Name)(); \
	} \
	testingprivMkCtor(Test ## Name) \
	void testingprivImplName(Test ## Name)(void)

extern void TestFail(void);
extern void TestFailNow(void);
extern void TestSkipNow(void);

#define TestLogf(...) \
	(testingprivLogfFullThen(stdout, NULL, __FILE__, __LINE__, __VA_ARGS__))
#define TestErrorf(...) \
	(testingprivLogfFullThen(stderr, TestFail, __FILE__, __LINE__, __VA_ARGS__))
#define TestFatalf(...) \
	(testingprivLogfFullThen(stderr, TestFailNow, __FILE__, __LINE__, __VA_ARGS__))
// TODO remember if this needs to go to stdout or to stderr
#define TestSkipf(...) \
	(testingprivLogfFullThen(stderr, TestSkipNow, __FILE__, __LINE__, __VA_ARGS__))

extern void testingprivRegisterTest(const char *name, void (*f)(void));
#include "../sharedbits/printfwarn_header.h"
sharedbitsPrintfFunc(
	extern void testingprivLogfFullThen(FILE *f, void (*then)(void), const char *filename, long line, const char *fmt, ...),
	5, 6);
#undef sharedbitsPrintfFunc

// end of test framework definitions

#define diff(fmt) "\ngot  " fmt "\nwant " fmt

#ifdef __cplusplus
}
#endif
