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

#define whichGlobal true
#define whichSender NULL
#define whichArgs h
testingTest(BasicEventsAddDeleteEventHandlers)
{
	uiEvent *e;
	uiEventOptions opts;
	struct handler hbase[6];
	struct handler h[6];
	int handler1, handler2, handler3;
	int newHandler1, newHandler2, newHandler3;

	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = whichGlobal;
	e = uiNewEvent(&opts);

	memset(hbase, 0, 6 * sizeof (struct handler));
	hbase[0].name = "handler 1";
	hbase[1].name = "handler 2";
	hbase[2].name = "handler 3";
	hbase[3].name = "new handler 1";
	hbase[4].name = "new handler 2";
	hbase[5].name = "new handler 3";

	testingTLogf(t, "*** initial handlers");
	handler1 = uiEventAddHandler(e, handler, whichSender, h + 0);
	handler2 = uiEventAddHandler(e, handler, whichSender, h + 1);
	handler3 = uiEventAddHandler(e, handler, whichSender, h + 2);

	memmove(h, hbase, 6 * sizeof (struct handler));
	uiEventFire(e, whichSender, whichArgs);
	checkHandlerRun(h[0], whichSender, whichArgs);
	checkHandlerRun(h[1], whichSender, whichArgs);
	checkHandlerRun(h[2], whichSender, whichArgs);
	checkHandlerNotRun(h[3]);
	checkHandlerNotRun(h[4]);
	checkHandlerNotRun(h[5]);

	testingTLogf(t, "*** deleting a handler from the middle");
	uiEventDeleteHandler(e, handler2);

	memmove(h, hbase, 6 * sizeof (struct handler));
	uiEventFire(e, whichSender, whichArgs);
	checkHandlerRun(h[0], whichSender, whichArgs);
	checkHandlerNotRun(h[1]);
	checkHandlerRun(h[2], whichSender, whichArgs);
	checkHandlerNotRun(h[3]);
	checkHandlerNotRun(h[4]);
	checkHandlerNotRun(h[5]);

	testingTLogf(t, "*** adding handler after deleting a handler from the middle");
	newHandler1 = uiEventAddHandler(e, handler, whichSender, h + 3);

	memmove(h, hbase, 6 * sizeof (struct handler));
	uiEventFire(e, whichSender, whichArgs);
	checkHandlerRun(h[0], whichSender, whichArgs);
	checkHandlerNotRun(h[1]);
	checkHandlerRun(h[2], whichSender, whichArgs);
	checkHandlerRun(h[3], whichSender, whichArgs);
	checkHandlerNotRun(h[4]);
	checkHandlerNotRun(h[5]);

	testingTLogf(t, "*** deleting first handler added and adding another");
	uiEventDeleteHandler(e, handler1);
	newHandler2 = uiEventAddHandler(e, handler, whichSender, h + 4);

	memmove(h, hbase, 6 * sizeof (struct handler));
	uiEventFire(e, whichSender, whichArgs);
	checkHandlerNotRun(h[0]);
	checkHandlerNotRun(h[1]);
	checkHandlerRun(h[2], whichSender, whichArgs);
	checkHandlerRun(h[3], whichSender, whichArgs);
	checkHandlerRun(h[4], whichSender, whichArgs);
	checkHandlerNotRun(h[5]);

	testingTLogf(t, "*** deleting most recently added handler and adding another");
	uiEventDeleteHandler(e, newHandler2);
	newHandler3 = uiEventAddHandler(e, handler, whichSender, h + 5);

	memmove(h, hbase, 6 * sizeof (struct handler));
	uiEventFire(e, whichSender, whichArgs);
	checkHandlerNotRun(h[0]);
	checkHandlerNotRun(h[1]);
	checkHandlerRun(h[2], whichSender, whichArgs);
	checkHandlerRun(h[3], whichSender, whichArgs);
	checkHandlerNotRun(h[4]);
	checkHandlerRun(h[5], whichSender, whichArgs);

	testingTLogf(t, "*** deleting all handlers");
	uiEventDeleteHandler(e, handler3);
	uiEventDeleteHandler(e, newHandler1);
	uiEventDeleteHandler(e, newHandler3);

	memmove(h, hbase, 6 * sizeof (struct handler));
	uiEventFire(e, whichSender, whichArgs);
	checkHandlerNotRun(h[0]);
	checkHandlerNotRun(h[1]);
	checkHandlerNotRun(h[2]);
	checkHandlerNotRun(h[3]);
	checkHandlerNotRun(h[4]);
	checkHandlerNotRun(h[5]);

	testingTLogf(t, "*** adding handler after deleting all handlers");
	uiEventAddHandler(e, handler, whichSender, h + 0);

	memmove(h, hbase, 6 * sizeof (struct handler));
	uiEventFire(e, whichSender, whichArgs);
	checkHandlerRun(h[0], whichSender, whichArgs);
	checkHandlerNotRun(h[1]);
	checkHandlerNotRun(h[2]);
	checkHandlerNotRun(h[3]);
	checkHandlerNotRun(h[4]);
	checkHandlerNotRun(h[5]);
}

testingTest(EventErrors)
{
	// TODO
}
