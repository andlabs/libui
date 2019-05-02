// 23 april 2019
#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "testing.h"
#include "testingpriv.h"

struct testingTimer {
	struct timespec start;
	struct timespec end;
};

testingTimer *testingNewTimer(void)
{
	return testingprivNew(testingTimer);
}

void testingFreeTimer(testingTimer *t)
{
	testingprivFree(t);
}

void testingTimerStart(testingTimer *t)
{
	// TODO check errors
	clock_gettime(CLOCK_MONOTONIC, &(t->start));
}

void testingTimerEnd(testingTimer *t)
{
	// TODO check errors
	clock_gettime(CLOCK_MONOTONIC, &(t->end));
}

// TODO replace with proper subtraction code
int64_t testingTimerNsec(testingTimer *t)
{
	int64_t nstart, nend;

	nstart = ((int64_t) (t->start.tv_sec)) * testingNsecPerSec;
	nstart += t->start.tv_nsec;
	nend = ((int64_t) (t->end.tv_sec)) * testingNsecPerSec;
	nend += t->end.tv_nsec;
	return nend - nstart;
}
