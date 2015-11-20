// 18 november 2015
#include "uipriv_haiku.hpp"

struct uiArea {
	uiHaikuControl c;
	BStringView *dummy;
};

uiHaikuDefineControl(
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

	a->dummy = new BStringView(NULL, "TODO uiArea not implemented");

	uiHaikuFinishNewControl(a, uiArea);

	return a;
}
