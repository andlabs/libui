// 11 june 2015
#include "uipriv_darwin.h"

struct radiobuttons {
	uiRadioButtons r;
	NSTextField *dummy;
};

uiDefineControlType(uiRadioButtons, uiTypeRadioButtons, struct radiobuttons)

static uintptr_t radiobuttonsHandle(uiControl *c)
{
	struct radiobuttons *r = (struct radiobuttons *) c;

	return (uintptr_t) (r->dummy);
}

static void radiobuttonsAppend(uiRadioButtons *rr, const char *text)
{
	struct radiobuttons *r = (struct radiobuttons *) rr;

	PUT_CODE_HERE;
	uiControlQueueResize(uiControl(r));
}

uiRadioButtons *uiNewRadioButtons(void)
{
	struct radiobuttons *r;

	r = (struct radiobuttons *) uiNewControl(uiTypeRadioButtons());

	r->dummy = [[NSTextField alloc] initWithFrame:NSZeroRect];
	[r->dummy setStringValue:@"TODO uiRadioButtons not implemented"];
	uiDarwinMakeSingleViewControl(uiControl(r), r->dummy, YES);

	uiControl(r)->Handle = radiobuttonsHandle;

	uiRadioButtons(r)->Append = radiobuttonsAppend;

	return uiRadioButtons(r);
}
