// 19 may 2015
#include "test.h"

static uiSpinbox *spinbox;
static uiSlider *slider;
static uiProgressBar *pbar;

#define CHANGED(what) \
	static void on ## what ## Changed(ui ## what *this, void *data) \
	{ \
		uintmax_t value; \
		printf("on %s changed\n", #what); \
		value = ui ## what ## Value(this); \
		uiProgressBarSetValue(pbar, value); \
	}
CHANGED(Spinbox)
CHANGED(Slider)

uiBox *makePage4(void)
{
	uiBox *page4;

	page4 = newVerticalBox();

	spinbox = uiNewSpinbox();
	uiSpinboxOnChanged(spinbox, onSpinboxChanged, NULL);
	uiBoxAppend(page4, uiControl(spinbox), 0);

	slider = uiNewSlider();
	uiSliderOnChanged(slider, onSliderChanged, NULL);
	uiBoxAppend(page4, uiControl(slider), 0);

	pbar = uiNewProgressBar();
	uiBoxAppend(page4, uiControl(pbar), 0);

	return page4;
}
