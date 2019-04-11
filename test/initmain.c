// 10 april 2019
#include "../ui.h"
#include "testing.h"

testingTestBefore(Init)
{
}

testingTestAfter(Uninit)
{
}

static void queued(void *data)
{
	int *flag = (int *) data;

	*flag = 1;
}

static void timer(void *data)
{
	int *n = (int *) data;

	// TODO return stop if n == 5, continue otherwise
	*n++;
}

testingTest(QueueMain)
{
}

// TODO testingTest(QueueMain_DifferentThread)

testingTest(Timer)
{
}
