// 18 november 2015
#include "uipriv_wpf.hpp"

struct uiSeparator {
	uiWindowsControl c;
	DUMMY dummy;
};

uiWindowsDefineControl(
	uiSeparator,							// type name
	uiSeparatorType,						// type function
	dummy								// handle
)

uiSeparator *uiNewHorizontalSeparator(void)
{
	uiSeparator *s;

	s = (uiSeparator *) uiNewControl(uiSeparatorType());

	s->dummy = mkdummy(L"uiSeparator");

	uiWindowsFinishNewControl(s, uiSeparator, dummy);

	return s;
}
