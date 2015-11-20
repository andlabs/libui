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

	s->dummy = new BStringView(NULL, "TODO uiSeparator not implemented");

	uiHaikuFinishNewControl(s, uiSeparator);

	return s;
}
