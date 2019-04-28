// 10 april 2019
#include "test.h"

static void timeoutMain(testingT *t, void *data)
{
	uiMain();
}

void timeout_uiMain(testingT *t, int64_t timeout, int failNowOnError)
{
	testingRunWithTimeout(t, timeout,
		timeoutMain, NULL,
		"uiMain()", failNowOnError);
}

int main(void)
{
	return testingMain();
}
