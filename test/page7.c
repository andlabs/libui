// 13 october 2015
#include "test.h"

uiBox *makePage7(void)
{
	uiBox *page7;
	uiGroup *group;
	uiBox *box2;

	page7 = newHorizontalBox();

	group = makePage7a();
	uiBoxAppend(page7, uiControl(group), 1);

	box2 = newVerticalBox();
	uiBoxAppend(page7, uiControl(box2), 1);

	group = makePage7b();
	uiBoxAppend(box2, uiControl(group), 1);

	group = makePage7c();
	uiBoxAppend(box2, uiControl(group), 1);

	return page7;
}
