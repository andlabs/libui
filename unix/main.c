// 6 april 2015
#include "uipriv_unix.h"

uiInitOptions options;

const char *uiInit(uiInitOptions *o)
{
	GError *err = NULL;
	const char *msg;

	options = *o;
	if (gtk_init_with_args(NULL, NULL, NULL, NULL, NULL, &err) == FALSE) {
		msg = g_strdup(err->message);
		g_error_free(err);
		return msg;
	}
	initAlloc();
	return NULL;
}

void uiUninit(void)
{
	uninitMenus();
	uninitAlloc();
}

void uiFreeInitError(const char *err)
{
	g_free((gpointer) err);
}

void uiMain(void)
{
	gtk_main();
}

int uiMainStep(int wait)
{
	gboolean block;

	block = FALSE;
	if (wait)
		block = TRUE;
	return gtk_main_iteration_do(block) == FALSE;
}

// gtk_main_quit() may run immediately, or it may wait for other pending events; "it depends" (thanks mclasen in irc.gimp.net/#gtk+)
// PostQuitMessage() on Windows always waits, so we must do so too
// we'll do it by using an idle callback
static gboolean quit(gpointer data)
{
	gtk_main_quit();
	return FALSE;
}

void uiQuit(void)
{
	gdk_threads_add_idle(quit, NULL);
}

struct queued {
	void (*f)(void *);
	void *data;
};

static gboolean doqueued(gpointer data)
{
	struct queued *q = (struct queued *) data;

	(*(q->f))(q->data);
	uiFree(q);
	return FALSE;
}

void uiQueueMain(void (*f)(void *data), void *data)
{
	struct queued *q;

	q = uiNew(struct queued);
	q->f = f;
	q->data = data;
	gdk_threads_add_idle(doqueued, q);
}
