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

#define checkHandlerRun(h, whichSender, whichArgs) \
	if (!h.run) \
		testingTErrorf(t, "%s not run", h.name); \
	else { \
		if (h.sender != whichSender) \
			diff_2str(t, "incorrect sender seen by", h.name, \
				"%p", h.sender, whichSender); \
		if (h.args != whichArgs) \
			diff_2str(t, "incorrect args seen by", h.name, \
				"%p", h.args, whichArgs); \
	}

#define checkHandlerNotRun(h) \
	if (h.run) \
		testingTErrorf(t, "%s run; should not have been", h.name);

struct basicEventsSingleHandlerParams {
	bool global;
	void *sender;
	void *args;
};

static void basicEventsSingleHandlerImpl(testingT *t, void *data)
{
	struct basicEventsSingleHandlerParams *p = (struct basicEventsSingleHandlerParams *) data;
	uiEvent *e;
	uiEventOptions opts;
	struct handler h;

	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = p->global;
	e = uiNewEvent(&opts);

	memset(&h, 0, sizeof (struct handler));
	h.name = "handler";
	uiEventAddHandler(e, handler, p->sender, &h);

	uiEventFire(e, p->sender, p->args);
	checkHandlerRun(h, p->sender, p->args);
}

static void basicEventsSingleHandlerSubtestArgs(testingT *t, void *data)
{
	struct basicEventsSingleHandlerParams *p = (struct basicEventsSingleHandlerParams *) data;

	p->args = &p;
	testingTRun(t, "Args", basicEventsSingleHandlerImpl, p);
	p->args = NULL;
	testingTRun(t, "NoArgs", basicEventsSingleHandlerImpl, p);
}

testingTest(BasicEventsSingleHandler)
{
	struct basicEventsSingleHandlerParams p;

	memset(&p, 0, sizeof (struct basicEventsSingleHandlerParams));
	p.global = true;
	p.sender = NULL;
	testingTRun(t, "Global", basicEventsSingleHandlerSubtestArgs, &p);
	p.global = false;
	p.sender = t;
	testingTRun(t, "Nonglobal", basicEventsSingleHandlerSubtestArgs, &p);
}

struct basicEventsAddDeleteParams {
	bool global;
	void *sender;
	void *args;
	uiEvent *e;
	struct handler h[6];
	struct handler hbase[6];
	int handler1, handler2, handler3;
	int newHandler1, newHandler2, newHandler3;
};

