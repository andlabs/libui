// 14 august 2015
#import "uipriv_darwin.h"

// TODOs:
// - 10.8: increasing value animates just like with Aero

struct uiProgressBar {
	uiDarwinControl c;
	NSProgressIndicator *pi;
};

uiDarwinDefineControl(
	uiProgressBar,							// type name
	uiProgressBarType,						// type function
	pi									// handle
)

void uiProgressBarSetValue(uiProgressBar *p, int value)
{
	if (value < 0 || value > 100)
		complain("value %d out of range in progressbarSetValue()", value);
	[p->pi setDoubleValue:((double) value)];
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
