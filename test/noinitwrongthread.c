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

#define allcallsCase(f, ...) \
	static void beforeInitCase ## f ## Impl(testingT *t, void *data) \
	{ \
		struct errorCase *c = (struct errorCase *) data; \
		memset(c, 0, sizeof (struct errorCase)); \
		c->msgWant = "attempt to call " #f "() before uiInit()"; \
		uiprivTestHookReportProgrammerError(handleProgrammerError, c); \
		f(__VA_ARGS__); \
		if (!c->caught) \
			testingTErrorf(t, "did not throw a programmer error; should have"); \
		if (c->msgGot != NULL) { \
			testingTErrorf(t, "message doesn't contain expected string:" diff("%s"), \
				c->msgGot, c->msgWant); \
			testingUtilFreeStrdup(c->msgGot); \
		} \
		uiprivTestHookReportProgrammerError(NULL, NULL); \
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

#if 0
TODOTODO

static struct errorCase *runCasesWrongThread(void)
{
	struct errorCase *first = NULL;
	struct errorCase *last = NULL;

#define allcallsMsgSuffix "on a thread other than the GUI thread"
#include "allcalls.h"
#undef allcallsMsgSuffix
	return first;
}

static void wrongThreadThreadProc(void *data)
{
	struct errorCase **pCases = (struct errorCase **) data;

	uiprivTestHookReportProgrammerError(catalogProgrammerError);
	*pCases = runCasesWrongThread();
	uiprivTestHookReportProgrammerError(NULL);
	// do this now in case memory was exhausted and something gets allocated before we return to the main thread
	if (caseError != NULL) {
		freeCases(*pCases);
		*pCases = NULL;
	}
}

testingTest(FunctionsFailOnWrongThread)
{
	struct errorCase *cases;
	threadThread *thread;
	threadSysError err;

	caseError = NULL;
	err = threadNewThread(wrongThreadThreadProc, &cases, &thread);
	if (err != 0)
		testingTFatalf(t, "error creating thread: " threadSysErrorFmt, threadSysErrorFmtArg(err));
	err = threadThreadWaitAndFree(thread);
	if (err != 0)
		testingTFatalf(t, "error waiting for thread to finish: " threadSysErrorFmt, threadSysErrorFmtArg(err));
	if (caseError != NULL) {
		freeCases(cases);
		testingTErrorf(t, "%s running tests", caseError);
		if (caseError != caseErrorMemoryExhausted && caseError != caseErrorEncodingError)
			free(caseError);
		caseError = NULL;
		testingTFailNow(t);
	}
	reportCases(t, cases);
	freeCases(cases);
}

#endif
