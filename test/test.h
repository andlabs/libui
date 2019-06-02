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

#define diffx(fmt) "\ngot  " fmt "\nwant " fmt
#define diff(t, clause, fmt, got, want) testingTErrorf(t, "%s:\ngot  " fmt "\nwant " fmt, clause, got, want)
#define diff_2str(t, clause, clause2, fmt, got, want) testingTErrorf(t, "%s %s:\ngot  " fmt "\nwant " fmt, clause, clause2, got, want)

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
struct errorParams {
	testingT *t;
	const char *file;
	long line;
	// TODO this shouldn't have a colon in it but the diff() macros above necessitate it
	const char *exprstr;
	const char *msgWant;
	bool caught;
};
extern struct errorParams errorParams;
extern void catchProgrammerError(const char *prefix, const char *msg, const char *suffix, bool internal);
#define testProgrammerError(tt, expr, mw) { \
	testingTLogf(t, "*** %s", #expr); \
	uiprivTestHookReportProgrammerError(catchProgrammerError); \
	errorParams.t = tt; \
	errorParams.file = __FILE__; \
	errorParams.line = __LINE__; \
	errorParams.exprstr = #expr ":"; \
	errorParams.msgWant = mw; \
	errorParams.caught = false; \
	expr; \
	if (!errorParams.caught) \
		testingTErrorfFull(t, errorParams.file, errorParams.line, "%s did not throw a programmer error; should have", #expr); \
	uiprivTestHookReportProgrammerError(NULL); \
}

// init.c
extern testingSet *beforeTests;
