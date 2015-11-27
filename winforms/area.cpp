// 26 november 2015
#include "uipriv_winforms.hpp"

struct uiArea {
	uiWindowsControl c;
	DUMMY dummy;
	uiAreaHandler *ah;
};

uiWindowsDefineControl(
	uiArea,								// type name
	uiAreaType,							// type function
	dummy								// handle
)

void uiAreaUpdateScroll(uiArea *a)
{
	// TODO
}

void uiAreaQueueRedrawAll(uiArea *a)
{
	// TODO
}

uiArea *uiNewArea(uiAreaHandler *ah)
{
	uiArea *a;

	a = (uiArea *) uiNewControl(uiAreaType());

	a->ah = ah;

	a->dummy = mkdummy(L"uiArea");

	uiWindowsFinishNewControl(a, uiArea, dummy);

	return a;
}
