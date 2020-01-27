// 28 may 2019
#include "test.h"
#include "thread.h"
#include "../common/testhooks.h"

// Do not put any test cases in this file; they will not be run.

struct checkProgrammerErrorParams {
	const char *file;
	long line;
	bool inThread;
	bool caught;
	void (*f)(void);
	char *msgGot;
	const char *msgWant;
};

static char *ourStrdup(struct checkProgrammerErrorParams *p, const char *s)
{
	char *t;
	size_t n;

	n = (strlen(s) + 1) * sizeof (char);
	t = (char *) malloc(n);
	if (t == NULL)
		TestFatalfFull(p->file, p->line, "memory exhausted in ourStrdup() copying %s", s);
	memcpy(t, s, n);
	return t;
}

static void handleProgrammerError(const char *msg, void *data)
{
	struct checkProgrammerErrorParams *p = (struct checkProgrammerErrorParams *) data;

	p->caught = true;
	if (strcmp(msg, p->msgWant) != 0)
		p->msgGot = ourStrdup(p, msg);
}

static void checkProgrammerErrorThreadProc(void *data)
{
	struct checkProgrammerErrorParams *p = (struct checkProgrammerErrorParams *) data;

	(*(p->f))();
}

static void checkProgrammerErrorSubtestImpl(struct checkProgrammerErrorParams *p)
{
	if (p->inThread) {
		threadThread *thread;
		threadSysError err;

		err = threadNewThread(checkProgrammerErrorThreadProc, p, &thread);
		if (err != 0)
			// TODO these should only fatal out of the subtest
			TestFatalfFull(p->file, p->line, "error creating thread: " threadSysErrorFmt, threadSysErrorFmtArg(err));
		err = threadThreadWaitAndFree(thread);
		if (err != 0)
			TestFatalfFull(p->file, p->line, "error waiting for thread to finish: " threadSysErrorFmt, threadSysErrorFmtArg(err));
	} else
		(*(p->f))();
	if (!p->caught)
		TestErrorfFull(p->file, p->line, "did not throw a programmer error; should have");
	if (p->msgGot != NULL) {
		TestErrorfFull(p->file, p->line, "message doesn't match expected string:" diff("%s"),
			p->msgGot, p->msgWant);
		free(p->msgGot);
	}
}

void checkProgrammerErrorsFull(const char *file, long line, const struct checkErrorCase *cases, bool inThread)
{
	const struct checkErrorCase *c;
	struct checkProgrammerErrorParams p;

	memset(&p, 0, sizeof (struct checkProgrammerErrorParams));
	p.file = file;
	p.line = line;
	p.inThread = inThread;
	uiprivTestHookReportProgrammerError(handleProgrammerError, &p);
	for (c = cases; c->name != NULL; c++) {
		p.caught = false;
		p.f = c->f;
		p.msgGot = NULL;
		p.msgWant = c->msgWant;
		// TODO this should be a proper subtest, but we don't have that facility with meson yet
		checkProgrammerErrorSubtestImpl(&p);
	}
	uiprivTestHookReportProgrammerError(NULL, NULL);
}
