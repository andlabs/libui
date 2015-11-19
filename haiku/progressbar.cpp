// 18 november 2015
#include "uipriv_haiku.hpp"

struct uiProgressBar {
	uiHaikuControl c;
	BStringView *dummy;
};

uiHaikuDefineControl(
	uiProgressBar,							// type name
	uiProgressBarType,						// type function
	dummy								// handle
)

void uiProgressBarSetValue(uiProgressBar *p, int n)
{
	// TODO
}

uiProgressBar *uiNewProgressBar(void)
{
	uiProgressBar *p;

	p = (uiProgressBar *) uiNewControl(uiProgressBarType());

	p->dummy = new BStringView(BRect(0, 0, 1, 1), NULL,
		"TODO uiProgressBar not implemented");

	uiHaikuFinishNewControl(p, uiProgressBar);

	return p;
}
