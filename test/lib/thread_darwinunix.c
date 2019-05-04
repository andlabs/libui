// 28 april 2019
// TODO pin down minimum POSIX versions (depends on what macOS 10.8 conforms to and what GLib/GTK+ require)
#define _POSIX_C_SOURCE 200112L
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include "thread.h"

struct threadThread {
	pthread_t thread;
	void (*f)(void *data);
	void *data;
};

static void *threadThreadProc(void *data)
{
	threadThread *t = (threadThread *) data;

	(*(t->f))(t->data);
	return NULL;
}

threadSysError threadNewThread(void (*f)(void *data), void *data, threadThread **t)
{
	threadThread *tout;
	int err;

	*t = NULL;

	errno = 0;
	tout = (threadThread *) malloc(sizeof (threadThread));
	if (tout == NULL) {
		err = errno;
		if (err == 0)
			err = ENOMEM;
		return (threadSysError) err;
	}
	tout->f = f;
	tout->data = data;

	err = pthread_create(&(tout->thread), NULL, threadThreadProc, tout);
	if (err != 0) {
		free(tout);
		return (threadSysError) err;
	}

	*t = tout;
	return 0;
}

threadSysError threadThreadWaitAndFree(threadThread *t)
{
	int err;

	err = pthread_join(t->thread, NULL);
	if (err != 0)
		return (threadSysError) err;
	// TODO do we have to release t->thread somehow?
	free(t);
	return 0;
}
