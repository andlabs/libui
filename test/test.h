// 27 february 2018

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ui.h"
#ifdef testingprivOSHeader
#include testingprivOSHeader
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define testingprivImplName(basename) testingprivImpl ## basename

// references:
// - https://gitlab.gnome.org/GNOME/glib/blob/master/glib/gconstructor.h
// - https://gitlab.gnome.org/GNOME/glib/blob/master/gio/glib-compile-resources.c
// - https://msdn.microsoft.com/en-us/library/bb918180.aspx
#define testingprivCtorName(basename) testingprivCtor ## basename
#define testingprivCtorPtrName(basename) testingprivCtorPtr ## basename
#if defined(__GNUC__)
#define testingprivMkCtor(basename) \
	__attribute__((constructor)) static void testingprivCtorName(basename)(void) { testingprivRegisterTest(#basename, testingprivImplName(basename)); }
#elif defined(_MSC_VER)
#define testingprivMkCtor(basename) \
	static int testingprivCtorName(basename)(void) { testingprivRegisterTest(#basename, testingprivImplName(basename)); return 0; } \
	__pragma(section(".CRT$XCU",read)) \
	__declspec(allocate(".CRT$XCU")) static int (*testingprivCtorPtrName(basename))(void) = testingprivCtorName(basename);
#elif defined(__SUNPRO_C)
#define testingprivMkCtor(basename) \
	static void testingprivCtorName(basename)(void) { testingprivRegisterTest(#basename, testingprivImplName(basename)); } \
	_Pragma(init(testingprivCtorName(basename)))
#else
#error unknown compiler for making constructors in C; cannot continue
#endif

#define Test(basename) \
	int testingprivImplName(basename)(void); \
	testingprivMkCtor(basename) \
	int testingprivImplName(basename)(void)

#define Test(Name) \
	testingprivMk(Test ## Name)

// These can only be called directly from the Test functions.
// TODO make it otherwise
#define TestFailNow() return 1
// see https://mesonbuild.com/Unit-tests.html#skipped-tests-and-hard-errors
#define TestSkipNow() return 77

extern void testingprivRegisterTest(const char *name, int (*f)(void));

#ifdef __cplusplus
}
#endif
