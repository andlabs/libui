// 6 april 2015
#include "uipriv_unix.h"

static pthread_t mainThread;

bool uiprivSysInit(void *options, uiInitError *err)
{
	GError *gerr = NULL;

	if (gtk_init_with_args(NULL, NULL, NULL, NULL, NULL, &gerr) == FALSE) {
		uiprivInitReturnErrorf(err, "%s", gerr->message);
		g_error_free(gerr);
		return false;
	}
	mainThread = pthread_self();
	return true;
}

void uiMain(void)
{
	if (!uiprivCheckInitializedAndThread())
		return;
	gtk_main();
}

void uiQuit(void)
{
	if (!uiprivCheckInitializedAndThread())
		return;
	gtk_main_quit();
}

struct queued {
	void (*f)(void *);
	void *data;
};

static gboolean doqueued(gpointer data)
{
	struct queued *q = (struct queued *) data;

	(*(q->f))(q->data);
	g_free(q);
	return FALSE;
}

void uiprivSysQueueMain(void (*f)(void *data), void *data)
{
	struct queued *q;

	q = g_new0(struct queued, 1);
	q->f = f;
	q->data = data;
	gdk_threads_add_idle(doqueued, q);
}

bool uiprivSysCheckThread(void)
{
	return pthread_equal(pthread_self(), mainThread);
}

void uiprivReportError(const char *prefix, const char *msg, const char *suffix, bool internal)
{
	g_critical("%s: %s. %s", prefix, msg, suffix);
	G_BREAKPOINT();
}
