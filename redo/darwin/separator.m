// 11 june 2015
#include "uipriv_darwin.h"

// TODO sizing

struct separator {
	uiSeparator s;
	NSBox *box;
};

uiDefineControlType(uiSeparator, uiTypeSeparator, struct separator)

static uintptr_t separatorHandle(uiControl *c)
{
	struct separator *s = (struct separator *) c;

	return (uintptr_t) (s->box);
}

uiSeparator *uiNewHorizontalSeparator(void)
{
	struct separator *s;

	s = (struct separator *) MAKE_CONTROL_INSTANCE(uiTypeSeparator());

	s->box = [[NSBox alloc] initWithFrame:NSZeroRect];
	[s->box setBoxType:NSBoxSeparator];
	[s->box setBorderType:TODO];
	[s->box setTransparent:NO];
	[s->box setTitlePosition:NSNoTitle];

	uiDarwinMakeSingleViewControl(uiControl(s), s->box, NO);

	uiControl(s)->Handle = separatorHandle;

	return uiSeparator(s);
}
