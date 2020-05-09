// 10 april 2019
#include "test.h"
#include "thread.h"
#include "../common/testhooks.h"

// TODO test the number of calls to queued functions made

TestNoInit(InitFailure)
{
	uiInitError err;
	void *ctx;

	ctx = beginCheckProgrammerError(NULL);
	uiprivTestHookSetInitShouldFailArtificially(true);
	memset(&err, 0, sizeof (uiInitError));
	err.Size = sizeof (uiInitError);
	if (uiInit(NULL, &err))
		TestErrorf("uiInit() succeeded; expected failure");
	else if (strcmp(err.Message, "general failure") != 0)
		TestErrorf("uiInit() failed with wrong message:" diff("%s"),
			err.Message, "general failure");
	endCheckProgrammerError(ctx);
}

TestNoInit(InitWithNonNullOptionsIsProgrammerError)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiInit(): invalid uiInitOptions passed");
	if (uiInit(ctx, NULL))
		TestErrorf("uiInit() with non-NULL options succeeded; expected failure");
	endCheckProgrammerError(ctx);
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

// TODO TestNoInit(InitCorrectlyAfterFailureToInitailize)
// TODO TestNoInit(InitIncorrectlyAfterFailureToInitialize)

TestNoInit(InitCorrectlyAfterIncorrectInitialization)
{
	uiInitError err;
	void *ctx;

	ctx = beginCheckProgrammerError("uiInit(): invalid uiInitOptions passed");
	if (uiInit(ctx, NULL))
		TestErrorf("uiInit() with non-NULL options succeeded; expected failure");
	endCheckProgrammerError(ctx);
	ctx = beginCheckProgrammerError("uiInit(): attempt to call more than once");
	memset(&err, 0, sizeof (uiInitError));
	err.Size = sizeof (uiInitError);
	if (uiInit(NULL, &err))
		TestFatalf("uiInit() after a previous successful call succeeded; expected failure");
	endCheckProgrammerError(ctx);
}

TestNoInit(InitIncorrectlyAfterIncorrectInitialization)
{
	void *ctx;

	ctx = beginCheckProgrammerError("uiInit(): invalid uiInitOptions passed");
	if (uiInit(ctx, NULL))
		TestErrorf("uiInit() with non-NULL options succeeded; expected failure");
	endCheckProgrammerError(ctx);
	ctx = beginCheckProgrammerError("uiInit(): attempt to call more than once");
	if (uiInit(NULL, NULL))
		TestFatalf("bad uiInit() after a previous successful call succeeded; expected failure");
	endCheckProgrammerError(ctx);
}

// TODOTODOTODOTODOTODOTODOTODOTODOTODOTODO

struct testParams {
	uint32_t flag;
	threadSysError err;
};

/*
TODO if I remove the uiQuit() from this test on Windows, I will occasionally get
=== RUN   TestQueueMain_DifferentThread
    ../test/initmain.c:161: uiMain() timed out (5s)
--- FAIL: TestQueueMain_DifferentThread (4.9989539s)
TODO see if this is still relevant
*/
static void queued(void *data)
{
	struct testParams *p = (struct testParams *) data;

	p->flag = 1;
	uiQuit();
}

Test(QueueMain)
{
	struct testParams p;

	memset(&p, 0, sizeof (struct testParams));
	p.flag = 0;
	uiQueueMain(queued, &p);
	uiMain();
	if (p.flag != 1)
		TestErrorf("uiQueueMain() didn't set flag properly:" diff("%d"),
			p.flag, 1);
}

// TODO there has to be a way to do this that absolutely will not possibly go in the wrong order... or produce a false positive
#define queueOp(name, expr) \
	static void name(void *data) \
	{ \
		struct testParams *p = (struct testParams *) data; \
		p->flag = p->flag expr; \
	}
queueOp(sub2, - 2)
queueOp(div3, / 3)
queueOp(mul8, * 8)

static void done(void *data)
{
	uiQuit();
}

static const struct {
	uint32_t result;
	const char *order;
} orders[] = {
	{ 8, "sub2 -> div3 -> mul8" },			// the one we want
	{ 13, "sub2 -> mul8 -> div3" },
	{ 0, "div3 -> sub2 -> mul8" },
	{ 14, "div3 -> mul8 -> sub2" },
	{ 18, "mul8 -> sub2 -> div3" },
	{ 16, "mul8 -> div3 -> sub2" },
};

static void queueOrder(struct testParams *p)
{
	p->flag = 7;
	uiQueueMain(sub2, p);
	uiQueueMain(div3, p);
	uiQueueMain(mul8, p);
	uiQueueMain(done, NULL);
}

static void checkOrderFull(const char *file, long line, uint32_t flag)
{
	int i;

	if (flag == orders[0].result)
		return;
	for (i = 1; i < 6; i++)
		if (flag == orders[i].result) {
			TestErrorfFull(file, line, "wrong order:" diff("%" PRIu32 " (%s)"),
				flag, orders[i].order,
				orders[0].result, orders[0].order);
			return;
		}
	TestErrorfFull(file, line, "wrong result:" diff("%" PRIu32 " (%s)"),
		flag, "unknown order",
		orders[0].result, orders[0].order);
}

#define checkOrder(flag) checkOrderFull(__FILE__, __LINE__, flag)

Test(QueueMain_Sequence)
{
	struct testParams p;

	memset(&p, 0, sizeof (struct testParams));
	queueOrder(&p);
	uiMain();
	checkOrder(p.flag);
}

// TODO make a version of these where the thread is started by a queued function

static void queueThread(void *data)
{
	struct testParams *p = (struct testParams *) data;

	p->err = threadSleep(1250 * threadMillisecond);
	uiQueueMain(queued, p);
}

Test(QueueMain_DifferentThread)
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

Test(QueueMain_DifferentThreadSequence)
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
