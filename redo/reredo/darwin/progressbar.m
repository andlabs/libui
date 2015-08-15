// 14 august 2015
#import "uipriv_darwin.h"

struct uiProgressBar {
	uiDarwinControl c;
	NSProgressIndicator *pi;
};

uiDarwinDefineControl(
	uiProgressBar,							// type name
	uiProgressBarType,						// type function
	pi									// handle
)

void uiProgressbarSetValue(uiProgressBar *p, int value)
{
	if (value < 0 || value > 100)
		complain("value %d out of range in progressbarSetValue()", value);
	// TODO
}

uiProgressBar *uiNewProgressBar(void)
{
	uiProgressBar *p;

	p = (uiProgressBar *) uiNewControl(uiProgressBarType());

	p->pi = [[NSProgressIndicator alloc] initWithFrame:NSZeroRect];
	[p->pi setControlSize:NSRegularControlSize];
	[p->pi setBezeled:YES];
	[p->pi setStyle:NSProgressIndicatorBarStyle];
	[p->pi setIndeterminate:NO];

	uiDarwinFinishNewControl(p, uiProgressBar);

	return p;
}
