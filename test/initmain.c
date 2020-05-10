// 10 april 2019
#include "test.h"
#include "thread.h"
#include "../common/testhooks.h"

// TODO test the number of calls to queued functions made

static void testImplInitFailureFull(const char *file, long line)
{
	uiInitError err;
	void *ctx;

	ctx = beginCheckProgrammerError(NULL);
	uiprivTestHookSetInitShouldFailArtificially(true);
	memset(&err, 0, sizeof (uiInitError));
	err.Size = sizeof (uiInitError);
	if (uiInit(NULL, &err))
		TestErrorfFull(file, line, "uiInit() succeeded; expected failure");
	else if (strcmp(err.Message, "general failure") != 0)
		TestErrorfFull(file, line, "uiInit() failed with wrong message:" diff("%s"),
			err.Message, "general failure");
	endCheckProgrammerError(ctx);
}

#define testImplInitFailure() testImplInitFailureFull(__FILE__, __LINE__)

TestNoInit(InitFailure)
{
	testImplInitFailure();
}

static void testImplNonNullOptionsFull(const char *file, long line)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiInit(): invalid uiInitOptions passed");
	if (uiInit(ctx, NULL))
		TestErrorfFull(file, line, "uiInit() with non-NULL options succeeded; expected failure");
	endCheckProgrammerError(ctx);
}

#define testImplNonNullOptions() testImplNonNullOptionsFull(__FILE__, __LINE__)

TestNoInit(InitWithNonNullOptionsIsProgrammerError)
{
	testImplNonNullOptions();
}

TestNoInit(InitWithNullErrorIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiInit(): invalid null pointer for uiInitError");
	if (uiInit(NULL, NULL))
		TestErrorf("uiInit() with NULL error succeeded; expected failure");
	endCheckProgrammerError(ctx);
}

TestNoInit(InitWithWrongErrorSizeIsProgrammerError)
{
	uiInitError err;
	void *ctx;

	ctx = beginCheckProgrammerError("uiInit(): wrong size 2 for uiInitError");
	memset(&err, 0, sizeof (uiInitError));
	err.Size = 2;
	if (uiInit(NULL, &err))
		TestErrorf("uiInit() with error with invalid size succeeded; expected failure");
	endCheckProgrammerError(ctx);
}

Test(InitCorrectlyAfterInitializedSuccessfully)
{
	uiInitError err;
	void *ctx;

	ctx = beginCheckProgrammerError("uiInit(): attempt to call more than once");
	memset(&err, 0, sizeof (uiInitError));
	err.Size = sizeof (uiInitError);
	if (uiInit(NULL, &err))
		TestFatalf("uiInit() after a previous successful call succeeded; expected failure");
	endCheckProgrammerError(ctx);
}

Test(InitIncorrectlyAfterInitializedSuccessfully)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiInit(): attempt to call more than once");
	if (uiInit(NULL, NULL))
		TestFatalf("bad uiInit() after a previous successful call succeeded; expected failure");
	endCheckProgrammerError(ctx);
}

TestNoInit(InitCorrectlyAfterFailureToInitialize)
{
	uiInitError err;
	void *ctx;

	testImplInitFailure();
	ctx = beginCheckProgrammerError("uiInit(): attempt to call more than once");
	memset(&err, 0, sizeof (uiInitError));
	err.Size = sizeof (uiInitError);
	if (uiInit(NULL, &err))
		TestFatalf("uiInit() after a previous failed call succeeded; expected failure");
	endCheckProgrammerError(ctx);
}

TestNoInit(InitIncorrectlyAfterFailureToInitialize)
{
	void *ctx;

	testImplInitFailure();
	ctx = beginCheckProgrammerError("uiInit(): attempt to call more than once");
	if (uiInit(NULL, NULL))
		TestFatalf("bad uiInit() after a previous failed call succeeded; expected failure");
	endCheckProgrammerError(ctx);
}

TestNoInit(InitCorrectlyAfterIncorrectInitialization)
{
	uiInitError err;
	void *ctx;

	testImplNonNullOptions();
	ctx = beginCheckProgrammerError("uiInit(): attempt to call more than once");
	memset(&err, 0, sizeof (uiInitError));
	err.Size = sizeof (uiInitError);
	if (uiInit(NULL, &err))
		TestFatalf("uiInit() after a previous erroneous call succeeded; expected failure");
	endCheckProgrammerError(ctx);
}

TestNoInit(InitIncorrectlyAfterIncorrectInitialization)
{
	void *ctx;

	testImplNonNullOptions();
	ctx = beginCheckProgrammerError("uiInit(): attempt to call more than once");
	if (uiInit(NULL, NULL))
		TestFatalf("bad uiInit() after a previous erroneous call succeeded; expected failure");
	endCheckProgrammerError(ctx);
}

struct testQueueMainParams {
	unsigned int n;
	threadSysError err;
};

static void queued(void *data)
{
	struct testQueueMainParams *p = (struct testQueueMainParams *) data;

	p->n++;
	if (p->n > 2)
		p->n = 2;
	uiQuit();
}

Test(QueueMain)
{
	struct testQueueMainParams p;

	memset(&p, 0, sizeof (struct testQueueMainParams));
	p.n = 0;
	uiQueueMain(queued, &p);
	uiMain();
	switch (p.n) {
	case 0:
		TestErrorf("uiQueueMain() function was not called");
		break;
	case 1:
		// do nothing; this is the expected case
		break;
	default:
		TestErrorf("uiQueueMain() called more than once");
	}
}

