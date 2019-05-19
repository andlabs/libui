// 15 may 2019
#include <stdlib.h>
#include <string.h>
#include "ui.h"
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
	uiprivArray handlers;
	int *unusedIDs;
	size_t unusedIDsLen;
	size_t unusedIDsCap;
	bool firing;
};

uiEvent *uiNewEvent(const uiEventOptions *options)
{
	uiEvent *e;

	if (options == NULL) {
		uiprivProgrammerError(uiprivProgrammerErrorNullPointer, "uiEventOptions", __func__);
		return NULL;
	}
	if (options->Size != sizeof (uiEventOptions)) {
		uiprivProgrammerError(uiprivProgrammerErrorWrongStructSize, options->Size, "uiEventOptions");
		return NULL;
	}
	e = (uiEvent *) uiprivAlloc(sizeof (uiEvent), "uiEvent");
	e->opts = *options;
	uiprivArrayInit(e->handlers, struct handler, 32, "uiEvent handlers");
	return e;
}

#define checkEventNonnull(e, ret) if ((e) == NULL) { \
	uiprivProgrammerError(uiprivProgrammerErrorNullPointer, "uiEvent", __func__); \
	return ret; \
}
#define checkEventNotFiring(e, ret) if ((e)->firing) { \
	uiprivProgrammerError(uiprivProgrammerErrorChangingEventDuringFire, __func__); \
	return ret; \
}

static bool checkEventSender(const uiEvent *e, void *sender, const char *func)
{
	if (e->opts.Global && sender != NULL) {
		uiprivProgrammerError(uiprivProgrammerErrorBadSenderForEvent, "non-NULL", "global", func);
		return false;
	}
	if (!e->opts.Global && sender == NULL) {
		uiprivProgrammerError(uiprivProgrammerErrorBadSenderForEvent, "NULL", "non-global", func);
		return false;
	}
	return true;
}

int uiEventAddHandler(uiEvent *e, uiEventHandler handler, void *sender, void *data)
{
	struct handler *h;
	int id;

	checkEventNonnull(e, 0);
	checkEventNotFiring(e, 0);
	if (handler == NULL) {
		uiprivProgrammerError(uiprivProgrammerErrorNullPointer, "uiEventHandler", __func__);
		return 0;
	}
	if (!checkEventSender(e, sender, __func__))
		return 0;

	id = 0;
	if (e->unusedIDsLen > 0) {
		id = e->unusedIDs[e->unusedIDsLen - 1];
		e->unusedIDsLen--;
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
		uiprivProgrammerError(uiprivProgrammerErrorIntIDNotFound, "uiEvent handler", id, func);
	return ret;
}

void uiEventDeleteHandler(uiEvent *e, int id)
{
	struct handler *h;

	checkEventNonnull(e, /* nothing */);
	checkEventNotFiring(e, /* nothing */);
	h = findHandler(e, id, __func__);
	if (h == NULL)
		return;

	uiprivArrayDeleteItem(&(e->handlers), h, 1);

	if (e->unusedIDsLen >= e->unusedIDsCap) {
		e->unusedIDs = (int *) uiprivRealloc(e->unusedIDs,
			e->unusedIDsCap * sizeof (int),
			(e->unusedIDsCap + 32) * sizeof (int),
			"uiEvent handler unused IDs");
		e->unusedIDsCap += 32;
	}
	e->unusedIDs[e->unusedIDsLen] = id;
	e->unusedIDsLen++;
}

void uiEventFire(uiEvent *e, void *sender, void *args)
{
	struct handler *h;
	size_t i;

	checkEventNonnull(e, /* nothing */);
	if (e->firing) {
		uiprivProgrammerError(uiprivProgrammerErrorRecursiveEventFire);
		return;
	}
	if (!checkEventSender(e, sender, __func__))
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

	checkEventNonnull(e, false);
	h = findHandler(e, id, __func__);
	if (h == NULL)
		return false;
	return h->blocked;
}

void uiEventSetHandlerBlocked(uiEvent *e, int id, bool blocked)
{
	struct handler *h;

	checkEventNonnull(e, /* nothing */);
	checkEventNotFiring(e, /* nothing */);
	h = findHandler(e, id, __func__);
	if (h == NULL)
		return;
	h->blocked = blocked;
}
