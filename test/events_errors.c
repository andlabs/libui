// 12 june 2019
#include "test.h"

static void dummyHandler(void *sender, void *args, void *data)
{
	// do nothing
}

struct checkErrorParams {
	uiEvent *globalEvent;
	uiEvent *nonglobalEvent;
	uiEventOptions eventOptionsBadSize;
	uiEvent *eventPlaceholder;
	uiEventHandler handlerPlaceholder;
	void *senderPlaceholder;
	void *argsPlaceholder;
	void *dataPlaceholder;
	void *nonNullSender;
	int idPlaceholder;
	bool blockedPlaceholder;
	uiEvent *firingEvent;
	uiEvent *eventWithHandlers;
};

#define checkErrorCaseWhileFiringFull(line, call, msgWant) \
	static void checkCat(eventHandler, line)(void *sender, void *args, void *data) \
	{ \
		struct checkErrorParams *p = (struct checkErrorParams *) data; \
		(void) p; /* in the event call does not use this */ \
		call; \
	} \
	static void checkCat(runEventCheck, line)(struct checkErrorParams *p) \
	{ \
		int id; \
		id = uiEventAddHandler(p->firingEvent, checkCat(eventHandler, line), NULL, p); \
		uiEventFire(p->firingEvent, NULL, NULL); \
		uiEventDeleteHandler(p->firingEvent, id); \
	} \
	checkErrorCaseFull(line, checkCat(runEventCheck, line)(p), msgWant)
#define checkErrorCaseWhileFiring(call, msgWant) checkErrorCaseWhileFiringFull(__LINE__, call, msgWant)
#include "events_errors.h"
#undef checkErrorCaseWhileFiringFull
#undef checkErrorCaseWhileFiring

testingTest(EventErrors)
{
	struct checkErrorParams p;
	uiEventOptions opts;
	size_t i;

	memset(&p, 0, sizeof (struct checkErrorParams));
	p.eventOptionsBadSize.Size = 1;
	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = true;
	p.globalEvent = uiNewEvent(&opts);
	testingTDefer(t, deferEventFree, p.globalEvent);
	opts.Global = false;
	p.nonglobalEvent = uiNewEvent(&opts);
	testingTDefer(t, deferEventFree, p.nonglobalEvent);
	p.eventPlaceholder = p.globalEvent;
	p.handlerPlaceholder = dummyHandler;
	p.nonNullSender = &p;
	p.firingEvent = p.globalEvent;
	p.eventWithHandlers = uiNewEvent(&opts);
	// TODO properly free this
	uiEventAddHandler(p.eventWithHandlers, dummyHandler, &p, &p);

	for (i = 0; checkErrorCases[i].name != NULL; i++)
		checkProgrammerError(t, checkErrorCases[i].name, checkErrorCases[i].f, &p, checkErrorCases[i].msgWant);
}

// TODO check deleting each internal event
