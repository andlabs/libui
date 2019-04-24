// 23 april 2019
#define UNICODE
#define _UNICODE
#define STRICT
#define STRICT_TYPED_ITEMIDS
#define WINVER			0x0600
#define _WIN32_WINNT		0x0600
#define _WIN32_WINDOWS	0x0600
#define _WIN32_IE			0x0700
#define NTDDI_VERSION		0x06000000
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "testing.h"

struct testingTimer {
	LARGE_INTEGER start;
	LARGE_INTEGER end;
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
	QueryPerformanceCounter(&(t->start));
}

void testingTimerEnd(testingTimer *t)
{
	QueryPerformanceCounter(&(t->end));
}

int64_t testingTimerNsec(testingTimer *t)
{
	LARGE_INTEGER qpf;
	int64_t c;
	int64_t sec;
	int64_t subsec;
	double subsecf;

	QueryPerformanceFrequency(&qpf);
	c = t->end.QuadPart - t->start.QuadPart;

	sec = c / qpf.QuadPart;
	sec *= testingTimerNsecPerSec;

	subsec = c % qpf.QuadPart;
	subsecf = (double) subsec;
	subsecf /= qpf.QuadPart;
	subsecf *= testingTimerNsecPerSec;
	subsec = (int64_t) subsecf;

	return sec + subsec;
}