testingTest(BasicEventsAddDeleteEventHandlers)
{
	struct basicEventsAddDeleteParams *p = (struct basicEventsAddDeleteParams *) data;
	uiEventOptions opts;

	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = p->global;
	p->e = uiNewEvent(&opts);

	memset(p->hbase, 0, 6 * sizeof (struct handler));
	p->hbase[0].name = "handler 1";
	p->hbase[1].name = "handler 2";
	p->hbase[2].name = "handler 3";
	p->hbase[3].name = "new handler 1";
	p->hbase[4].name = "new handler 2";
	p->hbase[5].name = "new handler 3";

	testingTLogf(t, "*** initial handlers");
	p->handler1 = uiEventAddHandler(p->e, handler, p->sender, p->h + 0);
	p->handler2 = uiEventAddHandler(p->e, handler, p->sender, p->h + 1);
	p->handler3 = uiEventAddHandler(p->e, handler, p->sender, p->h + 2);

	memmove(p->h, p->hbase, 6 * sizeof (struct handler));
	uiEventFire(p->e, p->sender, p->args);
	checkHandlerRun(p->h[0], p->sender, p->args);
	checkHandlerRun(p->h[1], p->sender, p->args);
	checkHandlerRun(p->h[2], p->sender, p->args);
	checkHandlerNotRun(p->h[3]);
	checkHandlerNotRun(p->h[4]);
	checkHandlerNotRun(p->h[5]);

	testingTLogf(t, "*** deleting a handler from the middle");
	uiEventDeleteHandler(p->e, p->handler2);

	memmove(p->h, p->hbase, 6 * sizeof (struct handler));
	uiEventFire(p->e, p->sender, p->args);
	checkHandlerRun(p->h[0], p->sender, p->args);
	checkHandlerNotRun(p->h[1]);
	checkHandlerRun(p->h[2], p->sender, p->args);
	checkHandlerNotRun(p->h[3]);
	checkHandlerNotRun(p->h[4]);
	checkHandlerNotRun(p->h[5]);

	testingTLogf(t, "*** adding handler after deleting a handler from the middle");
	p->newHandler1 = uiEventAddHandler(p->e, handler, p->sender, p->h + 3);

	memmove(p->h, p->hbase, 6 * sizeof (struct handler));
	uiEventFire(p->e, p->sender, p->args);
	checkHandlerRun(p->h[0], p->sender, p->args);
	checkHandlerNotRun(p->h[1]);
	checkHandlerRun(p->h[2], p->sender, p->args);
	checkHandlerRun(p->h[3], p->sender, p->args);
	checkHandlerNotRun(p->h[4]);
	checkHandlerNotRun(p->h[5]);

	testingTLogf(t, "*** deleting first handler added and adding another");
	uiEventDeleteHandler(p->e, p->handler1);
	p->newHandler2 = uiEventAddHandler(p->e, handler, p->sender, p->h + 4);

	memmove(p->h, p->hbase, 6 * sizeof (struct handler));
	uiEventFire(p->e, p->sender, p->args);
	checkHandlerNotRun(p->h[0]);
	checkHandlerNotRun(p->h[1]);
	checkHandlerRun(p->h[2], p->sender, p->args);
	checkHandlerRun(p->h[3], p->sender, p->args);
	checkHandlerRun(p->h[4], p->sender, p->args);
	checkHandlerNotRun(p->h[5]);

	testingTLogf(t, "*** deleting most recently added handler and adding another");
	uiEventDeleteHandler(p->e, p->newHandler2);
	p->newHandler3 = uiEventAddHandler(p->e, handler, p->sender, p->h + 5);

	memmove(p->h, p->hbase, 6 * sizeof (struct handler));
	uiEventFire(p->e, p->sender, p->args);
	checkHandlerNotRun(p->h[0]);
	checkHandlerNotRun(p->h[1]);
	checkHandlerRun(p->h[2], p->sender, p->args);
	checkHandlerRun(p->h[3], p->sender, p->args);
	checkHandlerNotRun(p->h[4]);
	checkHandlerRun(p->h[5], p->sender, p->args);

	testingTLogf(t, "*** deleting all handlers");
	uiEventDeleteHandler(p->e, p->handler3);
	uiEventDeleteHandler(p->e, p->newHandler1);
	uiEventDeleteHandler(p->e, p->newHandler3);

	memmove(p->h, p->hbase, 6 * sizeof (struct handler));
	uiEventFire(p->e, p->sender, p->args);
	checkHandlerNotRun(p->h[0]);
	checkHandlerNotRun(p->h[1]);
	checkHandlerNotRun(p->h[2]);
	checkHandlerNotRun(p->h[3]);
	checkHandlerNotRun(p->h[4]);
	checkHandlerNotRun(p->h[5]);

	testingTLogf(t, "*** adding handler after deleting all handlers");
	uiEventAddHandler(p->e, handler, p->sender, p->h + 0);

	memmove(p->h, p->hbase, 6 * sizeof (struct handler));
	uiEventFire(p->e, p->sender, p->args);
	checkHandlerRun(p->h[0], p->sender, p->args);
	checkHandlerNotRun(p->h[1]);
	checkHandlerNotRun(p->h[2]);
	checkHandlerNotRun(p->h[3]);
	checkHandlerNotRun(p->h[4]);
	checkHandlerNotRun(p->h[5]);
}

testingTest(EventErrors)
{
	// TODO
}
