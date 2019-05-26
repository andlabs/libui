// 18 may 2019
#include <stdlib.h>
#include <string.h>
#include "test.h"

struct handler {
	bool run;
	void *sender;
	void *args;
	int *runCount;
};

static void handler(void *sender, void *args, void *data)
{
	struct handler *h = (struct handler *) data;

	h->run = true;
	h->sender = sender;
	h->args = args;
	(*(h->runCount))++;
}

struct runParams {
	uiEvent *e;
	void *sender;
	void *args;
	int nHandlers;
	const char **names;
	struct handler *got;
	struct handler *want;
	int wantRunCount;
};

#define wantRun(want, s, a) \
	want.run = true; \
	want.sender = s; \
	want.args = a;
#define wantNotRun(want) \
	want.run = false;

// TODO carry over the file nad line numbers somehow
static void run(testingT *t, struct runParams *p)
{
	int i;
	int gotRunCount;

	memset(p->got, 0, p->nHandlers * sizeof (struct handler));
	for (i = 0; i < p->nHandlers; i++)
		p->got[i].runCount = &gotRunCount;

	gotRunCount = 0;
	uiEventFire(p->e, p->sender, p->args);

	for (i = 0; i < p->nHandlers; i++) {
		if (!p->want[i].run) {
			if (p->got[i].run)
				testingTErrorf(t, "%s run; should not have been", p->names[i]);
			continue;
		}
		// otherwise we want it to be run
		if (!p->got[i].run) {
			testingTErrorf(t, "%s not run; should have been", p->names[i]);
			continue;
		}
		if (p->got[i].sender != p->want[i].sender)
			diff_2str(t, "incorrect sender seen by", p->names[i],
				"%p", p->got[i].sender, p->want[i].sender);
		if (p->got[i].args != p->want[i].args)
			diff_2str(t, "incorrect args seen by", p->names[i],
				"%p", p->got[i].args, p->want[i].args);
	}
	if (gotRunCount != p->wantRunCount)
		diff(t, "incorrect number of handler runs",
			"%d", gotRunCount, p->wantRunCount);
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
	struct runParams rp;
	uiEventOptions opts;
	const char *names[1];
	struct handler got[1];
	struct handler want[1];

	memset(&rp, 0, sizeof (struct runParams));
	rp.sender = p->sender;
	rp.args = p->args;
	rp.nHandlers = 1;
	rp.names = names;
	rp.names[0] = "handler";
	rp.got = got;
	rp.want = want;
	memset(rp.want, 0, rp.nHandlers * sizeof (struct handler));

	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = p->global;
	rp.e = uiNewEvent(&opts);

	uiEventAddHandler(rp.e, handler, p->sender, rp.got + 0);
	wantRun(rp.want[0], p->sender, p->args);
	rp.wantRunCount = 1;
	run(t, &rp);
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
	struct runParams rp;
	const char *names[6];
	struct handler got[6];
	struct handler want[6];
	int handler1, handler2, handler3;
	int newHandler1, newHandler2, newHandler3;
	uiEventOptions opts;

	memset(&rp, 0, sizeof (struct runParams));
	rp.sender = p->sender;
	rp.args = p->args;
	rp.nHandlers = 6;
	rp.names = names;
	rp.names[0] = "handler 1";
	rp.names[1] = "handler 2";
	rp.names[2] = "handler 3";
	rp.names[3] = "new handler 1";
	rp.names[4] = "new handler 2";
	rp.names[5] = "new handler 3";
	rp.got = got;
	rp.want = want;
	memset(rp.want, 0, rp.nHandlers * sizeof (struct handler));

	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = p->global;
	rp.e = uiNewEvent(&opts);

	testingTLogf(t, "*** initial handlers");
	handler1 = uiEventAddHandler(rp.e, handler, p->sender, rp.got + 0);
	handler2 = uiEventAddHandler(rp.e, handler, p->sender, rp.got + 1);
	handler3 = uiEventAddHandler(rp.e, handler, p->sender, rp.got + 2);
	wantRun(rp.want[0], p->sender, p->args);
	wantRun(rp.want[1], p->sender, p->args);
	wantRun(rp.want[2], p->sender, p->args);
	wantNotRun(rp.want[3]);
	wantNotRun(rp.want[4]);
	wantNotRun(rp.want[5]);
	rp.wantRunCount = 3;
	run(t, &rp);

	testingTLogf(t, "*** deleting a handler from the middle");
	uiEventDeleteHandler(rp.e, handler2);
	wantRun(rp.want[0], p->sender, p->args);
	wantNotRun(rp.want[1]);
	wantRun(rp.want[2], p->sender, p->args);
	wantNotRun(rp.want[3]);
	wantNotRun(rp.want[4]);
	wantNotRun(rp.want[5]);
	rp.wantRunCount = 2;
	run(t, &rp);

	testingTLogf(t, "*** adding handler after deleting a handler from the middle");
	newHandler1 = uiEventAddHandler(rp.e, handler, p->sender, rp.got + 3);
	wantRun(rp.want[0], p->sender, p->args);
	wantNotRun(rp.want[1]);
	wantRun(rp.want[2], p->sender, p->args);
	wantRun(rp.want[3], p->sender, p->args);
	wantNotRun(rp.want[4]);
	wantNotRun(rp.want[5]);
	rp.wantRunCount = 3;
	run(t, &rp);

	testingTLogf(t, "*** deleting first handler added and adding another");
	uiEventDeleteHandler(rp.e, handler1);
	newHandler2 = uiEventAddHandler(rp.e, handler, p->sender, rp.got + 4);
	wantNotRun(rp.want[0]);
	wantNotRun(rp.want[1]);
	wantRun(rp.want[2], p->sender, p->args);
	wantRun(rp.want[3], p->sender, p->args);
	wantRun(rp.want[4], p->sender, p->args);
	wantNotRun(rp.want[5]);
	rp.wantRunCount = 3;
	run(t, &rp);

	testingTLogf(t, "*** deleting most recently added handler and adding another");
	uiEventDeleteHandler(rp.e, newHandler2);
	newHandler3 = uiEventAddHandler(rp.e, handler, p->sender, rp.got + 5);
	wantNotRun(rp.want[0]);
	wantNotRun(rp.want[1]);
	wantRun(rp.want[2], p->sender, p->args);
	wantRun(rp.want[3], p->sender, p->args);
	wantNotRun(rp.want[4]);
	wantRun(rp.want[5], p->sender, p->args);
	rp.wantRunCount = 3;
	run(t, &rp);

	testingTLogf(t, "*** deleting all handlers");
	uiEventDeleteHandler(rp.e, handler3);
	uiEventDeleteHandler(rp.e, newHandler1);
	uiEventDeleteHandler(rp.e, newHandler3);
	wantNotRun(rp.want[0]);
	wantNotRun(rp.want[1]);
	wantNotRun(rp.want[2]);
	wantNotRun(rp.want[3]);
	wantNotRun(rp.want[4]);
	wantNotRun(rp.want[5]);
	rp.wantRunCount = 0;
	run(t, &rp);

	testingTLogf(t, "*** adding handler after deleting all handlers");
	uiEventAddHandler(rp.e, handler, p->sender, rp.got + 0);
	wantRun(rp.want[0], p->sender, p->args);
	wantNotRun(rp.want[1]);
	wantNotRun(rp.want[2]);
	wantNotRun(rp.want[3]);
	wantNotRun(rp.want[4]);
	wantNotRun(rp.want[5]);
	rp.wantRunCount = 1;
	run(t, &rp);
}

testingTest(AddDeleteEventHandlers)
{
	struct baseParams p;

	memset(&p, 0, sizeof (struct baseParams));
	p.impl = addDeleteEventHandlersImpl;
	runGlobalSubtests(t, &p);
}

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
