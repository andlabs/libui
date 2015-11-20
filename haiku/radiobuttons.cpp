// 18 november 2015
#include "uipriv_haiku.hpp"

struct uiRadioButtons {
	uiHaikuControl c;
	BStringView *dummy;
};

uiHaikuDefineControl(
	uiRadioButtons,						// type name
	uiRadioButtonsType,						// type function
	dummy								// handle
)

void uiRadioButtonsAppend(uiRadioButtons *r, const char *text)
{
	// TODO
}

uiRadioButtons *uiNewRadioButtons(void)
{
	uiRadioButtons *r;

	r = (uiRadioButtons *) uiNewControl(uiRadioButtonsType());

	r->dummy = new BStringView(NULL, "TODO uiRadioButtons not implemented");

	uiHaikuFinishNewControl(r, uiRadioButtons);

	return r;
}
