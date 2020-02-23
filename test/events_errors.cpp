// 10 june 2019
#include "test.h"

// TODO consider rewriting this in C

static void dummyHandler(void *sender, void *args, void *data)
{
	// do nothing
}

static void checkWithGlobalEvent(void (*f)(uiEvent *e))
{
	uiEvent *e;
	uiEventOptions opts;

	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = true;
	e = uiNewEvent(&opts);
	(*f)(e);
	uiEventFree(e);
}

static void checkWithNonglobalEvent(void (*f)(uiEvent *e))
{
	uiEvent *e;
	uiEventOptions opts;

	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = false;
	e = uiNewEvent(&opts);
	(*f)(e);
	uiEventFree(e);
}

struct checkWhileFiringParams {
	uiEvent *e;
	void (*f)(uiEvent *e);
};

static void checkWhileFiringHandler(void *sender, void *args, void *data)
{
	struct checkWhileFiringParams *p = (struct checkWhileFiringParams *) data;

	(*(p->f))(p->e);
}

static void checkWhileFiring(void (*f)(uiEvent *e))
{
	uiEvent *e;
	uiEventOptions opts;
	struct checkWhileFiringParams p;
	int handler;

	memset(&opts, 0, sizeof (uiEventOptions));
	opts.Size = sizeof (uiEventOptions);
	opts.Global = true;
	e = uiNewEvent(&opts);
	memset(&p, 0, sizeof (struct checkWhileFiringParams));
	p.e = e;
	p.f = f;
	handler = uiEventAddHandler(e, checkWhileFiringHandler, NULL, &p);
	uiEventFire(e, NULL, NULL);
	uiEventDeleteHandler(e, handler);
	uiEventFree(e);
}

