// 18 may 2019
#include <stdlib.h>
#include <string.h>
#include "test.h"

struct handler {
	int id;
	bool validID;
	const char *name;

	bool gotRun;
	void *gotSender;
	void *gotArgs;

	bool wantRun;
	void *wantSender;
	void *wantArgs;
	bool wantBlocked;

	int *runCount;
};

static void handler(void *sender, void *args, void *data)
{
	struct handler *h = (struct handler *) data;

	h->gotRun = true;
	h->gotSender = sender;
	h->gotArgs = args;
	(*(h->runCount))++;
}

static void resetGot(struct handler *h, int *runCount)
{
	h->gotRun = false;
	h->gotSender = NULL;
	h->gotArgs = NULL;
	h->runCount = runCount;
}

static void registerHandler(struct handler *h, uiEvent *e, void *sender, void *args)
{
	h->wantSender = sender;
	h->wantArgs = args;
	h->id = uiEventAddHandler(e, handler, h->wantSender, h);
	h->validID = true;
}

static void unregisterHandler(struct handler *h, uiEvent *e)
{
	uiEventDeleteHandler(e, h->id);
	h->validID = false;
}

static void wantRun(struct handler *h)
{
	h->wantRun = true;
	h->wantBlocked = false;
}

static void wantNotRun(struct handler *h)
{
	h->wantRun = false;
	h->wantBlocked = false;
}

static void wantBlocked(struct handler *h)
{
	h->wantRun = false;
	h->wantBlocked = true;
}

// TODO carry over the file nad line numbers somehow
static void run(testingT *t, uiEvent *e, void *sender, void *args, struct handler *handlers, int n, int wantRunCount)
{
	int i;
	int gotRunCount;
	struct handler *h;
	bool gotBlocked;

	gotRunCount = 0;
	for (i = 0; i < n; i++)
		resetGot(handlers + i, &gotRunCount);

	uiEventFire(e, sender, args);

	h = handlers;
	for (i = 0; i < n; i++) {
		if (!h->gotRun && h->wantRun)
			testingTErrorf(t, "%s not run; should have been", h->name);
		else if (h->gotRun && !h->wantRun)
			testingTErrorf(t, "%s run; should not have been", h->name);
		if (h->gotRun && h->wantRun) {
			// only check these if it was correctly run, to reduce noise if the above failed
			if (h->gotSender != h->wantSender)
				diff_2str(t, "incorrect sender seen by", h->name,
					"%p", h->gotSender, h->wantSender);
			if (h->gotArgs != h->wantArgs)
				diff_2str(t, "incorrect args seen by", h->name,
					"%p", h->gotArgs, h->wantArgs);
		}
		if (h->validID) {
			// the following call will fail if the ID isn't valid
			gotBlocked = uiEventHandlerBlocked(e, h->id);
			if (!gotBlocked && h->wantBlocked)
				testingTErrorf(t, "%s not blocked; should have been", h->name);
			else if (gotBlocked && !h->wantBlocked)
				testingTErrorf(t, "%s blocked; should not have been", h->name);
		}
		h++;
	}
	if (gotRunCount != wantRunCount)
		diff(t, "incorrect number of handler runs",
			"%d", gotRunCount, wantRunCount);
}

struct baseParams {
	void (*impl)(testingT *t, void *data);
	bool global;
	void *sender;
	void *args;
};

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

static void basicEventFunctionalityImpl(testingT *t, void *data)
{
	struct baseParams *p = (struct baseParams *) data;
	uiEvent *e;
	uiEventOptions opts;
	struct handler h[1];

	memset(h, 0, 1 * sizeof (struct handler));
	h[0].name = "handler";

	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = p->global;
	e = uiNewEvent(&opts);

	registerHandler(h + 0, e, p->sender, p->args);
	wantRun(h + 0);
	run(t, e, p->sender, p->args,
		h, 1, 1);
}

testingTest(BasicEventFunctionality)
{
	struct baseParams p;

	memset(&p, 0, sizeof (struct baseParams));
	p.impl = basicEventFunctionalityImpl;
	runGlobalSubtests(t, &p);
}

