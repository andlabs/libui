// 19 may 2015
#include "test.h"

static uiSpinbox *spinbox;

#define CHANGED(what) \
	static void on ## what ## Changed(ui ## what *this, void *data) \
	{ \
		printf("on %s changed\n", #what); \
	}
CHANGED(Spinbox)

uiBox *makePage4(void)
{
	uiBox *page4;

	page4 = newVerticalBox();

	spinbox = uiNewSpinbox();
	uiSpinboxOnChanged(spinbox, onSpinboxChanged, NULL);
	uiBoxAppend(page4, uiControl(spinbox), 0);

	return page4;
}
