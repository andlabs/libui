// 12 june 2019
#include "test.h"

// Usage:
// struct checkErrorParams {
// 	// ...
// };
// #define checkErrorHeader "header.h"
// #include "errors.h"
// testingTest(TestName)
// {
// 	struct checkErrorParams p = ...;
// 	checkErrors(t, checkErrorCases, &p);
// }
// Feel free to redefine checkErrorParams and checkErrorCases as necessary.

#define checkErrorCat(a, b) a ## b
#define checkErrorCase(call, msgWant) checkErrorCaseFull(__LINE__, call, msgWant)

#define checkErrorCaseFull(line, call, msgWant) \
	static void checkErrorCat(doCheck, line)(void *data) \
	{ \
		struct checkErrorParams *p = (struct checkErrorParams *) data; \
		(void) p; /* in the event call does not use this */ \
		call; \
	}
#include checkErrorHeader
#undef checkErrorCaseFull

static const struct checkErrorCase checkErrorCases[] = {
#define checkErrorCaseFull(line, callstr, msgWant) { callstr, checkErrorCat(doCheck, line), msgWant },
#include checkErrorHeader
#undef checkErrorCaseFull
	{ NULL, NULL, NULL, },
};

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

#undef checkErrorCase
#undef checkErrorCat
