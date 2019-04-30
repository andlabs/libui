// 22 april 2019
#include <stdlib.h>
#include <string.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include "testing.h"
#include "testingpriv.h"

struct testingTimer {
	uint64_t start;
	uint64_t end;
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
	t->start = mach_absolute_time();
}

void testingTimerEnd(testingTimer *t)
{
	t->end = mach_absolute_time();
}

int64_t testingTimerNsec(testingTimer *t)
{
	mach_timebase_info_data_t mt;
	uint64_t c;

	mach_timebase_info(&mt);
	c = t->end - t->start;
	c = c * mt.numer / mt.denom;
	return (int64_t) c;
}