static const struct checkErrorCase cases[] = {
	{
		"uiNewEvent() with NULL uiEventOptions",
		[](void) {
			uiNewEvent(NULL);
		},
		"uiNewEvent(): invalid null pointer for uiEventOptions",
	},
	{
		"uiNewEvent() with bad uiEventOptions size",
		[](void) {
			uiEventOptions opts;
	
			memset(&opts, 0, sizeof (uiEventOptions));
			opts.Size = 1;
			uiNewEvent(&opts);
		},
		"uiNewEvent(): wrong size 1 for uiEventOptions",
	},
	
	{
		"uiEventFree() with NULL uiEvent",
		[](void) {
			uiEventFree(NULL);
		},
		"uiEventFree(): invalid null pointer for uiEvent",
	},
#define checkEventFreeInternal(which) \
	{ \
		"uiEventFree() called on " #which, \
		[](void) { uiEventFree(which); }, \
		"uiEventFree(): can't free a libui-provided event", \
	}
	checkEventFreeInternal(uiControlOnFree()),
	{
		"uiEventFree() while said event is firing",
		[](void) {
			checkWhileFiring([](uiEvent *e) {
				uiEventFree(e);
			});
		},
		"uiEventFree(): can't change a uiEvent while it is firing",
	},
	{
		"uiEventFree() while handlers registered",
		[](void) {
			checkWithGlobalEvent([](uiEvent *e) {
				int handler;
	
				handler = uiEventAddHandler(e, dummyHandler, NULL, NULL);
				uiEventFree(e);
				uiEventDeleteHandler(e, handler);
			});
		},
		"uiEventFree(): can't free event that still has handlers registered",
	},
	
	{
		"uiEventAddHandler() with NULL uiEvent",
		[](void) {
			uiEventAddHandler(NULL, NULL, NULL, NULL);
		},
		"uiEventAddHandler(): invalid null pointer for uiEvent",
	},
	{
		"uiEventAddHandler() while that event is firing",
		[](void) {
			checkWhileFiring([](uiEvent *e) {
				uiEventAddHandler(e, dummyHandler, NULL, NULL);
			});
		},
		"uiEventAddHandler(): can't change a uiEvent while it is firing",
	},
	{
		"uiEventAddHandler() with a NULL uiEventHandler",
		[](void) {
			checkWithGlobalEvent([](uiEvent *e) {
				uiEventAddHandler(e, NULL, NULL, NULL);
			});
		},
		"uiEventAddHandler(): invalid null pointer for uiEventHandler",
	},
	{
		"uiEventAddHandler() with a non-NULL sender on a global uiEvent",
		[](void) {
			checkWithGlobalEvent([](uiEvent *e) {
				uiEventAddHandler(e, dummyHandler, e, NULL);
			});
		},
		"uiEventAddHandler(): can't use a non-NULL sender with a global event",
	},
	{
		"uiEventAddHandler() with a NULL sender on a non-global uiEvent",
		[](void) {
			checkWithNonglobalEvent([](uiEvent *e) {
				uiEventAddHandler(e, dummyHandler, NULL, NULL);
			});
		},
		"uiEventAddHandler(): can't use a NULL sender with a non-global event",
	},
	
	{
		"uiEventDeleteHandler() with NULL uiEvent",
		[](void) {
			uiEventDeleteHandler(NULL, 5);
		},
		"uiEventDeleteHandler(): invalid null pointer for uiEvent",
	},
	{
		"uiEventDeleteHandler() while that event is firing",
		[](void) {
			checkWhileFiring([](uiEvent *e) {
				uiEventDeleteHandler(e, 5);
			});
		},
		"uiEventDeleteHandler(): can't change a uiEvent while it is firing",
	},
	{
		"uiEventDeleteHandler() with invalid handler",
		[](void) {
			checkWithGlobalEvent([](uiEvent *e) {
				uiEventDeleteHandler(e, 5);
			});
		},
		"uiEventDeleteHandler(): event handler 5 not found",
	},
	
	{
		"uiEventFire() on a NULL uiEvent",
		[](void) {
			uiEventFire(NULL, NULL, NULL);
		},
		"uiEventFire(): invalid null pointer for uiEvent",
	},
	{
		"uiEventFire() on an event that is firing",
		[](void) {
			checkWhileFiring([](uiEvent *e) {
				uiEventFire(e, NULL, NULL);
			});
		},
		"uiEventFire(): can't recursively fire a uiEvent",
	},
	{
		"uiEventFire() with a non-NULL sender on a global event",
		[](void) {
			checkWithGlobalEvent([](uiEvent *e) {
				uiEventFire(e, e, NULL);
			});
		},
		"uiEventFire(): can't use a non-NULL sender with a global event",
	},
	{
		"uiEventFire() with a NULL sender on a non-global event",
		[](void) {
			checkWithNonglobalEvent([](uiEvent *e) {
				uiEventFire(e, NULL, NULL);
			});
		},
		"uiEventFire(): can't use a NULL sender with a non-global event",
	},
	
	{
		"uiEventHandlerBlocked() with a NULL uiEvent",
		[](void) {
			uiEventHandlerBlocked(NULL, 5);
		},
		"uiEventHandlerBlocked(): invalid null pointer for uiEvent",
	},
	{
		"uiEventHandlerBlocked() with an invalid handler",
		[](void) {
			checkWithGlobalEvent([](uiEvent *e) {
				uiEventHandlerBlocked(e, 5);
			});
		},
		"uiEventHandlerBlocked(): event handler 5 not found",
	},
	
	{
		"uiEventSetHandlerBlocked() with a NULL uiEvent",
		[](void) {
			uiEventSetHandlerBlocked(NULL, 5, false);
		},
		"uiEventSetHandlerBlocked(): invalid null pointer for uiEvent",
	},
	{
		"uiEventSetHandlerBlocked() while the given event is firing",
		[](void) {
			checkWhileFiring([](uiEvent *e) {
				uiEventSetHandlerBlocked(e, 5, false);
			});
		},
		"uiEventSetHandlerBlocked(): can't change a uiEvent while it is firing",
	},
	{
		"uiEventSetHandlerBlocked() with invalid handler",
		[](void) {
			checkWithGlobalEvent([](uiEvent *e) {
				uiEventSetHandlerBlocked(e, 5, false);
			});
		},
		"uiEventSetHandlerBlocked(): event handler 5 not found",
	},
	
	{
		"uiEventInvalidateSender() with NULL uiEvent",
		[](void) {
			uiEventInvalidateSender(NULL, NULL);
		},
		"uiEventInvalidateSender(): invalid null pointer for uiEvent",
	},
	{
		"uiEventInvalidateSender() while that event is firing",
		[](void) {
			checkWhileFiring([](uiEvent *e) {
				uiEventInvalidateSender(e, NULL);
			});
		},
		"uiEventInvalidateSender(): can't change a uiEvent while it is firing",
	},
	{
		"uiEventInvalidateSender() with a global event",
		[](void) {
			checkWithGlobalEvent([](uiEvent *e) {
				uiEventInvalidateSender(e, NULL);
			});
		},
		"uiEventInvalidateSender(): can't invalidate a global event",
	},
	{
		"uiEventInvalidateSender() with a NULL sender on a non-global event",
		[](void) {
			checkWithNonglobalEvent([](uiEvent *e) {
				uiEventInvalidateSender(e, NULL);
			});
		},
		"uiEventInvalidateSender(): can't use a NULL sender with a non-global event",
	},
	{ NULL, NULL, NULL },
};

Test(EventErrors)
{
	checkProgrammerErrors(cases);
}
