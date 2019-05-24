// 18 may 2019
#include <stdlib.h>
#include <string.h>
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

struct basicEventFunctionalityParams {
	struct baseParams bp;
};

static void basicEventFunctionalityImpl(testingT *t, void *data)
{
	struct basicEventFunctionalityParams *p = (struct basicEventFunctionalityParams *) data;
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

testingTest(BasicEventFunctionality)
{
	struct basicEventFunctionalityParams p;

	memset(&p, 0, sizeof (struct basicEventFunctionalityParams));
	p.bp.impl = basicEventFunctionalityImpl;
	runGlobalSubtests(t, &p);
}

struct addDeleteEventHandlers {
	struct baseParams bp;
	uiEvent *e;
	struct handler h[6];
	struct handler hbase[6];
	int handler1, handler2, handler3;
	int newHandler1, newHandler2, newHandler3;
};

static void addDeleteEventHandlersImpl(testingT *t, void *data)
{
	struct addDeleteEventHandlers *p = (struct addDeleteEventHandlers *) data;
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

testingTest(AddDeleteEventHandlers)
{
	struct addDeleteEventHandlers p;

	memset(&p, 0, sizeof (struct addDeleteEventHandlers));
	p.bp.impl = addDeleteEventHandlersImpl;
	runGlobalSubtests(t, &p);
}

struct eventSendersHonoredParams {
	struct baseParams bp;
	uiEvent *e;
	struct handler h[4];
	struct handler hbase[4];
	void *sender1, *sender2, *sender3;
};

static void eventSendersHonoredImpl(testingT *t, void *data)
{
	struct eventSendersHonoredParams *p = (struct eventSendersHonoredParams *) data;

	testingTLogf(t, "*** sender 1");
	memmove(p->h, p->hbase, 4 * sizeof (struct handler));
	p->bp.sender = p->sender1;
	uiEventFire(p->e, p->bp.sender, p->bp.args);
	checkHandlerRun(p->h[0], p->bp);
	checkHandlerNotRun(p->h[1]);
	checkHandlerNotRun(p->h[2]);
	checkHandlerRun(p->h[3], p->bp);

	testingTLogf(t, "*** sender 2");
	memmove(p->h, p->hbase, 4 * sizeof (struct handler));
	p->bp.sender = p->sender2;
	uiEventFire(p->e, p->bp.sender, p->bp.args);
	checkHandlerNotRun(p->h[0]);
	checkHandlerRun(p->h[1], p->bp);
	checkHandlerNotRun(p->h[2]);
	checkHandlerNotRun(p->h[3]);

	testingTLogf(t, "*** sender 3");
	memmove(p->h, p->hbase, 4 * sizeof (struct handler));
	p->bp.sender = p->sender3;
	uiEventFire(p->e, p->bp.sender, p->bp.args);
	checkHandlerNotRun(p->h[0]);
	checkHandlerNotRun(p->h[1]);
	checkHandlerRun(p->h[2], p->bp);
	checkHandlerNotRun(p->h[3]);

	testingTLogf(t, "*** an entirely different sender");
	memmove(p->h, p->hbase, 4 * sizeof (struct handler));
	p->bp.sender = p;
	uiEventFire(p->e, p->bp.sender, p->bp.args);
	checkHandlerNotRun(p->h[0]);
	checkHandlerNotRun(p->h[1]);
	checkHandlerNotRun(p->h[2]);
	checkHandlerNotRun(p->h[3]);
}

testingTest(EventSendersHonored)
{
	struct eventSendersHonoredParams p;
	uiEventOptions opts;

	memset(&p, 0, sizeof (struct eventSendersHonoredParams));
	p.bp.impl = eventSendersHonoredImpl;

	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = false;
	p.e = uiNewEvent(&opts);

	memset(p.hbase, 0, 4 * sizeof (struct handler));
	p.hbase[0].name = "sender 1 handler 1";
	p.hbase[1].name = "sender 2 handler";
	p.hbase[2].name = "sender 3 handler";
	p.hbase[3].name = "sender 1 handler 2";

	// dynamically allocate these so we don't run the risk of upsetting an optimizer somewhere, since we don't touch this memory
	p.sender1 = malloc(16);
	if (p.sender1 == NULL)
		testingTFatalf(t, "memory exhausted allocating sender 1");
	memset(p.sender1, 5, 16);
	p.sender2 = malloc(32);
	if (p.sender2 == NULL)
		testingTFatalf(t, "memory exhausted allocating sender 2");
	memset(p.sender2, 10, 32);
	p.sender3 = malloc(64);
	if (p.sender3 == NULL)
		testingTFatalf(t, "memory exhausted allocating sender 3");
	memset(p.sender3, 15, 64);

	uiEventAddHandler(p.e, handler, p.sender1, p.h + 0);
	uiEventAddHandler(p.e, handler, p.sender2, p.h + 1);
	uiEventAddHandler(p.e, handler, p.sender3, p.h + 2);
	uiEventAddHandler(p.e, handler, p.sender1, p.h + 3);

	runArgsSubtests(t, &p);

	free(p.sender3);
	free(p.sender2);
	free(p.sender1);
}

// TODO events being added and deleted with different senders

struct eventBlocksHonoredParams {
	struct baseParams bp;
	struct handler h[3];
	struct handler hbase[3];
};

static void eventBlocksHonoredImpl(testingT *t, void *data)
{
	struct eventBlocksHonoredParams *p = (struct eventBlocksHonoredParams *) data;
	uiEvent *e;
	uiEventOptions opts;
	int ids[3];

	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = p->bp.global;
	e = uiNewEvent(&opts);

	memset(p->hbase, 0, 3 * sizeof (struct handler));
	p->hbase[0].name = "handler 1";
	p->hbase[1].name = "handler 2";
	p->hbase[2].name = "handler 3";

	testingTLogf(t, "*** initial handlers are unblocked");
	ids[0] = uiEventAddHandler(e, handler, p->bp.sender, p->h + 0);
	ids[1] = uiEventAddHandler(e, handler, p->bp.sender, p->h + 1);
	ids[2] = uiEventAddHandler(e, handler, p->bp.sender, p->h + 2);

	memmove(p->h, p->hbase, 3 * sizeof (struct handler));
	uiEventFire(e, p->bp.sender, p->bp.args);
	if (uiEventHandlerBlocked(e, ids[0]))
		testingTErrorf(t, "handler 1 blocked; want unblocked");
	checkHandlerRun(p->h[0], p->bp);
	if (uiEventHandlerBlocked(e, ids[1]))
		testingTErrorf(t, "handler 2 blocked; want unblocked");
	checkHandlerRun(p->h[1], p->bp);
	if (uiEventHandlerBlocked(e, ids[2]))
		testingTErrorf(t, "handler 3 blocked; want unblocked");
	checkHandlerRun(p->h[2], p->bp);

	testingTLogf(t, "*** blocking handler 2 omits it");
	uiEventSetHandlerBlocked(e, ids[1], true);

	memmove(p->h, p->hbase, 3 * sizeof (struct handler));
	uiEventFire(e, p->bp.sender, p->bp.args);
	if (uiEventHandlerBlocked(e, ids[0]))
		testingTErrorf(t, "handler 1 blocked; want unblocked");
	checkHandlerRun(p->h[0], p->bp);
	if (!uiEventHandlerBlocked(e, ids[1]))
		testingTErrorf(t, "handler 2 unblocked; want blocked");
	checkHandlerNotRun(p->h[1]);
	if (uiEventHandlerBlocked(e, ids[2]))
		testingTErrorf(t, "handler 3 blocked; want unblocked");
	checkHandlerRun(p->h[2], p->bp);

	testingTLogf(t, "*** blocking handler 3 omits both 2 and 3");
	uiEventSetHandlerBlocked(e, ids[2], true);

	memmove(p->h, p->hbase, 3 * sizeof (struct handler));
	uiEventFire(e, p->bp.sender, p->bp.args);
	if (uiEventHandlerBlocked(e, ids[0]))
		testingTErrorf(t, "handler 1 blocked; want unblocked");
	checkHandlerRun(p->h[0], p->bp);
	if (!uiEventHandlerBlocked(e, ids[1]))
		testingTErrorf(t, "handler 2 unblocked; want blocked");
	checkHandlerNotRun(p->h[1]);
	if (!uiEventHandlerBlocked(e, ids[2]))
		testingTErrorf(t, "handler 3 unblocked; want blocked");
	checkHandlerNotRun(p->h[2]);

	testingTLogf(t, "*** unblocking handler 2 omits only 3");
	uiEventSetHandlerBlocked(e, ids[1], false);

	memmove(p->h, p->hbase, 3 * sizeof (struct handler));
	uiEventFire(e, p->bp.sender, p->bp.args);
	if (uiEventHandlerBlocked(e, ids[0]))
		testingTErrorf(t, "handler 1 blocked; want unblocked");
	checkHandlerRun(p->h[0], p->bp);
	if (uiEventHandlerBlocked(e, ids[1]))
		testingTErrorf(t, "handler 2 blocked; want unblocked");
	checkHandlerRun(p->h[1], p->bp);
	if (!uiEventHandlerBlocked(e, ids[2]))
		testingTErrorf(t, "handler 3 unblocked; want blocked");
	checkHandlerNotRun(p->h[2]);

	// TODO block all three and make sure nothing runs
	// TODO also add a general "handler() not called" check to all these "no handler run" checks (or better: a total run counter)

	// TODO unblock everything and make sure they all run
}

testingTest(EventBlocksHonored)
{
	struct eventBlocksHonoredParams p;

	memset(&p, 0, sizeof (struct eventBlocksHonoredParams));
	p.bp.impl = eventBlocksHonoredImpl;
	runGlobalSubtests(t, &p);
}

// TODO event blocks being honored with different senders

// TODO other combinations of the same

testingTest(EventErrors)
{
	// TODO
}
