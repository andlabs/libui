// 11 june 2015
#include "uipriv_darwin.h"

struct radiobuttons {
	uiRadioButtons r;
};

uiDefineControlType(uiRadioButtons, uiTypeRadioButtons, struct radiobuttons)

static uintptr_t radiobuttonsHandle(uiControl *c)
{
	return 0;
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

	r = (struct radiobuttons *) MAKE_CONTROL_INSTANCE(uiTypeRadioButtons());

	PUT_CODE_HERE;

	uiControl(r)->Handle = radiobuttonsHandle;

	uiRadioButtons(r)->Append = radiobuttonsAppend;

	return uiRadioButtons(r);
}
