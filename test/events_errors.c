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

#define checkEventErrorCat(a, b) a ## b

// First, define the firing-case handlers.
#define checkErrorCase(call, msgWant)
#define checkEventErrorCaseWhileFiringFull(line, call, msgWant) \
	static void checkEventErrorCat(eventHandler, line)(void *sender, void *args, void *data) \
	{ \
		struct checkErrorParams *p = (struct checkErrorParams *) data; \
		(void) p; /* in the event call does not use this */ \
		call; \
	} \
	static void checkEventErrorCat(runEventCheck, line)(struct checkErrorParams *p) \
	{ \
		int id; \
		id = uiEventAddHandler(p->firingEvent, checkEventErrorCat(eventHandler, line), NULL, p); \
		uiEventFire(p->firingEvent, NULL, NULL); \
		uiEventDeleteHandler(p->firingEvent, id); \
	}
#define checkEventErrorCaseWhileFiring(call, msgWant) checkEventErrorCaseWhileFiringFull(__LINE__, call, msgWant)
#include "event_errors.h"
#undef checkEventErrorCaseWhileFiring
#undef checkEventErrorCaseWhileFiringFull
#undef checkErrorCase

#define checkEventErrorCaseWhileFiringFull(line, call, msgWant) checkErrorCaseFull(line, #call, checkEventErrorCat(runEventCheck, line), msgWant)
#define checkEventErrorCaseWhileFiring(call, msgWant) checkEventErrorCaseWhileFiringFull(__LINE__, call, msgWant)
#define checkErrorHeader "events_errors.h"
#include "events.h"
#undef checkErrorHeader
#undef checkEventErrorCaseWhileFiring
#undef checkEventErrorCaseWhileFiringFull

#undef checkEventErrorCat

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
