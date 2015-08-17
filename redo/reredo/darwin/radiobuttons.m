// 14 august 2015
#import "uipriv_darwin.h"

struct uiRadioButtons {
	uiDarwinControl c;
	NSTextField *dummy;
};

uiDarwinDefineControl(
	uiRadioButtons,						// type name
	uiRadioButtonsType,						// type function
	dummy								// handle
)

void uiRadioButtonsAppend(uiRadioButtons *r, const char *text)
{
	// TODO
//TODO	uiControlQueueResize(uiControl(r));
}

uiRadioButtons *uiNewRadioButtons(void)
{
	uiRadioButtons *r;

	r = (uiRadioButtons *) uiNewControl(uiRadioButtonsType());

	r->dummy = [[NSTextField alloc] initWithFrame:NSZeroRect];
	[r->dummy setStringValue:@"TODO uiRadioButtons not implemented"];

	uiDarwinFinishNewControl(r, uiRadioButtons);

	return r;
}
