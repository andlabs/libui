// 15 may 2019
#include "uipriv.h"

struct handler {
	int id;
	uiEventHandler f;
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

struct uiEvent {
	uiEventOptions opts;
	bool internal;
	uiprivArray handlers;
	uiprivArray unusedIDs;
	bool firing;
};

uiEvent *uiNewEvent(const uiEventOptions *options)
{
	uiEvent *e;

	if (!uiprivCheckInitializedAndThread())
		return NULL;
	if (options == NULL) {
		uiprivProgrammerErrorNullPointer("uiEventOptions", uiprivFunc);
		return NULL;
	}
	if (options->Size != sizeof (uiEventOptions)) {
		uiprivProgrammerErrorWrongStructSize(options->Size, "uiEventOptions");
		return NULL;
	}
	e = (uiEvent *) uiprivAlloc(sizeof (uiEvent), "uiEvent");
	e->opts = *options;
	uiprivArrayInit(e->handlers, struct handler, 32, "uiEvent handlers");
	uiprivArrayInit(e->unusedIDs, int, 32, "uiEvent handler unused IDs");
	return e;
}

#define checkEventNonnull(e, ret) if ((e) == NULL) { \
	uiprivProgrammerErrorNullPointer("uiEvent", uiprivFunc); \
	return ret; \
}
#define checkEventNotFiring(e, ret) if ((e)->firing) { \
	uiprivProgrammerErrorChangingEventDuringFire(uiprivFunc); \
	return ret; \
}

void uiFreeEvent(uiEvent *e)
{
	if (!uiprivCheckInitializedAndThread())
		return;
	checkEventNonnull(e, /* nothing */);
	if (e->internal) {
		uiprivProgrammerErrorFreeingInternalEvent();
		return;
	}
	checkEventNotFiring(e, /* nothing */);
	if (e->handlers.len != 0) {
		uiprivProgrammerErrorFreeingEventInUse();
		return;
	}

	uiprivArrayFree(e->unusedIDs);
	uiprivArrayFree(e->handlers);
	uiprivFree(e);
}

static bool checkEventSender(const uiEvent *e, void *sender, const char *func)
{
	if (e->opts.Global && sender != NULL) {
		uiprivProgrammerErrorBadSenderForEvent("non-NULL", "global", func);
		return false;
	}
	if (!e->opts.Global && sender == NULL) {
		uiprivProgrammerErrorBadSenderForEvent("NULL", "non-global", func);
		return false;
	}
	return true;
}

int uiEventAddHandler(uiEvent *e, uiEventHandler handler, void *sender, void *data)
{
	struct handler *h;
	int id;

	if (!uiprivCheckInitializedAndThread())
		return 0;
	checkEventNonnull(e, 0);
	checkEventNotFiring(e, 0);
	if (handler == NULL) {
		uiprivProgrammerErrorNullPointer("uiEventHandler", uiprivFunc);
		return 0;
	}
	if (!checkEventSender(e, sender, uiprivFunc))
		return 0;

	id = 0;
	if (e->unusedIDs.len > 0) {
		int *p;

		p = uiprivArrayAt(e->unusedIDs, int, e->unusedIDs.len - 1);
		id = *p;
		uiprivArrayDeleteItem(&(e->unusedIDs), p, 1);
	} else if (e->handlers.len != 0)
		id = uiprivArrayAt(e->handlers, struct handler, e->handlers.len - 1)->id + 1;

	h = (struct handler *) uiprivArrayAppend(&(e->handlers), 1);
	h->id = id;
	h->f = handler;
	h->sender = sender;
	h->data = data;
	uiprivArrayQsort(&(e->handlers), handlerCmp);
	return id;
}

static struct handler *findHandler(const uiEvent *e, int id, const char *func)
{
	struct handler key;
	struct handler *ret;

	memset(&key, 0, sizeof (struct handler));
	key.id = id;
	ret = (struct handler *) uiprivArrayBsearch(&(e->handlers), &key, handlerCmp);
	if (ret == NULL)
		uiprivProgrammerErrorIntIDNotFound("uiEvent handler", id, func);
	return ret;
}

void uiEventDeleteHandler(uiEvent *e, int id)
{
	struct handler *h;

	if (!uiprivCheckInitializedAndThread())
		return;
	checkEventNonnull(e, /* nothing */);
	checkEventNotFiring(e, /* nothing */);
	h = findHandler(e, id, uiprivFunc);
	if (h == NULL)
		return;

	uiprivArrayDeleteItem(&(e->handlers), h, 1);
	*((int *) uiprivArrayAppend(&(e->unusedIDs), 1)) = id;
}

void uiEventFire(uiEvent *e, void *sender, void *args)
{
	struct handler *h;
	size_t i;

	if (!uiprivCheckInitializedAndThread())
		return;
	checkEventNonnull(e, /* nothing */);
	if (e->firing) {
		uiprivProgrammerErrorRecursiveEventFire();
		return;
	}
	if (!checkEventSender(e, sender, uiprivFunc))
		return;

	e->firing = true;
	h = (struct handler *) (e->handlers.buf);
	for (i = 0; i < e->handlers.len; i++) {
		if (h->sender == sender && !h->blocked)
			(*(h->f))(sender, args, h->data);
		h++;
	}
	e->firing = false;
}

bool uiEventHandlerBlocked(const uiEvent *e, int id)
{
	struct handler *h;

	if (!uiprivCheckInitializedAndThread())
		return false;
	checkEventNonnull(e, false);
	h = findHandler(e, id, uiprivFunc);
	if (h == NULL)
		return false;
	return h->blocked;
}

void uiEventSetHandlerBlocked(uiEvent *e, int id, bool blocked)
{
	struct handler *h;

	if (!uiprivCheckInitializedAndThread())
		return;
	checkEventNonnull(e, /* nothing */);
	checkEventNotFiring(e, /* nothing */);
	h = findHandler(e, id, uiprivFunc);
	if (h == NULL)
		return;
	h->blocked = blocked;
}
