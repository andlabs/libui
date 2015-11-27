// 26 november 2015
#include "uipriv_winforms.hpp"

struct uiProgressBar {
	uiWindowsControl c;
	DUMMY dummy;
};

uiWindowsDefineControl(
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

	p->dummy = mkdummy(L"uiProgressBar");

	uiWindowsFinishNewControl(p, uiProgressBar, dummy);

	return p;
}
