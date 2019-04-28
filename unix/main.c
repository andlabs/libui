// 6 april 2015
#include "uipriv_unix.h"

int uiInit(void *options, uiInitError *err)
{
	GError *gerr = NULL;

	if (!uiprivInitCheckParams(options, err, NULL))
		return 0;

	if (gtk_init_with_args(NULL, NULL, NULL, NULL, NULL, &gerr) == FALSE) {
		strncpy(err->Message, gerr->message, 255);
		g_error_free(gerr);
		return 0;
	}

	uiprivMarkInitialized();
	return 1;
}

void uiMain(void)
{
	gtk_main();
}

void uiQuit(void)
{
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

void uiQueueMain(void (*f)(void *data), void *data)
{
	struct queued *q;

	q = g_new0(struct queued, 1);
	q->f = f;
	q->data = data;
	gdk_threads_add_idle(doqueued, q);
}
