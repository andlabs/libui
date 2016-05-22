// 22 may 2016
#include "test.h"

uiBox *makePage12(void)
{
	uiBox *page12;

	page12 = newHorizontalBox();

	uiBoxAppend(page12, uiControl(uiNewMultilineEntry()), 1);
	uiBoxAppend(page12, uiControl(uiNewNonWrappingMultilineEntry()), 1);

	return page12;
}
