// 23 april 2019
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "testing.h"

struct testingTimer {
	struct timespec start;
	struct timespec end;
};

testingTimer *testingNewTimer(void)
{
	testingTimer *t;

	t = (testingTimer *) malloc(sizeof (testingTimer));
	// TODO handle failure
	memset(t, 0, sizeof (testingTimer));
	return t;
}

void testingFreeTimer(testingTimer *t)
{
	free(t);
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
