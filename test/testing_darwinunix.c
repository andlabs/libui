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
