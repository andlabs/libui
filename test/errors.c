// 28 may 2019
#include "test.h"
#include "thread.h"
#include "../common/testhooks.h"

// Do not put any test cases in this file; they will not be run.

// Notes on these functions:
// - Try to wrap them as tightly around the specific calls being tested as possible, to avoid accidentally catching something else.
// - I don't know if these are thread-safe yet (TODO potentially make them so so this first part can be made tighter).

struct checkProgrammerErrorParams {
	bool caught;
	char *msgGot;
	const char *msgWant;		// NULL if should not be caught
};

static char *ourStrdup(const char *s)
{
	char *t;
	size_t n;

	n = (strlen(s) + 1) * sizeof (char);
	t = (char *) malloc(n);
	if (t == NULL)
		TestFatalf("memory exhausted in ourStrdup() copying %s", s);
	memcpy(t, s, n);
	return t;
}

static void handleProgrammerError(const char *msg, void *data)
{
	struct checkProgrammerErrorParams *p = (struct checkProgrammerErrorParams *) data;

	p->caught = true;
	if (strcmp(msg, p->msgWant) != 0)
		p->msgGot = ourStrdup(msg);
}

#if 0
// TODO
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
			// TODO these should only fatal out of the subtest (see TODO below)
			TestFatalfFull(p->file, p->line, "error creating thread: " threadSysErrorFmt, threadSysErrorFmtArg(err));
		err = threadThreadWaitAndFree(thread);
		if (err != 0)
			TestFatalfFull(p->file, p->line, "error waiting for thread to finish: " threadSysErrorFmt, threadSysErrorFmtArg(err));
	}
}
#endif

void *beginCheckProgrammerError(const char *want)
{
	struct checkProgrammerErrorParams *p;

	p = (struct checkProgrammerErrorParams *) malloc(sizeof (struct checkProgrammerErrorParams));
	if (p == NULL)
		TestFatalf("memory exhausted allocating beginCheckProgrammerError() context");
	memset(p, 0, sizeof (struct checkProgrammerErrorParams));
	p->msgWant = want;
	uiprivTestHookReportProgrammerError(handleProgrammerError, p);
	return p;
}

static void checkWantProgrammerError(const char *file, long line, struct checkProgrammerErrorParams *p)
{
	if (!p->caught)
		TestErrorfFull(file, line, "did not throw a programmer error; should have");
	if (p->msgGot != NULL) {
		TestErrorfFull(file, line, "message doesn't match expected string:" diff("%s"),
			p->msgGot, p->msgWant);
		free(p->msgGot);
	}
}

static void checkWantNoProgrammerError(const char *file, long line, struct checkProgrammerErrorParams *p)
{
	if (p->caught)
		TestErrorfFull(file, line, "threw a programmer error; should not have");
	if (p->msgGot != NULL) {
		TestErrorfFull(file, line, "got unexpected programmer error message when none was expected: %s", p->msgGot);
		free(p->msgGot);
	}
}

void endCheckProgrammerErrorFull(const char *file, long line, void *context)
{
	struct checkProgrammerErrorParams *p = (struct checkProgrammerErrorParams *) context;

	if (p->msgWant != NULL)
		checkWantProgrammerError(file, line, p);
	else
		checkWantNoProgrammerError(file, line, p);
	free(p);
	uiprivTestHookReportProgrammerError(NULL, NULL);
}
