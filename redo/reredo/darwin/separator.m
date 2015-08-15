// 14 august 2015
#import "uipriv_darwin.h"

// TODO sizing
// namely, figure out how horizontal and vertical work

struct uiSeparator {
	uiDarwinControl c;
	NSBox *box;
};

uiDarwinDefineControl(
	uiSeparator,							// type name
	uiSeparatorType,						// type function
	box									// handle
)

uiSeparator *uiNewHorizontalSeparator(void)
{
	uiSeparator *s;

	s = (uiSeparator *) uiNewControl(uiSeparatorType());

	s->box = [[NSBox alloc] initWithFrame:NSZeroRect];
	[s->box setBoxType:NSBoxSeparator];
//TODO	[s->box setBorderType:TODO];
	[s->box setTransparent:NO];
	[s->box setTitlePosition:NSNoTitle];

	uiDarwinFinishNewControl(s, uiSeparator);

	return s;
}
