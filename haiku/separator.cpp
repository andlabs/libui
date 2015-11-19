// 18 november 2015
#include "uipriv_haiku.hpp"

struct uiSeparator {
	uiHaikuControl c;
	BStringView *dummy;
};

uiHaikuDefineControl(
	uiSeparator,							// type name
	uiSeparatorType,						// type function
	dummy								// handle
)

uiSeparator *uiNewHorizontalSeparator(void)
{
	uiSeparator *s;

	s = (uiSeparator *) uiNewControl(uiSeparatorType());

	s->dummy = new BStringView(BRect(0, 0, 1, 1), NULL,
		"TODO uiSeparator not implemented");

	uiHaikuFinishNewControl(s, uiSeparator);

	return s;
}
