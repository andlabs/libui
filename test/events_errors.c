// 12 june 2019
#include "test.h"

static void dummyHandler(void *sender, void *args, void *data)
{
	// do nothing
}

// TODO deduplicate this with events.c
static void deferEventFree(testingT *t, void *data)
{
	uiEventFree((uiEvent *) data);
}

struct checkEventErrorsParams {
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

// TODO clean up these macros
#define checkCat(a, b) a ## b
#define checkErrorCaseFull(line, call, msgWant) \
	static void checkCat(doCheck, line)(void *data) \
	{ \
		struct checkEventErrorsParams *p = (struct checkEventErrorsParams *) data; \
		(void) p; /* in the event call does not use this */ \
		call; \
	}
#define checkErrorCase(call, msgWant) checkErrorCaseFull(__LINE__, call, msgWant)
#define checkErrorCaseWhileFiringFull(line, call, msgWant) \
	static void checkCat(eventHandler, line)(void *sender, void *args, void *data) \
	{ \
		struct checkEventErrorsParams *p = (struct checkEventErrorsParams *) data; \
		(void) p; /* in the event call does not use this */ \
		call; \
	} \
	static void checkCat(doCheck, line)(void *data) \
	{ \
		struct checkEventErrorsParams *p = (struct checkEventErrorsParams *) data; \
		int id; \
		id = uiEventAddHandler(p->firingEvent, checkCat(eventHandler, line), NULL, p); \
		uiEventFire(p->firingEvent, NULL, NULL); \
		uiEventDeleteHandler(p->firingEvent, id); \
	}
#define checkErrorCaseWhileFiring(call, msgWant) checkErrorCaseWhileFiringFull(__LINE__, call, msgWant)
#include "events_errors.h"
#undef checkErrorCaseWhileFiringFull
#undef checkErrorCaseWhileFiring
#undef checkErrorCaseFull
#undef checkErrorCase

static const struct {
	const char *name;
	void (*f)(void *data);
	const char *msgWant;
} eventErrorCases[] = {
#define checkErrorCaseFull(line, callstr, msgWant) { callstr, checkCat(doCheck, line), msgWant },
#define checkErrorCase(call, msgWant) checkErrorCaseFull(__LINE__, #call, msgWant)
#define checkErrorCaseWhileFiring(call, msgWant) checkErrorCaseFull(__LINE__, #call, msgWant)
#include "events_errors.h"
#undef checkErrorCaseWhileFiring
#undef checkErrorCase
#undef checkErrorCaseFull
#undef checkCat
	{ NULL, NULL, NULL, },
};

testingTest(EventErrors)
{
	struct checkEventErrorsParams p;
	uiEventOptions opts;
	size_t i;

	memset(&p, 0, sizeof (struct checkEventErrorsParams));
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

	for (i = 0; eventErrorCases[i].name != NULL; i++)
		checkProgrammerError(t, eventErrorCases[i].name, eventErrorCases[i].f, &p, eventErrorCases[i].msgWant);
}

// TODO check deleting each internal event
