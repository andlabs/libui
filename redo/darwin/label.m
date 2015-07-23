// 11 june 2015
#include "uipriv_darwin.h"

struct label {
	uiLabel l;
	NSTextField *label;
};

uiDefineControlType(uiLabel, uiTypeLabel, struct label)

static uintptr_t labelHandle(uiControl *c)
{
	struct label *l = (struct label *) c;

	return (uintptr_t) (l->label);
}

static char *labelText(uiLabel *ll)
{
	struct label *l = (struct label *) ll;

	return uiDarwinNSStringToText([l->label stringValue]);
}

static void labelSetText(uiLabel *ll, const char *text)
{
	struct label *l = (struct label *) ll;

	[l->label setStringValue:toNSString(text)];
	// changing the text might necessitate a change in the label's size
	uiControlQueueResize(uiControl(l));
}

uiLabel *uiNewLabel(const char *text)
{
	struct label *l;

	l = (struct label *) uiNewControl(uiTypeLabel());

	l->label = [[NSTextField alloc] initWithFrame:NSZeroRect];

	[l->label setStringValue:toNSString(text)];
	[l->label setEditable:NO];
	[l->label setSelectable:NO];
	[l->label setDrawsBackground:NO];
	finishNewTextField(uiControl(l), l->label, NO);

	uiControl(l)->Handle = labelHandle;

	uiLabel(l)->Text = labelText;
	uiLabel(l)->SetText = labelSetText;

	return uiLabel(l);
}
