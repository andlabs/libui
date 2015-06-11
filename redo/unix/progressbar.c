// 11 june 2015
#include "uipriv_unix.h"

struct progressbar {
	uiProgressBar p;
	GtkWidget *widget;
};

uiDefineControlType(uiProgressBar, uiTypeProgressBar, struct progressbar)

static uintptr_t progressbarHandle(uiControl *c)
{
	struct progressbar *p = (struct progressbar *) c;

	return (uintptr_t) (p->widget);
}

static void progressbarSetValue(uiProgressBar *pp, int value)
{
	struct progressbar *p = (struct progressbar *) pp;

	if (value < 0 || value > 100)
		complain("value %d out of range in progressbarSetValue()", value);
	PUT_CODE_HERE;
}

uiProgressBar *uiNewProgressBar(void)
{
	struct progressbar *p;

	p = (struct progressbar *) MAKE_CONTROL_INSTANCE(uiTypeProgressBar());

	PUT_CODE_HERE;

	uiControl(p)->Handle = progressbarHandle;

	uiProgressBar(p)->SetValue = progressbarSetValue;

	return uiProgressBar(p);
}
