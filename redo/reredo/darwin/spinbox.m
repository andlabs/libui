// 14 august 2015
#import "uipriv_darwin.h"

struct uiSpinbox {
	uiDarwinControl c;
	NSTextField *dummy;
	void (*onChanged)(uiSpinbox *, void *);
	void *onChangedData;
};

uiDarwinDefineControl(
	uiSpinbox,							// type name
	uiSpinboxType,						// type function
	dummy								// handle
)

intmax_t uiSpinboxValue(uiSpinbox *s)
{
	return PUT_CODE_HERE;
}

void uiSpinboxSetValue(uiSpinbox *s, intmax_t value)
{
	// TODO
}

void uiSpinboxOnChanged(uiSpinbox *s, void (*f)(uiSpinbox *, void *), void *data)
{
	s->onChanged = f;
	s->onChangedData = data;
}

static void defaultOnChanged(uiSpinbox *s, void *data)
{
	// do nothing
}

uiSpinbox *uiNewSpinbox(intmax_t min, intmax_t max)
{
	uiSpinbox *s;

	s = (uiSpinbox *) uiNewControl(uiSpinboxType());

	s->dummy = [[NSTextField alloc] initWithFrame:NSZeroRect];
	[s->dummy setStringValue:@"TODO uiSpinbox not implemented"];

	uiSpinboxOnChanged(s, defaultOnChanged, NULL);

	uiDarwinFinishNewControl(s, uiSpinbox);

	return s;
}
