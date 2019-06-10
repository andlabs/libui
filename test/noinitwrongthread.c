// 28 may 2019
#include "test.h"

struct errorCase {
	void (*f)(void *data);
	void *data;
	bool caught;
	char *msgGot;
	const char *msgWant;
};

static void handleProgrammerError(const char *msg, void *data)
{
	struct errorCase *c = (struct errorCase *) data;

	c->caught = true;
	if (strcmp(msg, c->msgWant) != 0)
		c->msgGot = testingUtilStrdup(msg);
}

static void deferResetProgrammerError(testingT *t, void *data)
{
	uiprivTestHookReportProgrammerError(NULL, NULL);
}

static void doCase(testingT *t, void *data)
{
	struct errorCase *c = (struct errorCase *) data;

	c->caught = false;
	c->msgGot = NULL;
	uiprivTestHookReportProgrammerError(handleProgrammerError, c);
	testingTDefer(t, deferResetProgrammerError, NULL);
	(*(c->f))(c->data);
	if (!c->caught)
		testingTErrorf(t, "did not throw a programmer error; should have");
	if (c->msgGot != NULL) {
		testingTErrorf(t, "message doesn't match expected string:" diff("%s"),
			c->msgGot, c->msgWant);
		testingUtilFreeStrdup(c->msgGot);
	}
}

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
	"attempt to call " #f "() on a thread other than the GUI thread", \
},
	{ "uiQueueMain", doCaseuiQueueMain, "attempt to call uiQueueMain() before uiInit()", NULL },
#include "allcalls.h"
#undef allcallsCase
	{ NULL, NULL, NULL, NULL },
};

testingTestInSet(beforeTests, FunctionsFailBeforeInit)
{
	struct errorCase c;
	size_t i;

	memset(&c, 0, sizeof (struct errorCase));
	for (i = 0; allCases[i].name != NULL; i++) {
		c.f = allCases[i].f;
		c.msgWant = allCases[i].beforeInitWant;
		if (c.msgWant == NULL)
			continue;
		testingTRun(t, allCases[i].name, doCase, &c);
	}
}

struct runInThreadParams {
	testingT *t;
	void (*f)(void *data);
};

static void runInThreadThreadProc(void *data)
{
	struct runInThreadParams *p = (struct runInThreadParams *) data;

	(*(p->f))(NULL);
}

static void runInThread(void *data)
{
	struct runInThreadParams *p = (struct runInThreadParams *) data;
	threadThread *thread;
	threadSysError err;

	err = threadNewThread(runInThreadThreadProc, p, &thread);
	if (err != 0)
		testingTFatalf(p->t, "error creating thread: " threadSysErrorFmt, threadSysErrorFmtArg(err));
	err = threadThreadWaitAndFree(thread);
	if (err != 0)
		testingTFatalf(p->t, "error waiting for thread to finish: " threadSysErrorFmt, threadSysErrorFmtArg(err));
}

static void doCaseInThread(testingT *t, void *data)
{
	struct errorCase *c = (struct errorCase *) data;
	struct runInThreadParams *p;

	p = (struct runInThreadParams *) (c->data);
	p->t = t;
	doCase(t, c);
}

testingTest(FunctionsFailOnWrongThread)
{
	struct errorCase c;
	struct runInThreadParams p;
	size_t i;

	memset(&c, 0, sizeof (struct errorCase));
	c.f = runInThread;
	memset(&p, 0, sizeof (struct runInThreadParams));
	c.data = &p;
	for (i = 0; allCases[i].name != NULL; i++) {
		p.f = allCases[i].f;
		c.msgWant = allCases[i].wrongThreadWant;
		if (c.msgWant == NULL)
			continue;
		testingTRun(t, allCases[i].name, doCaseInThread, &c);
	}
}
