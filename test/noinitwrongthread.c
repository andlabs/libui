// 28 may 2019
#include "test.h"

struct errorCase {
	bool caught;
	char *msgGot;
	const char *msgWant;
};

static void handleProgrammerError(const char *msg, void *data)
{
	struct errorCase *c = (struct errorCase *) data;
	size_t n;

	c->caught = true;
	if (strcmp(msg, c->msgWant) != 0) {
		n = strlen(msg);
		c->msgGot = testingUtilStrdup(msg);
	}
}

static void deferResetProgrammerError(testingT *t, void *data)
{
	uiprivTestHookReportProgrammerError(NULL, NULL);
}

#define allcallsCase(f, ...) \
	static void beforeInitCase ## f ## Impl(testingT *t, void *data) \
	{ \
		struct errorCase *c = (struct errorCase *) data; \
		memset(c, 0, sizeof (struct errorCase)); \
		c->msgWant = "attempt to call " #f "() before uiInit()"; \
		uiprivTestHookReportProgrammerError(handleProgrammerError, c); \
		testingTDefer(t, deferResetProgrammerError, NULL); \
		f(__VA_ARGS__); \
		if (!c->caught) \
			testingTErrorf(t, "did not throw a programmer error; should have"); \
		if (c->msgGot != NULL) { \
			testingTErrorf(t, "message doesn't contain expected string:" diff("%s"), \
				c->msgGot, c->msgWant); \
			testingUtilFreeStrdup(c->msgGot); \
		} \
	}
allcallsCase(uiQueueMain, NULL, NULL)
#include "allcalls.h"
#undef allcallsCase

static const struct {
	const char *name;
	void (*f)(testingT *, void *);
} beforeInitCases[] = {
#define allcallsCase(f, ...) { #f, beforeInitCase ## f ## Impl },
allcallsCase(uiQueueMain, NULL, NULL)
#include "allcalls.h"
#undef allcallsCase
	{ NULL, NULL },
};

testingTestInSet(beforeTests, FunctionsFailBeforeInit)
{
	struct errorCase c;
	size_t i;

	for (i = 0; beforeInitCases[i].name != NULL; i++)
		testingTRun(t, beforeInitCases[i].name, beforeInitCases[i].f, &c);
}

#define allcallsCase(f, ...) \
	static void wrongThreadCase ## f ## ThreadProc(void *data) \
	{ \
		f(__VA_ARGS__); \
	} \
	static void wrongThreadCase ## f ## Impl(testingT *t, void *data) \
	{ \
		struct errorCase *c = (struct errorCase *) data; \
		threadThread *thread; \
		threadSysError err; \
		memset(c, 0, sizeof (struct errorCase)); \
		c->msgWant = "attempt to call " #f "() on a thread other than the GUI thread"; \
		uiprivTestHookReportProgrammerError(handleProgrammerError, c); \
		testingTDefer(t, deferResetProgrammerError, NULL); \
		err = threadNewThread(wrongThreadCase ## f ## ThreadProc, NULL, &thread); \
		if (err != 0) \
			testingTFatalf(t, "error creating thread: " threadSysErrorFmt, threadSysErrorFmtArg(err)); \
		err = threadThreadWaitAndFree(thread); \
		if (err != 0) \
			testingTFatalf(t, "error waiting for thread to finish: " threadSysErrorFmt, threadSysErrorFmtArg(err)); \
		if (!c->caught) \
			testingTErrorf(t, "did not throw a programmer error; should have"); \
		if (c->msgGot != NULL) { \
			testingTErrorf(t, "message doesn't contain expected string:" diff("%s"), \
				c->msgGot, c->msgWant); \
			testingUtilFreeStrdup(c->msgGot); \
		} \
	}
#include "allcalls.h"
#undef allcallsCase

static const struct {
	const char *name;
	void (*f)(testingT *, void *);
} wrongThreadCases[] = {
#define allcallsCase(f, ...) { #f, wrongThreadCase ## f ## Impl },
#include "allcalls.h"
#undef allcallsCase
	{ NULL, NULL },
};

testingTest(FunctionsFailOnWrongThread)
{
	struct errorCase c;
	size_t i;

	for (i = 0; wrongThreadCases[i].name != NULL; i++)
		testingTRun(t, wrongThreadCases[i].name, wrongThreadCases[i].f, &c);
}
