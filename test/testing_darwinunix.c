// 28 april 2019
#include <errno.h>
#include <inttypes.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include "testing.h"

// TODO testingTimers after this fails are wrong on GTK+

static jmp_buf timeout_ret;

static void onTimeout(int sig)
{
	longjmp(timeout_ret, 1);
}

void testingprivRunWithTimeout(testingT *t, const char *file, long line, int64_t timeout, void (*f)(testingT *t, void *data), void *data, const char *comment, int failNowOnError)
{
	char *timeoutstr;
	void (*prevsig)(int);
	struct itimerval timer, prevtimer;
	int setitimerError = 0;

	timeoutstr = testingNsecString(timeout);
	prevsig = signal(SIGALRM, onTimeout);

	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	timer.it_value.tv_sec = timeout / testingNsecPerSec;
	timer.it_value.tv_usec = (timeout % testingNsecPerSec) / testingNsecPerUsec;
	if (setitimer(ITIMER_REAL, &timer, &prevtimer) != 0) {
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
		setitimer(ITIMER_REAL, &prevtimer, NULL);
	signal(SIGALRM, prevsig);
	testingFreeNsecString(timeoutstr);
	if (failNowOnError)
		testingTFailNow(t);
}
