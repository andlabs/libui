// 28 may 2019
#include "test.h"

struct checkProgrammerErrorParams {
	const char *file;
	long line;
	bool inThread;
	bool caught;
	void (*f)(void);
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

	(*(p->f))();
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
		(*(p->f))();
	if (!p->caught)
		testingTErrorfFull(t, p->file, p->line, "did not throw a programmer error; should have");
	if (p->msgGot != NULL) {
		testingTErrorfFull(t, p->file, p->line, "message doesn't match expected string:" diff("%s"),
			p->msgGot, p->msgWant);
		testingUtilFreeStrdup(p->msgGot);
	}
}

void checkProgrammerErrorsFull(testingT *t, const char *file, long line, const struct checkErrorCase *cases, bool inThread)
{
	const struct checkErrorCase *c;
	struct checkProgrammerErrorParams p;

	memset(&p, 0, sizeof (struct checkProgrammerErrorParams));
	p.file = file;
	p.line = line;
	p.inThread = inThread;
	for (c = cases; c->name != NULL; c++) {
		p.caught = false;
		p.f = c->f;
		p.msgGot = NULL;
		p.msgWant = c->msgWant;
		testingTRun(t, c->name, checkProgrammerErrorSubtestImpl, &p);
	}
}