static void addDeleteEventHandlersImpl(testingT *t, void *data)
{
	struct baseParams *p = (struct baseParams *) data;
	uiEvent *e;
	uiEventOptions opts;
	struct handler h[6];

	memset(h, 0, 6 * sizeof (struct handler));
	h[0].name = "handler 1";
	h[1].name = "handler 2";
	h[2].name = "handler 3";
	h[3].name = "new handler 1";
	h[4].name = "new handler 2";
	h[5].name = "new handler 3";

	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = p->global;
	e = uiNewEvent(&opts);

	testingTLogf(t, "*** initial handlers");
	registerHandler(h + 0, e, p->sender, p->args);
	registerHandler(h + 1, e, p->sender, p->args);
	registerHandler(h + 2, e, p->sender, p->args);
	wantRun(h + 0);
	wantRun(h + 1);
	wantRun(h + 2);
	wantNotRun(h + 3);
	wantNotRun(h + 4);
	wantNotRun(h + 5);
	run(t, e, p->sender, p->args,
		h, 6, 3);

	testingTLogf(t, "*** deleting a handler from the middle");
	unregisterHandler(h + 1, e);
	wantRun(h + 0);
	wantNotRun(h + 1);
	wantRun(h + 2);
	wantNotRun(h + 3);
	wantNotRun(h + 4);
	wantNotRun(h + 5);
	run(t, e, p->sender, p->args,
		h, 6, 2);

	testingTLogf(t, "*** adding handler after deleting a handler from the middle");
	registerHandler(h + 3, e, p->sender, p->args);
	wantRun(h + 0);
	wantNotRun(h + 1);
	wantRun(h + 2);
	wantRun(h + 3);
	wantNotRun(h + 4);
	wantNotRun(h + 5);
	run(t, e, p-> sender, p->args,
		h, 6, 3);

	testingTLogf(t, "*** deleting first handler added and adding another");
	unregisterHandler(h + 0, e);
	registerHandler(h + 4, e, p->sender, p->args);
	wantNotRun(h + 0);
	wantNotRun(h + 1);
	wantRun(h + 2);
	wantRun(h + 3);
	wantRun(h + 4);
	wantNotRun(h + 5);
	run(t, e, p->sender, p->args,
		h, 6, 3);

	testingTLogf(t, "*** deleting most recently added handler and adding another");
	unregisterHandler(h + 4, e);
	registerHandler(h + 5, e, p->sender, p->args);
	wantNotRun(h + 0);
	wantNotRun(h + 1);
	wantRun(h + 2);
	wantRun(h + 3);
	wantNotRun(h + 4);
	wantRun(h + 5);
	run(t, e, p->sender, p->args,
		h, 6, 3);

	testingTLogf(t, "*** deleting all handlers");
	unregisterHandler(h + 2, e);
	unregisterHandler(h + 3, e);
	unregisterHandler(h + 5, e);
	wantNotRun(h + 0);
	wantNotRun(h + 1);
	wantNotRun(h + 2);
	wantNotRun(h + 3);
	wantNotRun(h  +4);
	wantNotRun(h + 5);
	run(t, e, p->sender, p->args,
		h, 6, 0);

	testingTLogf(t, "*** adding handler after deleting all handlers");
	registerHandler(h + 0, e, p->sender, p->args);
	wantRun(h + 0);
	wantNotRun(h + 1);
	wantNotRun(h + 2);
	wantNotRun(h + 3);
	wantNotRun(h + 4);
	wantNotRun(h + 5);
	run(t, e, p->sender, p->args,
		h, 6, 1);
}

testingTest(AddDeleteEventHandlers)
{
	struct baseParams p;

	memset(&p, 0, sizeof (struct baseParams));
	p.impl = addDeleteEventHandlersImpl;
	runGlobalSubtests(t, &p);
}

#if 0

struct eventSendersHonoredParams {
	struct baseParams bp;
	const char *names[4];
	uiEvent *e;
	struct handler got[4];
	void *sender1, *sender2, *sender3;
};

static void eventSendersHonoredImpl(testingT *t, void *data)
{
	struct eventSendersHonoredParams *p = (struct eventSendersHonoredParams *) data;
	struct runParams rp;
	struct handler want[4];

	memset(&rp, 0, sizeof (struct runParams));
	rp.args = p->bp.args;
	rp.nHandlers = 4;
	rp.names = p->names;
	rp.got = p->got;
	rp.want = want;
	memset(rp.want, 0, rp.nHandlers * sizeof (struct handler));

	rp.e = p->e;

	testingTLogf(t, "*** sender 1");
	rp.sender = p->sender1;
	wantRun(rp.want[0], p->sender1, p->bp.args);
	wantNotRun(rp.want[1]);
	wantNotRun(rp.want[2]);
	wantRun(rp.want[3], p->sender1, p->bp.args);
	rp.wantRunCount = 2;
	run(t, &rp);

	testingTLogf(t, "*** sender 2");
	rp.sender = p->sender2;
	wantNotRun(rp.want[0]);
	wantRun(rp.want[1], p->sender2, p->bp.args);
	wantNotRun(rp.want[2]);
	wantNotRun(rp.want[3]);
	rp.wantRunCount = 1;
	run(t, &rp);

	testingTLogf(t, "*** sender 3");
	rp.sender = p->sender3;
	wantNotRun(rp.want[0]);
	wantNotRun(rp.want[1]);
	wantRun(rp.want[2], p->sender3, p->bp.args);
	wantNotRun(rp.want[3]);
	rp.wantRunCount = 1;
	run(t, &rp);

	testingTLogf(t, "*** an entirely different sender");
	rp.sender = p;
	wantNotRun(rp.want[0]);
	wantNotRun(rp.want[1]);
	wantNotRun(rp.want[2]);
	wantNotRun(rp.want[3]);
	rp.wantRunCount = 0;
	run(t, &rp);
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

	p.names[0] = "sender 1 handler 1";
	p.names[1] = "sender 2 handler";
	p.names[2] = "sender 3 handler";
	p.names[3] = "sender 1 handler 2";

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

	uiEventAddHandler(p.e, handler, p.sender1, p.got + 0);
	uiEventAddHandler(p.e, handler, p.sender2, p.got + 1);
	uiEventAddHandler(p.e, handler, p.sender3, p.got + 2);
	uiEventAddHandler(p.e, handler, p.sender1, p.got + 3);

	runArgsSubtests(t, &p);

	free(p.sender3);
	free(p.sender2);
	free(p.sender1);
}

