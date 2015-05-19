// 19 may 2015
#include "test.h"

static uiSpinbox *spinbox;

uiBox *makePage4(void)
{
	uiBox *page4;

	page4 = newVerticalBox();

	spinbox = uiNewSpinbox();
	uiBoxAppend(page4, uiControl(spinbox), 0);

	return page4;
}
