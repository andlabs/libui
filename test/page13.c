// 28 may 2016
#include "test.h"

uiBox *makePage13(void)
{
	uiBox *page13;
	uiRadioButtons *rb;
	uiButton *b;

	page13 = newVerticalBox();

	rb = uiNewRadioButtons();
	uiRadioButtonsAppend(rb, "Item 1");
	uiRadioButtonsAppend(rb, "Item 2");
	uiRadioButtonsAppend(rb, "Item 3");
	uiBoxAppend(page13, uiControl(rb), 0);

	rb = uiNewRadioButtons();
	uiRadioButtonsAppend(rb, "Item A");
	uiRadioButtonsAppend(rb, "Item B");
	uiBoxAppend(page13, uiControl(rb), 0);

	b = uiNewButton("Horizontal");
	uiBoxAppend(page13, uiControl(b), 0);

	b = uiNewButton("Vertical");
	uiBoxAppend(page13, uiControl(b), 0);

	return page13;
}
