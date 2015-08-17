// 14 august 2015
#import "uipriv_darwin.h"

struct uiLabel {
	uiDarwinControl c;
	NSTextField *textfield;		// TODO rename to label?
};

uiDarwinDefineControl(
	uiLabel,								// type name
	uiLabelType,							// type function
	textfield								// handle
)

char *uiLabelText(uiLabel *ll)
{
	return uiDarwinNSStringToText([l->textfield stringValue]);
}

void uiLabelSetText(uiLabel *l, const char *text)
{
	[l->textfield setStringValue:toNSString(text)];
	// changing the text might necessitate a change in the label's size
//TODO	uiControlQueueResize(uiControl(l));
}

uiLabel *uiNewLabel(const char *text)
{
	uiLabel *l;

	l = (uiLabel *) uiNewControl(uiLabelType());

	l->textfield = [[NSTextField alloc] initWithFrame:NSZeroRect];
	[l->textfield setStringValue:toNSString(text)];
	[l->textfield setEditable:NO];
	[l->textfield setSelectable:NO];
	[l->textfield setDrawsBackground:NO];
	finishNewTextField(l->textfield, NO);

	uiDarwinFinishNewControl(l, uiLabel);

	return l;
}
