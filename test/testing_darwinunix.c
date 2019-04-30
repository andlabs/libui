// 28 april 2019
#include <errno.h>
#include <inttypes.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
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
	struct timespec duration;

	// TODO check errors, possibly falling back to usleep, setitimer/pause, or even sleep
	duration.tv_sec = nsec / testingNsecPerSec;
	duration.tv_nsec = nsec % testingNsecPerSec;
	nanosleep(&duration, NULL);
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

	t = testingprivNew(testingThread);
	t->f = f;
	t->data = data;

	// TODO check error
	pthread_create(&(t->thread), NULL, threadThreadProc, t);
	return t;
}

void testingThreadWaitAndFree(testingThread *t)
{
	// TODO check errors
	pthread_join(t->thread, NULL);
	// TODO end check errors
	// TODO do we need to free t->thread somehow?
	testingprivFree(t);
}
