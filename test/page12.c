// 22 may 2016
#include "test.h"

// TODO add buttons for event testing and Append scroll/selection changing.

uiBox *makePage12(void)
{
	uiBox *page12;

	page12 = newHorizontalBox();

	uiBoxAppend(page12, uiControl(uiNewMultilineEntry()), 1);
	uiBoxAppend(page12, uiControl(uiNewNonWrappingMultilineEntry()), 1);

	return page12;
}