// TODO events being added and deleted with different senders

static void eventBlocksHonoredImpl(testingT *t, void *data)
{
	struct baseParams *p = (struct baseParams *) data;
	struct runParams rp;
	const char *names[3];
	struct handler got[3];
	struct handler want[3];
	uiEventOptions opts;
	int ids[3];

	memset(&rp, 0, sizeof (struct runParams));
	rp.sender = p->sender;
	rp.args = p->args;
	rp.nHandlers = 3;
	rp.names = names;
	rp.names[0] = "handler 1";
	rp.names[1] = "handler 2";
	rp.names[2] = "handler 3";
	rp.got = got;
	rp.want = want;
	memset(rp.want, 0, rp.nHandlers * sizeof (struct handler));

	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = p->global;
	rp.e = uiNewEvent(&opts);

	testingTLogf(t, "*** initial handlers are unblocked");
	ids[0] = uiEventAddHandler(rp.e, handler, p->sender, rp.got + 0);
	ids[1] = uiEventAddHandler(rp.e, handler, p->sender, rp.got + 1);
	ids[2] = uiEventAddHandler(rp.e, handler, p->sender, rp.got + 2);
	wantRun(rp.want[0], p->sender, p->args);
	wantRun(rp.want[1], p->sender, p->args);
	wantRun(rp.want[2], p->sender, p->args);
	rp.wantRunCount = 3;
	run(t, &rp);
	if (uiEventHandlerBlocked(rp.e, ids[0]))
		testingTErrorf(t, "handler 1 blocked; want unblocked");
	if (uiEventHandlerBlocked(rp.e, ids[1]))
		testingTErrorf(t, "handler 2 blocked; want unblocked");
	if (uiEventHandlerBlocked(rp.e, ids[2]))
		testingTErrorf(t, "handler 3 blocked; want unblocked");

	testingTLogf(t, "*** blocking handler 2 omits it");
	uiEventSetHandlerBlocked(rp.e, ids[1], true);
	wantRun(rp.want[0], p->sender, p->args);
	wantNotRun(rp.want[1]);
	wantRun(rp.want[2], p->sender, p->args);
	rp.wantRunCount = 2;
	run(t, &rp);
	if (uiEventHandlerBlocked(rp.e, ids[0]))
		testingTErrorf(t, "handler 1 blocked; want unblocked");
	if (!uiEventHandlerBlocked(rp.e, ids[1]))
		testingTErrorf(t, "handler 2 unblocked; want blocked");
	if (uiEventHandlerBlocked(rp.e, ids[2]))
		testingTErrorf(t, "handler 3 blocked; want unblocked");

	testingTLogf(t, "*** blocking handler 3 omits both 2 and 3");
	uiEventSetHandlerBlocked(rp.e, ids[2], true);
	wantRun(rp.want[0], p->sender, p->args);
	wantNotRun(rp.want[1]);
	wantNotRun(rp.want[2]);
	rp.wantRunCount = 1;
	run(t, &rp);
	if (uiEventHandlerBlocked(rp.e, ids[0]))
		testingTErrorf(t, "handler 1 blocked; want unblocked");
	if (!uiEventHandlerBlocked(rp.e, ids[1]))
		testingTErrorf(t, "handler 2 unblocked; want blocked");
	if (!uiEventHandlerBlocked(rp.e, ids[2]))
		testingTErrorf(t, "handler 3 unblocked; want blocked");

	testingTLogf(t, "*** unblocking handler 2 omits only 3");
	uiEventSetHandlerBlocked(rp.e, ids[1], false);
	wantRun(rp.want[0], p->sender, p->args);
	wantRun(rp.want[1], p->sender, p->args);
	wantNotRun(rp.want[2]);
	rp.wantRunCount = 2;
	run(t, &rp);
	if (uiEventHandlerBlocked(rp.e, ids[0]))
		testingTErrorf(t, "handler 1 blocked; want unblocked");
	if (uiEventHandlerBlocked(rp.e, ids[1]))
		testingTErrorf(t, "handler 2 blocked; want unblocked");
	if (!uiEventHandlerBlocked(rp.e, ids[2]))
		testingTErrorf(t, "handler 3 unblocked; want blocked");

	// TODO block all three and make sure nothing runs

	// TODO unblock everything and make sure they all run
}

testingTest(EventBlocksHonored)
{
	struct baseParams p;

	memset(&p, 0, sizeof (struct baseParams));
	p.impl = eventBlocksHonoredImpl;
	runGlobalSubtests(t, &p);
}

// TODO event blocks being honored with different senders

// TODO other combinations of the same

testingTest(EventErrors)
{
	// TODO
}

#endif
