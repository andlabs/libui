// 10 april 2019
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
	timeout_uiMain(t, 5 * testingTimerNsecPerSec, 0);
	if (flag != 1)
		testingTErrorf(t, "uiQueueMain didn't set flag properly: got %d, want 1", flag);
}

testingTest(QueueMain_Sequence)
{
	// TODO
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