// TODOTODOTODOTODOTODOTODOTODOTODOTODOTODO

struct queuedOrder {
	int calls[4];
	int i;
	unsigned int extraCalls;
};

static void queueOrder(struct queuedOrder *q, int n)
{
	if (q->i < 4) {
		q->calls[q->i] = n;
		q->i++;
		return;
	}
	q->extraCalls++;
	if (q->extraCalls > 9)
		q->extraCalls = 9;
}

static void queueCheckOrderFull(const char *file, long line, struct queuedOrder *got, int wantI, int wantA, int wantB, int wantC, int wantD)
{
	int wantarr[4];
	int i;

	if (got->i != wantI || got->extraCalls != 0) {
		const char *orMore;

		orMore = "";
		if (got->extraCalls >= 9)
			orMore = " or more";
		TestErrorfFull(file, line, "wrong number of queued function calls:" diff("%d%s"),
			got->i + got->extraCalls, orMore,
			wantI, "");
	}
	wantarr[0] = wantA;
	wantarr[1] = wantB;
	wantarr[2] = wantC;
	wantarr[3] = wantD;
	for (i = 0; i < 4; i++)
		if (got->calls[i] != wantarr[i])
			TestErrorfFull(file, line, "wrong value for call %d in sequence:" diff("%d"),
				i + 1, got->calls[i], wantarr[i]);
}

#define queueCheckOrder(got, i, a, b, c, d) queueCheckOrderFull(__FILE__, __LINE__, got, i, a, b, c, d)

struct queueTestParams {
	struct queuedOrder order1;
	struct queuedOrder order2;
};

#define queueStep(name, type, field, n) \
	static void name(void *data) \
	{ \
		type *p = (type *) data; \
		queueOrder(&(p->field), n); \
	}
queueStep(step11, struct queueTestParams, order1, 1)
queueStep(step12, struct queueTestParams, order1, 2)
queueStep(step13, struct queueTestParams, order1, 3)
queueStep(step14, struct queueTestParams, order1, 4)
queueStep(step21, struct queueTestParams, order2, 1)
queueStep(step22, struct queueTestParams, order2, 2)
queueStep(step23, struct queueTestParams, order2, 3)
queueStep(step24, struct queueTestParams, order2, 4)

static void done(void *data)
{
	uiQuit();
}

static void queueOrder1(struct queueTestParams *p)
{
	uiQueueMain(step11, p);
	uiQueueMain(step12, p);
	uiQueueMain(step13, p);
	uiQueueMain(step14, p);
	uiQueueMain(done, NULL);
}

Test(QueueMain_Sequence)
{
	struct queueTestParams p;

	memset(&p, 0, sizeof (struct queueTestParams));
	queueOrder1(&p);
	uiMain();
	queueCheckOrder(&(p.order1), 4, 1, 2, 3, 4);
}

#if 0

// TODO make a version of these where the thread is started by a queued function

static void queueThread(void *data)
{
	struct testParams *p = (struct testParams *) data;

	p->err = threadSleep(1250 * threadMillisecond);
	uiQueueMain(queued, p);
}

TODOTest(QueueMain_DifferentThread)
{
	threadThread *thread;
	threadSysError err;
	struct testParams p;

	memset(&p, 0, sizeof (struct testParams));
	p.flag = 0;
	err = threadNewThread(queueThread, &p, &thread);
	if (err != 0)
		TestFatalf("error creating thread: " threadSysErrorFmt, threadSysErrorFmtArg(err));
	uiMain();
	err = threadThreadWaitAndFree(thread);
	if (err != 0)
		TestFatalf("error waiting for thread to finish: " threadSysErrorFmt, threadSysErrorFmtArg(err));
	if (p.err != 0)
		TestErrorf("error sleeping in thread to ensure a high likelihood the uiQueueMain() is run after uiMain() starts: " threadSysErrorFmt, threadSysErrorFmtArg(p.err));
	if (p.flag != 1)
		TestErrorf("uiQueueMain() didn't set flag properly:" diff("%d"),
			p.flag, 1);
}

static void queueOrderThread(void *data)
{
	struct testParams *p = (struct testParams *) data;

	p->err = threadSleep(1250 * threadMillisecond);
	queueOrder(p);
}

TODOTest(QueueMain_DifferentThreadSequence)
{
	threadThread *thread;
	threadSysError err;
	struct testParams p;

	memset(&p, 0, sizeof (struct testParams));
	p.flag = 1;		// make sure it's initialized just in case
	err = threadNewThread(queueOrderThread, &p, &thread);
	if (err != 0)
		TestFatalf("error creating thread: " threadSysErrorFmt, threadSysErrorFmtArg(err));
	uiMain();
	err = threadThreadWaitAndFree(thread);
	if (err != 0)
		TestFatalf("error waiting for thread to finish: " threadSysErrorFmt, threadSysErrorFmtArg(err));
	if (p.err != 0)
		TestErrorf("error sleeping in thread to ensure a high likelihood the uiQueueMain() is run after uiMain() starts: " threadSysErrorFmt, threadSysErrorFmtArg(p.err));
	checkOrder(p.flag);
}

#if 0
static void timer(void *data)
{
	int *n = (int *) data;

	// TODO return stop if n == 5, continue otherwise
	*n++;
}

testingTest(Timer)
{
}
#endif

#endif
