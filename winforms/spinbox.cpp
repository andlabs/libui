// 18 november 2015
#include "uipriv_winforms.hpp"

struct uiSpinbox {
	uiWindowsControl c;
	DUMMY dummy;
};

uiWindowsDefineControl(
	uiSpinbox,							// type name
	uiSpinboxType,						// type function
	dummy								// handle
)

intmax_t uiSpinboxValue(uiSpinbox *s)
{
	// TODO
	return 0;
}

void uiSpinboxSetValue(uiSpinbox *s, intmax_t value)
{
	// TODO
}

void uiSpinboxOnChanged(uiSpinbox *s, void (*f)(uiSpinbox *s, void *data), void *data)
{
	// TODO
}

uiSpinbox *uiNewSpinbox(intmax_t min, intmax_t max)
{
	uiSpinbox *s;

	s = (uiSpinbox *) uiNewControl(uiSpinboxType());

	s->dummy = mkdummy(L"uiSpinbox");

	uiWindowsFinishNewControl(s, uiSpinbox, dummy);

	return s;
}
