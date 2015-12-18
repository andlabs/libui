// 13 october 2015
#include "test.h"

uiBox *makePage7(void)
{
	uiBox *page7;
	uiGroup *group;

	page7 = newHorizontalBox();

	group = makePage7a();
	uiBoxAppend(page7, uiControl(group), 1);

	return page7;
}
