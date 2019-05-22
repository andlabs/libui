// 18 may 2019
#include "test.h"

struct handler {
	const char *name;
	bool run;
	void *sender;
	void *args;
};

static void handler(void *sender, void *args, void *data)
{
	struct handler *h = (struct handler *) data;

	h->run = true;
	h->sender = sender;
	h->args = args;
}

struct baseParams {
	void (*impl)(testingT *t, void *data);
	bool global;
	void *sender;
	void *args;
};

#define checkHandlerRun(h, bp) \
	if (!h.run) \
		testingTErrorf(t, "%s not run", h.name); \
	else { \
		if (h.sender != bp.sender) \
			diff_2str(t, "incorrect sender seen by", h.name, \
				"%p", h.sender, bp.sender); \
		if (h.args != bp.args) \
			diff_2str(t, "incorrect args seen by", h.name, \
				"%p", h.args, bp.args); \
	}

#define checkHandlerNotRun(h) \
	if (h.run) \
		testingTErrorf(t, "%s run; should not have been", h.name);

static void runArgsSubtests(testingT *t, void *data)
{
	struct baseParams *p = (struct baseParams *) data;

	p->args = &p;
	testingTRun(t, "Args", p->impl, data);
	p->args = NULL;
	testingTRun(t, "NoArgs", p->impl, data);
}

static void runGlobalSubtests(testingT *t, void *data)
{
	struct baseParams *p = (struct baseParams *) data;

	p->global = true;
	p->sender = NULL;
	testingTRun(t, "Global", runArgsSubtests, data);
	p->global = false;
	p->sender = t;
	testingTRun(t, "Nonglobal", runArgsSubtests, data);
}

struct basicEventsSingleHandlerParams {
	struct baseParams bp;
};

static void basicEventsSingleHandlerImpl(testingT *t, void *data)
{
	struct basicEventsSingleHandlerParams *p = (struct basicEventsSingleHandlerParams *) data;
	uiEvent *e;
	uiEventOptions opts;
	struct handler h;

	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = p->bp.global;
	e = uiNewEvent(&opts);

	memset(&h, 0, sizeof (struct handler));
	h.name = "handler";
	uiEventAddHandler(e, handler, p->bp.sender, &h);

	uiEventFire(e, p->bp.sender, p->bp.args);
	checkHandlerRun(h, p->bp);
}

testingTest(BasicEventsSingleHandler)
{
	struct basicEventsSingleHandlerParams p;

	memset(&p, 0, sizeof (struct basicEventsSingleHandlerParams));
	p.bp.impl = basicEventsSingleHandlerImpl;
	runGlobalSubtests(t, &p);
}

struct basicEventsAddDeleteParams {
	struct baseParams bp;
	uiEvent *e;
	struct handler h[6];
	struct handler hbase[6];
	int handler1, handler2, handler3;
	int newHandler1, newHandler2, newHandler3;
};

