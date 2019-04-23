// 23 april 2019
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "testing.h"

struct testingTimer {
	clock_t start;
	clock_t end;
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
	t->start = clock();
}

void testingTimerEnd(testingTimer *t)
{
	t->end = clock();
}

int64_t testingTimerNsec(testingTimer *t)
{
	clock_t c;
	clock_t sec;
	int64_t sec64;
	clock_t subsec;
	double subsecf;
	int64_t subsec64;

	c = t->end - t->start;

	sec = c / CLOCKS_PER_SEC;
	sec64 = (int64_t) sec;
	sec64 *= testingTimerNsecPerSec;

	subsec = c % CLOCKS_PER_SEC;
	subsecf = (double) subsec;
	subsecf /= CLOCKS_PER_SEC;
	subsecf *= testingTimerNsecPerSec;
	subsec64 = (int64_t) subsecf;

	return sec64 + subsec64;
}
