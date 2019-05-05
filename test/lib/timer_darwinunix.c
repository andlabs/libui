// 3 may 2019
// TODO pin down minimum POSIX versions (depends on what macOS 10.8 conforms to and what GLib/GTK+ require)
#define _POSIX_C_SOURCE 200112L
#include <errno.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif
#include "timer.h"
#include "timerpriv.h"

static void mustpthread_once(pthread_once_t *once, void (*init)(void))
{
	int err;

	err = pthread_once(once, init);
	if (err != 0) {
		fprintf(stderr, "*** internal error in timerMonotonicNow(): pthread_once() failed: %s (%d)\n", strerror(err), err);
		abort();
	}
}

#ifdef __APPLE__

static uint64_t base;
static mach_timebase_info_data_t mt;
static pthread_once_t baseOnce = PTHREAD_ONCE_INIT;

static void baseInit(void)
{
	kern_return_t err;

	base = mach_absolute_time();
	err = mach_timebase_info(&mt);
	if (err != KERN_SUCCESS) {
		fprintf(stderr, "*** internal error in timerMonotonicNow(): mach_timebase_info() failed: kern_return_t %d\n", err);
		abort();
	}
}

timerTime timerMonotonicNow(void)
{
	uint64_t t;
	timerprivInt128 quot;

	mustpthread_once(&baseOnce, baseInit);
	t = mach_absolute_time() - base;
	timerprivMulDivUint64(t, mt.numer, mt.denom, &quot);
	// on overflow, return the maximum possible timerTime; this is inspired by what Go does
	if (quot.high == 0)
		if (quot.low <= ((uint64_t) timerTimeMax))
			return (timerTime) (quot.low);
	return timerTimeMax;
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
	timerTime ret;

	mustpthread_once(&baseOnce, baseInit);
	mustclock_gettime(CLOCK_MONOTONIC, &ts);
	ts.tv_sec -= base.tv_sec;
	ts.tv_nsec -= base.tv_nsec;
	if (ts.tv_nsec < 0) {		// this is safe because POSIX requires this to be of type long
		ts.tv_sec--;
		ts.tv_nsec += timerSecond;
	}
	ret = ((timerTime) (ts.tv_sec)) * timerSecond;
	ret += (timerTime) (ts.tv_nsec);
	return ret;
}

#endif

timerDuration timerTimeSub(timerTime end, timerTime start)
{
	return end - start;
}

struct timeoutParams {
	jmp_buf retpos;
	struct itimerval prevDuration;
	struct sigaction prevSig;
};

static struct timeoutParams p;

static void onTimeout(int sig, siginfo_t *info, void *ctx)
{
	longjmp(p.retpos, 1);
}

// POSIX doesn't have atomic operations :|
// We could have special OS X-specific code that uses OSAtomic.h, but they have signedness type mismatches :| :|
// There's also the GCC intrinsic atomic operations, but I cannot find a way to portably detect their presence :| :| :|
// And pthread_mutex_lock() and pthread_mutex_unlock() CAN fail :| :| :| :|

static pthread_mutex_t nonReentranceMutex = PTHREAD_MUTEX_INITIALIZER;
static volatile uint32_t nonReentranceInCall = 0;

static void mustpthread_mutex_lock(pthread_mutex_t *mu)
{
	int err;

	err = pthread_mutex_lock(mu);
	if (err != 0) {
		fprintf(stderr, "*** internal error in timerRunWithTimeout(): pthread_mutex_lock() failed: %s (%d)\n", strerror(err), err);
		abort();
	}
}

static void mustpthread_mutex_unlock(pthread_mutex_t *mu)
{
	int err;

	err = pthread_mutex_unlock(mu);
	if (err != 0) {
		fprintf(stderr, "*** internal error in timerRunWithTimeout(): pthread_mutex_unlock() failed: %s (%d)\n", strerror(err), err);
		abort();
	}
}

static int setupNonReentrance(void)
{
	int err;

	mustpthread_mutex_lock(&nonReentranceMutex);
	err = 0;
	if (nonReentranceInCall)
		err = EALREADY;
	else
		nonReentranceInCall = 1;
	mustpthread_mutex_unlock(&nonReentranceMutex);
	return err;
}

static void teardownNonReentrance(void)
{
	mustpthread_mutex_lock(&nonReentranceMutex);
	nonReentranceInCall = 0;
	mustpthread_mutex_unlock(&nonReentranceMutex);
}

timerSysError timerRunWithTimeout(timerDuration d, void (*f)(void *data), void *data, int *timedOut)
{
	sigset_t sigalrm, allsigs;
	sigset_t prevMask;
	volatile int restorePrevMask = 0;
	struct sigaction sig;
	volatile int restoreSignal = 0;
	struct itimerval duration;
	volatile int destroyTimer = 0;
	int err = 0;

	*timedOut = 0;
	err = setupNonReentrance();
	if (err != 0)
		return (timerSysError) err;

	memset(&p, 0, sizeof (struct timeoutParams));

	errno = 0;
	if (sigemptyset(&sigalrm) != 0)
		return (timerSysError) errno;
	errno = 0;
	if (sigaddset(&sigalrm, SIGALRM) != 0)
		return (timerSysError) errno;
	errno = 0;
	if (sigfillset(&allsigs) != 0)
		return (timerSysError) errno;

	err = pthread_sigmask(SIG_BLOCK, &sigalrm, &prevMask);
	if (err != 0)
		return (timerSysError) err;
	restorePrevMask = 1;

	if (setjmp(p.retpos) == 0) {
		sig.sa_mask = allsigs;
		sig.sa_flags = SA_SIGINFO;
		sig.sa_sigaction = onTimeout;
		errno = 0;
		if (sigaction(SIGALRM, &sig, &(p.prevSig)) != 0) {
			err = errno;
			goto out;
		}
		restoreSignal = 1;

		duration.it_interval.tv_sec = 0;
		duration.it_interval.tv_usec = 0;
		duration.it_value.tv_sec = d / timerSecond;
		duration.it_value.tv_usec = (d % timerSecond) / timerMicrosecond;
		errno = 0;
		if (setitimer(ITIMER_REAL, &duration, &(p.prevDuration)) != 0) {
			err = errno;
			goto out;
		}
		destroyTimer = 1;

		// and fire away
		err = pthread_sigmask(SIG_UNBLOCK, &sigalrm, NULL);
		if (err != 0)
			goto out;

		(*f)(data);
	} else
		*timedOut = 1;
	err = 0;

out:
	if (destroyTimer)
		setitimer(ITIMER_REAL, &(p.prevDuration), NULL);
	if (restoreSignal)
		sigaction(SIGALRM, &(p.prevSig), NULL);
	if (restorePrevMask)
		pthread_sigmask(SIG_SETMASK, &prevMask, NULL);
	teardownNonReentrance();
	return (timerSysError) err;
}

timerSysError timerSleep(timerDuration d)
{
	struct timespec duration, remaining;
	int err;

	duration.tv_sec = d / timerSecond;
	duration.tv_nsec = d % timerSecond;
	for (;;) {
		errno = 0;
		if (nanosleep(&duration, &remaining) == 0)
			return 0;
		err = errno;
		if (err != EINTR)
			return (timerSysError) err;
		duration = remaining;
	}
}
