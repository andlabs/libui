// 11 june 2015
#include "uipriv_darwin.h"

struct progressbar {
	uiProgressBar p;
	NSProgressIndicator *pi;
};

uiDefineControlType(uiProgressBar, uiTypeProgressBar, struct progressbar)

static uintptr_t progressbarHandle(uiControl *c)
{
	struct progressbar *p = (struct progressbar *) c;

	return (uintptr_t) (p->pi);
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

	p->pi = [[NSProgressIndicator alloc] initWithFrame:NSZeroRect];
NSLog(@"TEST thread %d tint %d stopped %d", (int) [p->pi usesThreadedAnimation], [p->pi controlTint], (int) [p->pi displayedWhenStopped]);
	[p->pi setControlSize:NSRegularControlSize];
	[p->pi setBezeled:YES];
	[p->pi setStyle:NSProgressIndicatorBarStyle];
	[p->pi setIndeterminate:NO];
NSLog(@"TEST thread %d tint %d stopped %d", (int) [p->pi usesThreadedAnimation], [p->pi controlTint], (int) [p->pi displayedWhenStopped]);
	uiDarwinMakeSingleViewControl(uiControl(p), p->pi, NO);

	uiControl(p)->Handle = progressbarHandle;

	uiProgressBar(p)->SetValue = progressbarSetValue;

	return uiProgressBar(p);
}
