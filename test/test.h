// 28 april 2019
#include "../ui.h"
#include "lib/testing.h"
#include "lib/timer.h"

#define diff(t, clause, fmt, got, want) testingTErrorf(t, "%s:\ngot  " fmt "\nwant " fmt, clause, got, want)
#define diff_2str(t, clause, clause2, fmt, got, want) testingTErrorf(t, "%s %s:\ngot  " fmt "\nwant " fmt, clause, clause2, got, want)
#define diff2(t, clause, fmts, got1, got2, want1, want2) testingTErrorf(t, "%s:\ngot  " fmts "\nwant " fmts, clause, got1, got2, want1, want2)
#define diffFatal(t, clause, fmt, got, want) testingTFatalf(t, "%s:\ngot  " fmt "\nwant " fmt, clause, got, want)
#define diff2Fatal(t, clause, fmts, got1, got2, want1, want2) testingTFatalf(t, "%s:\ngot  " fmts "\nwant " fmts, clause, got1, got2, want1, want2)

// main.c
extern void timeoutMain(void *data);
#define timeout_uiMain(t, d) { \
	timerSysError err; \
	int timedOut; \
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
