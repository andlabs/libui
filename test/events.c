// 18 may 2019
#include "test.h"

struct handler {
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

#define basicTest(name, whichGlobal, whichSender, whichArgs) \
	testingTest(name) \
	{ \
		uiEvent *e; \
		uiEventOptions opts; \
		struct handler h; \
		memset(&opts, 0, sizeof (uiEventOptions)); \
		opts.Size = sizeof (uiEventOptions); \
		opts.Global = whichGlobal; \
		e = uiNewEvent(&opts); \
		memset(&h, 0, sizeof (struct handler)); \
		uiEventAddHandler(e, handler, whichSender, &h); \
		uiEventFire(e, whichSender, whichArgs); \
		if (!h.run) \
			testingTErrorf(t, "handler not run"); \
		if (h.sender != whichSender) \
			diff(t, "incorrect sender seen by handler", \
				"%p", h.sender, whichSender); \
		if (h.args != whichArgs) \
			diff(t, "incorrect args seen by handler", \
				"%p", h.args, whichArgs); \
	}
basicTest(BasicEvents_Global_Args, true, NULL, &h)
basicTest(BasicEvents_Global_NoArgs, true, NULL, NULL)
basicTest(BasicEvents_Nonglobal_Args, false, &opts, &h)
basicTest(BasicEvents_Nonglobal_NoArgs, false, &opts, NULL)

testingTest(AddDeleteEventHandler)
{
}

testingTest(EventErrors)
{
	// TODO
}
