// 28 may 2019
#include "test.h"

struct checkProgrammerErrorParams {
	const char *file;
	long line;
	void (*f)(void *data);
	void *data;
	bool inThread;
	bool caught;
	char *msgGot;
	const char *msgWant;
};

static void handleProgrammerError(const char *msg, void *data)
{
	struct checkProgrammerErrorParams *p = (struct checkProgrammerErrorParams *) data;

	p->caught = true;
	if (strcmp(msg, p->msgWant) != 0)
		p->msgGot = testingUtilStrdup(msg);
}

static void deferResetProgrammerError(testingT *t, void *data)
{
	uiprivTestHookReportProgrammerError(NULL, NULL);
}

static void checkProgrammerErrorThreadProc(void *data)
{
	struct checkProgrammerErrorParams *p = (struct checkProgrammerErrorParams *) data;

	(*(p->f))(p->data);
}

static void checkProgrammerErrorSubtestImpl(testingT *t, void *data)
{
	struct checkProgrammerErrorParams *p = (struct checkProgrammerErrorParams *) data;

	uiprivTestHookReportProgrammerError(handleProgrammerError, p);
	testingTDefer(t, deferResetProgrammerError, NULL);
	if (p->inThread) {
		threadThread *thread;
		threadSysError err;

		err = threadNewThread(checkProgrammerErrorThreadProc, p, &thread);
		if (err != 0)
			testingTFatalfFull(t, p->file, p->line, "error creating thread: " threadSysErrorFmt, threadSysErrorFmtArg(err));
		err = threadThreadWaitAndFree(thread);
		if (err != 0)
			testingTFatalfFull(t, p->file, p->line, "error waiting for thread to finish: " threadSysErrorFmt, threadSysErrorFmtArg(err));
	} else
		(*(p->f))(p->data);
	if (!p->caught)
		testingTErrorfFull(t, p->file, p->line, "did not throw a programmer error; should have");
	if (p->msgGot != NULL) {
		testingTErrorfFull(t, p->file, p->line, "message doesn't match expected string:" diff("%s"),
			p->msgGot, p->msgWant);
		testingUtilFreeStrdup(p->msgGot);
	}
}

void checkProgrammerErrorFull(testingT *t, const char *file, long line, const char *name, void (*f)(void *data), void *data, const char *msgWant, bool inThread)
{
	struct checkProgrammerErrorParams p;

	memset(&p, 0, sizeof (struct checkProgrammerErrorParams));
	p.file = file;
	p.line = line;
	p.f = f;
	p.data = data;
	p.inThread = inThread;
	p.msgWant = msgWant;
	testingTRun(t, name, checkProgrammerErrorSubtestImpl, &p);
}

#define checkProgrammerError(t, name, f, data, msgWant) checkProgrammerErrorFull(t, __FILE__, __LINE__, name, f, data, msgWant, false)
#define checkProgrammerErrorInThread(t, name, f, data, msgWant) checkProgrammerErrorFull(t, __FILE__, __LINE__, name, f, data, msgWant, true)

#define allcallsCase(f, ...) \
	void doCase ## f(void *data) \
	{ \
		f(__VA_ARGS__); \
	}
allcallsCase(uiQueueMain, NULL, NULL)
#include "allcalls.h"
#undef allcallsCase

static const struct {
	const char *name;
	void (*f)(void *data);
	const char *beforeInitWant;
	const char *wrongThreadWant;
} allCases[] = {
#define allcallsCase(f, ...) { #f, doCase ## f, \
	"attempt to call " #f "() before uiInit()", \
	allcallsThread(#f), \
},
#define allcallsThread(f) NULL
allcallsCase(uiQueueMain, NULL, NULL)
#undef allcallsThread
#define allcallsThread(f) "attempt to call " f "() on a thread other than the GUI thread"
#include "allcalls.h"
#undef allcallsCase
	{ NULL, NULL, NULL, NULL },
};

testingTestInSet(beforeTests, FunctionsFailBeforeInit)
{
	size_t i;

	for (i = 0; allCases[i].name != NULL; i++) {
		if (allCases[i].beforeInitWant == NULL)
			continue;
		checkProgrammerError(t, allCases[i].name, allCases[i].f, NULL, allCases[i].beforeInitWant);
	}
}

testingTest(FunctionsFailOnWrongThread)
{
	size_t i;

	for (i = 0; allCases[i].name != NULL; i++) {
		if (allCases[i].wrongThreadWant == NULL)
			continue;
		checkProgrammerErrorInThread(t, allCases[i].name, allCases[i].f, NULL, allCases[i].wrongThreadWant);
	}
}
