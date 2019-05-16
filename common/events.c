// 15 may 2019
#include <stdlib.h>
#include "ui.h"
#include "uipriv.h"

struct handler {
	int id;
	uiHandlerFunc f;
	void *sender;
	void *data;
	bool blocked;
};

static int handlerCmp(const void *a, const void *b)
{
	const struct handler *ha = (const struct handler *) a;
	const struct handler *hb = (const struct handler *) b;

	// This could be ha->id - hb->id, but let's do it the explicit way to avoid integer overflow/underflow.
	if (ha->id < hb->id)
		return -1;
	if (ha->id > hb->id)
		return 1;
	return 0;
}

static struct handler *handlerFind(struct handler *handlers, size_t len, int id)
{
	struct handler key;

	memset(&key, 0, sizeof (struct handler));
	key.id = id;
	return (struct handler *) bsearch(&key, handlers, len, sizeof (struct handler), handlerCmp);
}

static void handlerSort(struct handler *handlers, size_t len)
{
	qsort(handlers, len, sizeof (struct handler), handlerCmp);
}

struct uiEvent {
	uiEventOptions opts;
	struct handler *handlers;
	size_t len;
	size_t cap;
	bool firing;
};

uiEvent *uiNewEvent(uiEventOptions *options)
{
}

int uiEventAddHandler(uiEvent *e, uiEventHandler handler, void *sender, void *data)
{
}

void uiEventDeleteHandler(uiEvent *e, int id)
{
	struct handler *h;

	if (e == NULL)
		TODO
	if (e->firing)
		TODO
	if (e->len == 0)
		TODO
	h = handlerFind(e->handlers, e->len, id);
	if (h == NULL)
		TODO
	e->len--;
	memmove(h + 1, h, (e->len - (h - e->handlers)) * sizeof (struct handler));
}

void uiEventFire(uiEvent *e, void *sender, void *args)
{
	struct handler *h;
	size_t i;

	if (e == NULL)
		TODO
	if (e->firing)
		TODO
	if (e->opts.Global && sender != NULL)
		TODO
	e->firing = true;
	h = e->handlers;
	for (i = 0; i < e->len; i++) {
		if (h->sender == sender && !h->blocked)
			(*(h->f))(sender, args, h->data);
		h++;
	}
	e->firing = false;
}

bool uiEventHandlerBlocked(const uiEvent *e, int id)
{
}

void uiEventSetHandlerBlocked(uiEvent *e, int id, bool blocked)
{
}
