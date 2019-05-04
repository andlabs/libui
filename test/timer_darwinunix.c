// 3 may 2019
#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_time.h>
#else
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#endif
#include "timer.h"

#ifdef __APPLE__

timerTime timerMonotonicNow(void)
{
	return (timerTime) mach_absolute_time();
}

timerDuration timerTimeSub(timerTime end, timerTime start)
{
	mach_timebase_info_data_t mt;
	timerTime c;
	timerDuration ret;

	mach_timebase_info(&mt);
	c = end - start;
	ret = ((timerDuration) c) * mt.numer / mt.denom;
	return ret;
}

#else

static void mustclock_gettime(clockid_t id, struct timespec *ts)
{
	int err;

	errno = 0;
	if (clock_gettime(id, ts) != 0) {
		err = errno;
		fprintf(stderr, "*** internal error in timerMonotonicNow(): clock_gettime() failed: %s (%d)\n", strerror(err), err);
		abort();
	}
}

static struct timespec base;
static pthread_once_t baseOnce = PTHREAD_ONCE_INIT;

static void baseInit(void)
{
	mustclock_gettime(CLOCK_MONOTONIC, &base);
}

timerTime timerMonotonicNow(void)
{
	struct timespec ts;
	timerTIme ret;
	int err;

	err = pthread_once(&baseOnce, baseInit);
	if (err != 0) {
		fprintf(stderr, "*** internal error in timerMonotonicNow(): pthread_once() failed: %s (%d)\n", strerror(err), err);
		abort();
	}
	mustclock_gettime(CLOCK_MONOTONIC, &ts);
	ts.tv_sec -= base.tv_sec;
	ts.tv_nsec -= base.tv_nsec;
	if (ts.tv_nsec < 0) {		// this is safe because POSIX requires this to be of type long
		ts.tv_sec--;
		ts.tv_nsec += (long) timerSecond;
	}
	ret = ((timerTime) (ts.tv_sec)) * timerSecond;
	ret += (timerTime) (ts.tv_nsec);
	return ret;
}

timerDuration timerTimeSub(timerTime end, timerTime start)
{
	return end - start;
}

#endif
