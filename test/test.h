// 28 april 2019
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ui.h"
#include "../common/testhooks.h"
#include "lib/testing.h"
#include "lib/thread.h"
#include "lib/timer.h"

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

// init.c
extern testingSet *beforeTests;

// errors.c
extern void checkProgrammerErrorFull(testingT *t, const char *file, long line, const char *name, void (*f)(void *data), void *data, const char *msgWant, bool inThread);
#define checkProgrammerError(t, name, f, data, msgWant) checkProgrammerErrorFull(t, __FILE__, __LINE__, name, f, data, msgWant, false)
#define checkProgrammerErrorInThread(t, name, f, data, msgWant) checkProgrammerErrorFull(t, __FILE__, __LINE__, name, f, data, msgWant, true)
