// 10 april 2019
#include <inttypes.h>
#include <string.h>
#include "test.h"

// TODO fix up the formatting of testing.c so we can use newlines on the got/want stuff

#define errInvalidOptions "options parameter to uiInit() must be NULL"
#define errAlreadyInitialized "libui already initialized"

testingTestBefore(Init)
{
	uiInitError err;
	int ret;

	ret = uiInit(NULL, NULL);
	if (ret != 0)
		testingTErrorf(t, "uiInit() with NULL error succeeded with return value %d; expected failure", ret);

	memset(&err, 0, sizeof (uiInitError));

	err.Size = 2;
	ret = uiInit(NULL, &err);
	if (ret != 0)
		testingTErrorf(t, "uiInit() with error with invalid size succeeded with return value %d; expected failure", ret);

	err.Size = sizeof (uiInitError);

	ret = uiInit(&err, &err);
	if (ret != 0)
		testingTErrorf(t, "uiInit() with non-NULL options succeeded with return value %d; expected failure", err);
	if (strcmp(err.Message, errInvalidOptions) != 0)
		testingTErrorf(t, "uiInit() with non-NULL options returned bad error message: got %s, want %s", err.Message, errInvalidOptions);

	memset(&err, 0, sizeof (uiInitError));
	err.Size = sizeof (uiInitError);
	ret = uiInit(NULL, &err);
	if (ret == 0)
		testingTErrorf(t, "uiInit() failed: %s", err.Message);

	memset(&err, 0, sizeof (uiInitError));
	err.Size = sizeof (uiInitError);
	ret = uiInit(NULL, &err);
	if (ret != 0)
		testingTErrorf(t, "uiInit() after a previous successful call succeeded with return value %d; expected failure", ret);
	if (strcmp(err.Message, errAlreadyInitialized) != 0)
		testingTErrorf(t, "uiInit() after a previous successful call returned bad error message: got %s, want %s", err.Message, errAlreadyInitialized);
}

static void queued(void *data)
{
	int *flag = (int *) data;

	*flag = 1;
	uiQuit();
}

testingTest(QueueMain)
{
	int flag = 0;

	uiQueueMain(queued, &flag);
	timeout_uiMain(t, 5 * testingNsecPerSec, 0);
	if (flag != 1)
		testingTErrorf(t, "uiQueueMain didn't set flag properly: got %d, want 1", flag);
}

#define queueOp(name, expr) \
	static void name(void *data) \
	{ \
		uint32_t *flag = (uint32_t *) data; \
		*flag = *flag expr; \
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

testingTest(QueueMain_Sequence)
{
	uint32_t flag;
	int i;

	flag = 7;
	uiQueueMain(sub2, &flag);
	uiQueueMain(div3, &flag);
	uiQueueMain(mul8, &flag);
	uiQueueMain(done, NULL);
	timeout_uiMain(t, 5 * testingNsecPerSec, 0);
	if (flag != orders[0].result) {
		for (i = 1; i < 6; i++)
			if (flag == orders[i].result) {
				testingTErrorf(t, "got %" PRIu32 " (%s), want %" PRIu32 " (%s)", flag, orders[i].order, orders[0].result, orders[0].order);
				break;
			}
		if (i >= 6)
			testingTErrorf(t, "got %" PRIu32 " (unknown order), want %" PRIu32 " (%s)", flag, orders[0].result, orders[0].order);
	}
}

// TODO testingTest(QueueMain_DifferentThread)
// TODO testingTest(QueueMain_DifferentThreadSequence)

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
