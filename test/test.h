// 28 april 2019
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ui.h"
#ifdef libuiOSHeader
#include libuiOSHeader
#endif
#include "../common/testhooks.h"
#include "lib/testing.h"
#include "lib/thread.h"
#include "lib/timer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define diff(fmt) "\ngot  " fmt "\nwant " fmt

// main.c
extern void timeoutMain(void *data);
#define timeout_uiMain(t, d) { \
	timerSysError err; \
	bool timedOut; \
	err = timerRunWithTimeout(d, timeoutMain, NULL, &timedOut); \
	if (err != 0) \
		testingTErrorf(t, "error running uiMain() in timeout: " timerSysErrorFmt, timerSysErrorFmtArg(err)); \
	if (timedOut) { \
		char timeoutstr[timerDurationStringLen]; \
		timerDurationString(d, timeoutstr); \
		testingTErrorf(t, "uiMain() timed out (%s)", timeoutstr); \
	} \
}
extern void deferFree(testingT *t, void *data);
extern void deferEventFree(testingT *t, void *data);

// init.c
extern testingSet *beforeTests;

// errors.c
struct checkErrorCase {
	const char *name;
	void (*f)(void);
	const char *msgWant;
};
extern void checkProgrammerErrorsFull(testingT *t, const char *file, long line, const struct checkErrorCase *cases, bool inThread);
#define checkProgrammerErrors(t, cases) checkProgrammerErrorsFull(t, __FILE__, __LINE__, cases, false)
#define checkProgrammerErrorsInThread(t, cases) checkProgrammerErrorsFull(t, __FILE__, __LINE__, cases, true)

// controls.c
extern const uiControlVtable testVtable;
extern uiControlOSVtable *allocOSVtableFull(testingT *t, const char *file, long line);
#define allocOSVtable(t) allocOSVtableFull(t, __FILE__, __LINE__)

#ifdef __cplusplus
}
#endif
