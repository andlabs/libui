// 18 november 2015
#include "uipriv_haiku.hpp"

struct uiSpinbox {
	uiHaikuControl c;
	BStringView *dummy;
};

uiHaikuDefineControl(
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

	s->dummy = new BStringView(NULL, "TODO uiSpinbox not implemented");

	uiHaikuFinishNewControl(s, uiSpinbox);

	return s;
}
