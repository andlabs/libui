// 28 april 2019
// TODO pin down minimum POSIX versions (depends on what macOS 10.8 conforms to and what GLib/GTK+ require)
// TODO feature test macros for things like monotonic clocks?
// TODO is this needed in this file specifically, or just in testing_unix.c?
#define _POSIX_C_SOURCE 200112L
#include <errno.h>
#include <inttypes.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include "testing.h"
#include "testingpriv.h"

// TODO don't start the timer on any platform until after we call setjmp(); also decide whether to start the timer before or after resuming the thread on Windows

static jmp_buf timeout_ret;

static void onTimeout(int sig)
{
	longjmp(timeout_ret, 1);
}

void testingprivRunWithTimeout(testingT *t, const char *file, long line, int64_t timeout, void (*f)(testingT *t, void *data), void *data, const char *comment, int failNowOnError)
{
	char *timeoutstr;
	void (*prevsig)(int);
	struct itimerval duration, prevDuration;
	int setitimerError = 0;

	timeoutstr = testingNsecString(timeout);
	prevsig = signal(SIGALRM, onTimeout);

	duration.it_interval.tv_sec = 0;
	duration.it_interval.tv_usec = 0;
	duration.it_value.tv_sec = timeout / testingNsecPerSec;
	duration.it_value.tv_usec = (timeout % testingNsecPerSec) / testingNsecPerUsec;
	if (setitimer(ITIMER_REAL, &duration, &prevDuration) != 0) {
		setitimerError = errno;
		testingprivTLogfFull(t, file, line, "error applying %s timeout: %s", comment, strerror(setitimerError));
		testingTFail(t);
		goto out;
	}

	if (setjmp(timeout_ret) == 0) {
		(*f)(t, data);
		failNowOnError = 0;		// we succeeded
	} else {
		testingprivTLogfFull(t, file, line, "%s timeout passed (%s)", comment, timeoutstr);
		testingTFail(t);
	}

out:
	if (setitimerError == 0)
		setitimer(ITIMER_REAL, &prevDuration, NULL);
	signal(SIGALRM, prevsig);
	testingFreeNsecString(timeoutstr);
	if (failNowOnError)
		testingTFailNow(t);
}

void testingSleep(int64_t nsec)
{
	struct timespec duration, remaining;
	void (*prevsig)(int);
	sigset_t set, prevSet;
	struct itimerval setiDuration, prevSetiDuration;
	unsigned sec;

	duration.tv_sec = nsec / testingNsecPerSec;
	duration.tv_nsec = nsec % testingNsecPerSec;
	for (;;) {
		errno = 0;
		if (nanosleep(&duration, &remaining) == 0)
			return;
		if (errno != EINTR)
			break;
		duration = remaining;
	}

	// if we got here, nanosleep() failed outright
	if (sigemptyset(&set) != 0)
		goto fallback;
	if (sigaddset(&set, SIGABRT) != 0)
		goto fallback;
	if (pthread_sigmask(SIG_BLOCK, &set, &prevSet) != 0)
		goto fallback;
	prevsig = signal(SIGALRM, SIG_IGN);
	setiDuration.it_interval.tv_sec = 0;
	setiDuration.it_interval.tv_usec = 0;
	// keep using duration for this in case nanosleep() was interrupted before it failed
	setiDuration.it_value.tv_sec = duration.tv_sec;
	setiDuration.it_value.tv_usec = duration.tv_nsec / testingNsecPerUsec;
	if (setitimer(ITIMER_REAL, &setiDuration, &prevSetiDuration) != 0) {
		pthread_sigmask(SIG_SETMASK, &prevSet, NULL);
		signal(SIGALRM, prevsig);
		goto fallback;
	}
	// TODO can this return an errno other than EINTR?
	sigsuspend(&prevSet);
	setitimer(ITIMER_REAL, &prevSetiDuration, NULL);
	signal(SIGALRM, prevsig);
	return;

fallback:
	// hopefully we never reach this point, because it has the least granularity of all, but there are no errors, so...
	sec = duration.tv_sec;
	if (duration.tv_nsec > 0)
		sec++;
	while (sec > 0)
		sec = sleep(sec);
}

struct testingThread {
	pthread_t thread;
	void (*f)(void *data);
	void *data;
};

static void *threadThreadProc(void *data)
{
	testingThread *t = (testingThread *) data;

	(*(t->f))(t->data);
	return NULL;
}

testingThread *testingNewThread(void (*f)(void *data), void *data)
{
	testingThread *t;
	int err;

	t = testingprivNew(testingThread);
	t->f = f;
	t->data = data;

	err = pthread_create(&(t->thread), NULL, threadThreadProc, t);
	if (err != 0)
		testingprivInternalError("error creating thread: %s (%d)", strerror(err), err);
	return t;
}

void testingThreadWaitAndFree(testingThread *t)
{
	int err;

	err = pthread_join(t->thread, NULL);
	if (err != 0)
		testingprivInternalError("error waiting for thread to finish: %s (%d)", strerror(err), err);
	// TODO do we need to free t->thread somehow?
	testingprivFree(t);
}
