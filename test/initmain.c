// 10 april 2019
#include "../ui.h"
#include "testing.h"

Test(Init)
{
}

Test(Uninit)
{
}

static void queued(void *data)
{
	int *flag = (int *) data;

	*flag = 1;
}

static void timer(void *n)
{
	int *n = (int *) data;

	// TODO return stop if n == 5, continue otherwise
	*n++;
}

Test(QueueMain)
{
}

// TODO Test(QueueMain_DifferentThread)

Test(Timer)
{
}
