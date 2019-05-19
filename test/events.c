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

#define TestBasicEventsSingleHandler(subname, whichGlobal, whichSender, whichArgs) \
	testingTest(BasicEventsSingleHandler ## subname) \
	{ \
		uiEvent *e; \
		uiEventOptions opts; \
		struct handler h; \
		memset(&opts, 0, sizeof (uiEventOptions)); \
		opts.Size = sizeof (uiEventOptions); \
		opts.Global = whichGlobal; \
		e = uiNewEvent(&opts); \
		memset(&h, 0, sizeof (struct handler)); \
		h.name = "handler"; \
		uiEventAddHandler(e, handler, whichSender, &h); \
		uiEventFire(e, whichSender, whichArgs); \
		checkHandlerRun(h, whichSender, whichArgs); \
	}
TestBasicEventsSingleHandler(Global_Args, true, NULL, &h)
TestBasicEventsSingleHandler(Global_NoArgs, true, NULL, NULL)
TestBasicEventsSingleHandler(Nonglobal_Args, false, &opts, &h)
TestBasicEventsSingleHandler(Nonglobal_NoArgs, false, &opts, NULL)

#define whichGlobal true
#define whichSender NULL
#define whichArgs h
testingTest(BasicEventsAddDeleteEventHandlers)
{
	uiEvent *e;
	uiEventOptions opts;
	struct handler hbase[6];
	struct handler h[6];
	int firstHandler, middleHandler, lastHandler;

	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = whichGlobal;
	e = uiNewEvent(&opts);

	memset(hbase, 0, 6 * sizeof (struct handler));
	hbase[0].name = "first handler";
	hbase[1].name = "middle handler";
	hbase[2].name = "last handler";
	hbase[3].name = "new handler 1";
	hbase[4].name = "new handler 2";
	hbase[5].name = "new handler 3";

	testingTLogf(t, "*** initial handlers");
	firstHandler = uiEventAddHandler(e, handler, whichSender, h + 0);
	middleHandler = uiEventAddHandler(e, handler, whichSender, h + 1);
	uiEventAddHandler(e, handler, whichSender, h + 2);

	memmove(h, hbase, 6 * sizeof (struct handler));
	uiEventFire(e, whichSender, whichArgs);
	checkHandlerRun(h[0], whichSender, whichArgs);
	checkHandlerRun(h[1], whichSender, whichArgs);
	checkHandlerRun(h[2], whichSender, whichArgs);
	checkHandlerNotRun(h[3]);
	checkHandlerNotRun(h[4]);
	checkHandlerNotRun(h[5]);

	testingTLogf(t, "*** deleting middle handler");
	uiEventDeleteHandler(e, middleHandler);

	memmove(h, hbase, 6 * sizeof (struct handler));
	uiEventFire(e, whichSender, whichArgs);
	checkHandlerRun(h[0], whichSender, whichArgs);
	checkHandlerNotRun(h[1]);
	checkHandlerRun(h[2], whichSender, whichArgs);
	checkHandlerNotRun(h[3]);
	checkHandlerNotRun(h[4]);
	checkHandlerNotRun(h[5]);

	testingTLogf(t, "*** adding handler after deleting middle handler");
	uiEventAddHandler(e, handler, whichSender, h + 3);

	memmove(h, hbase, 6 * sizeof (struct handler));
	uiEventFire(e, whichSender, whichArgs);
	checkHandlerRun(h[0], whichSender, whichArgs);
	checkHandlerNotRun(h[1]);
	checkHandlerRun(h[2], whichSender, whichArgs);
	checkHandlerRun(h[3], whichSender, whichArgs);
	checkHandlerNotRun(h[4]);
	checkHandlerNotRun(h[5]);

	testingTLogf(t, "*** deleting first handler and adding another");
	uiEventDeleteHandler(e, firstHandler);
	lastHandler = uiEventAddHandler(e, handler, whichSender, h + 4);

	memmove(h, hbase, 6 * sizeof (struct handler));
	uiEventFire(e, whichSender, whichArgs);
	checkHandlerNotRun(h[0]);
	checkHandlerNotRun(h[1]);
	checkHandlerRun(h[2], whichSender, whichArgs);
	checkHandlerRun(h[3], whichSender, whichArgs);
	checkHandlerRun(h[4], whichSender, whichArgs);
	checkHandlerNotRun(h[5]);

	testingTLogf(t, "*** deleting last handler and adding another");
	uiEventDeleteHandler(e, lastHandler);
	uiEventAddHandler(e, handler, whichSender, h + 5);

	memmove(h, hbase, 6 * sizeof (struct handler));
	uiEventFire(e, whichSender, whichArgs);
	checkHandlerNotRun(h[0]);
	checkHandlerNotRun(h[1]);
	checkHandlerRun(h[2], whichSender, whichArgs);
	checkHandlerRun(h[3], whichSender, whichArgs);
	checkHandlerNotRun(h[4]);
	checkHandlerRun(h[5], whichSender, whichArgs);
}

testingTest(EventErrors)
{
	// TODO
}