static void basicEventsAddDeleteEventHandlersImpl(testingT *t, void *data)
{
	struct basicEventsAddDeleteParams *p = (struct basicEventsAddDeleteParams *) data;
	uiEventOptions opts;

	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = p->bp.global;
	p->e = uiNewEvent(&opts);

	memset(p->hbase, 0, 6 * sizeof (struct handler));
	p->hbase[0].name = "handler 1";
	p->hbase[1].name = "handler 2";
	p->hbase[2].name = "handler 3";
	p->hbase[3].name = "new handler 1";
	p->hbase[4].name = "new handler 2";
	p->hbase[5].name = "new handler 3";

	testingTLogf(t, "*** initial handlers");
	p->handler1 = uiEventAddHandler(p->e, handler, p->bp.sender, p->h + 0);
	p->handler2 = uiEventAddHandler(p->e, handler, p->bp.sender, p->h + 1);
	p->handler3 = uiEventAddHandler(p->e, handler, p->bp.sender, p->h + 2);

	memmove(p->h, p->hbase, 6 * sizeof (struct handler));
	uiEventFire(p->e, p->bp.sender, p->bp.args);
	checkHandlerRun(p->h[0], p->bp);
	checkHandlerRun(p->h[1], p->bp);
	checkHandlerRun(p->h[2], p->bp);
	checkHandlerNotRun(p->h[3]);
	checkHandlerNotRun(p->h[4]);
	checkHandlerNotRun(p->h[5]);

	testingTLogf(t, "*** deleting a handler from the middle");
	uiEventDeleteHandler(p->e, p->handler2);

	memmove(p->h, p->hbase, 6 * sizeof (struct handler));
	uiEventFire(p->e, p->bp.sender, p->bp.args);
	checkHandlerRun(p->h[0], p->bp);
	checkHandlerNotRun(p->h[1]);
	checkHandlerRun(p->h[2], p->bp);
	checkHandlerNotRun(p->h[3]);
	checkHandlerNotRun(p->h[4]);
	checkHandlerNotRun(p->h[5]);

	testingTLogf(t, "*** adding handler after deleting a handler from the middle");
	p->newHandler1 = uiEventAddHandler(p->e, handler, p->bp.sender, p->h + 3);

	memmove(p->h, p->hbase, 6 * sizeof (struct handler));
	uiEventFire(p->e, p->bp.sender, p->bp.args);
	checkHandlerRun(p->h[0], p->bp);
	checkHandlerNotRun(p->h[1]);
	checkHandlerRun(p->h[2], p->bp);
	checkHandlerRun(p->h[3], p->bp);
	checkHandlerNotRun(p->h[4]);
	checkHandlerNotRun(p->h[5]);

	testingTLogf(t, "*** deleting first handler added and adding another");
	uiEventDeleteHandler(p->e, p->handler1);
	p->newHandler2 = uiEventAddHandler(p->e, handler, p->bp.sender, p->h + 4);

	memmove(p->h, p->hbase, 6 * sizeof (struct handler));
	uiEventFire(p->e, p->bp.sender, p->bp.args);
	checkHandlerNotRun(p->h[0]);
	checkHandlerNotRun(p->h[1]);
	checkHandlerRun(p->h[2], p->bp);
	checkHandlerRun(p->h[3], p->bp);
	checkHandlerRun(p->h[4], p->bp);
	checkHandlerNotRun(p->h[5]);

	testingTLogf(t, "*** deleting most recently added handler and adding another");
	uiEventDeleteHandler(p->e, p->newHandler2);
	p->newHandler3 = uiEventAddHandler(p->e, handler, p->bp.sender, p->h + 5);

	memmove(p->h, p->hbase, 6 * sizeof (struct handler));
	uiEventFire(p->e, p->bp.sender, p->bp.args);
	checkHandlerNotRun(p->h[0]);
	checkHandlerNotRun(p->h[1]);
	checkHandlerRun(p->h[2], p->bp);
	checkHandlerRun(p->h[3], p->bp);
	checkHandlerNotRun(p->h[4]);
	checkHandlerRun(p->h[5], p->bp);

	testingTLogf(t, "*** deleting all handlers");
	uiEventDeleteHandler(p->e, p->handler3);
	uiEventDeleteHandler(p->e, p->newHandler1);
	uiEventDeleteHandler(p->e, p->newHandler3);

	memmove(p->h, p->hbase, 6 * sizeof (struct handler));
	uiEventFire(p->e, p->bp.sender, p->bp.args);
	checkHandlerNotRun(p->h[0]);
	checkHandlerNotRun(p->h[1]);
	checkHandlerNotRun(p->h[2]);
	checkHandlerNotRun(p->h[3]);
	checkHandlerNotRun(p->h[4]);
	checkHandlerNotRun(p->h[5]);

	testingTLogf(t, "*** adding handler after deleting all handlers");
	uiEventAddHandler(p->e, handler, p->bp.sender, p->h + 0);

	memmove(p->h, p->hbase, 6 * sizeof (struct handler));
	uiEventFire(p->e, p->bp.sender, p->bp.args);
	checkHandlerRun(p->h[0], p->bp);
	checkHandlerNotRun(p->h[1]);
	checkHandlerNotRun(p->h[2]);
	checkHandlerNotRun(p->h[3]);
	checkHandlerNotRun(p->h[4]);
	checkHandlerNotRun(p->h[5]);
}

testingTest(BasicEventsAddDeleteEventHandlers)
{
	struct basicEventsAddDeleteParams p;

	memset(&p, 0, sizeof (struct basicEventsAddDeleteParams));
	p.bp.impl = basicEventsAddDeleteEventHandlersImpl;
	runGlobalSubtests(t, &p);
}

testingTest(EventErrors)
{
	// TODO
}
