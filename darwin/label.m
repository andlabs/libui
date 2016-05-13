// 14 august 2015
#import "uipriv_darwin.h"

struct uiLabel {
	uiDarwinControl c;
	NSTextField *textfield;
};

uiDarwinControlAllDefaults(uiLabel, textfield)

char *uiLabelText(uiLabel *l)
{
	return uiDarwinNSStringToText([l->textfield stringValue]);
}

void uiLabelSetText(uiLabel *l, const char *text)
{
	[l->textfield setStringValue:toNSString(text)];
}

uiLabel *uiNewLabel(const char *text)
{
	uiLabel *l;

	uiDarwinNewControl(uiLabel, l);

	l->textfield = [[NSTextField alloc] initWithFrame:NSZeroRect];
	[l->textfield setStringValue:toNSString(text)];
	[l->textfield setEditable:NO];
	[l->textfield setSelectable:NO];
	[l->textfield setDrawsBackground:NO];
	finishNewTextField(l->textfield, NO);

	return l;
}
