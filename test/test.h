// 27 february 2018
#include <errno.h>
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
#include "../common/testhooks.h"

#ifdef __cplusplus
extern "C" {
#endif

#define testingprivImplName(basename) testingprivImpl ## basename
#define testingprivScaffoldName(basename) testingprivScaffold ## basename

#define Test(Name) \
	static void testingprivImplName(Test ## Name)(void); \
	void testingprivScaffoldName(Test ## Name)(void) \
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
	static void testingprivImplName(Test ## Name)(void)

#define TestNoInit(Name) \
	static void testingprivImplName(Test ## Name)(void); \
	void testingprivScaffoldName(Test ## Name)(void) \
	{ \
		testingprivImplName(Test ## Name)(); \
	} \
	static void testingprivImplName(Test ## Name)(void)

extern void TestFail(void);
extern void TestFailNow(void);
extern void TestSkipNow(void);

#define TestLogf(...) \
	(testingprivLogfFullThen(stdout, NULL, __FILE__, __LINE__, __VA_ARGS__))
#define TestLogfFull(f, n, ...) \
	(testingprivLogfFullThen(stdout, NULL, f, n, __VA_ARGS__))
#define TestErrorf(...) \
	(testingprivLogfFullThen(stderr, TestFail, __FILE__, __LINE__, __VA_ARGS__))
#define TestErrorfFull(f, n, ...) \
	(testingprivLogfFullThen(stderr, TestFail, f, n, __VA_ARGS__))
#define TestFatalf(...) \
	(testingprivLogfFullThen(stderr, TestFailNow, __FILE__, __LINE__, __VA_ARGS__))
#define TestFatalfFull(f, n, ...) \
	(testingprivLogfFullThen(stderr, TestFailNow, f, n, __VA_ARGS__))
// TODO remember if this needs to go to stdout or to stderr
#define TestSkipf(...) \
	(testingprivLogfFullThen(stderr, TestSkipNow, __FILE__, __LINE__, __VA_ARGS__))
// TODO TestSkipfFull (after resolving above TODO)

extern void TestDefer(void (*f)(void *data), void *data);

#include "../sharedbits/printfwarn_header.h"
sharedbitsPrintfFunc(
	extern void testingprivLogfFullThen(FILE *f, void (*then)(void), const char *filename, long line, const char *fmt, ...),
	5, 6);
#undef sharedbitsPrintfFunc

#include "testlist.h"

// end of test framework definitions

#define diff(fmt) "\ngot  " fmt "\nwant " fmt

// errors.c
extern void *beginCheckProgrammerError(const char *want);
extern void endCheckProgrammerErrorFull(const char *file, long line, void *context);
#define endCheckProgrammerError(context) endCheckProgrammerErrorFull(__FILE__, __LINE__, context)

// controls.c
struct testControlImplData {
	const uiControlVtable *realVtable;
	const uiControlOSVtable *realOSVtable;
	void *realImplData;
};
extern const uiControlVtable *testControlVtable(void);
extern const uiControlOSVtable *testControlOSVtable(void);
extern uint32_t testControlType(void);

// utf8.c
extern const char testUTF8Empty[];
extern const char testUTF8ASCIIOnly[];
extern const char testUTF8WithTwoByte[];
extern const char testUTF8WithThreeByte[];
extern const char testUTF8WithFourByte[];
extern const char testUTF8Combined[];
extern const char testUTF8InvalidInput[];
extern const char testUTF8InvalidOutput[];
extern const uint16_t testUTF16Empty[];
extern const uint16_t testUTF16ASCIIOnly[];
extern const uint16_t testUTF16WithTwoByte[];
extern const uint16_t testUTF16WithThreeByte[];
extern const uint16_t testUTF16WithFourByte[];
extern const uint16_t testUTF16Combined[];
extern const uint16_t testUTF16InvalidOutput[];
extern bool utf8equal(const char *s, const char *t);
extern void utf8diffErrorFull(const char *file, long line, const char *msg, const char *got, const char *want);
#define utf8diffError(msg, got, want) utf8diffErrorFull(__FILE__, __LINE__, msg, got, want)
extern bool utf16equal(const uint16_t *s, const uint16_t *t);
extern void utf16diffErrorFull(const char *file, long line, const char *msg, const uint16_t *got, const uint16_t *want);
#define utf16diffError(msg, got, want) utf16diffErrorFull(__FILE__, __LINE__, msg, got, want)

#ifdef __cplusplus
}
#endif
