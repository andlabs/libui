// 26 november 2015
#include "uipriv_winforms.hpp"

struct uiRadioButtons {
	uiWindowsControl c;
	DUMMY dummy;
};

uiWindowsDefineControl(
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

	r->dummy = mkdummy(L"uiRadioButtons");

	uiWindowsFinishNewControl(r, uiRadioButtons, dummy);

	return r;
}
