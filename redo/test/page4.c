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
		uiSpinboxSetValue(spinbox, value); \
		uiSliderSetValue(slider, value); \
		uiProgressBarSetValue(pbar, value); \
	}
CHANGED(Spinbox)
CHANGED(Slider)

#define SETTOO(what, name, n) \
	static void set ## what ## Too ## name(uiButton *this, void *data) \
	{ \
		ui ## what ## SetValue(ui ## what(data), n); \
	}
SETTOO(Spinbox, Low, -80)
SETTOO(Spinbox, High, 80)
SETTOO(Slider, Low, -80)
SETTOO(Slider, High, 80)

uiCombobox *cbox;

uiBox *makePage4(void)
{
	uiBox *page4;
	uiBox *hbox;
	uiSpinbox *xsb;
	uiButton *b;
	uiSlider *xsl;

	page4 = newVerticalBox();

	spinbox = uiNewSpinbox(0, 100);
	uiSpinboxOnChanged(spinbox, onSpinboxChanged, NULL);
	uiBoxAppend(page4, uiControl(spinbox), 0);

	slider = uiNewSlider(0, 100);
	uiSliderOnChanged(slider, onSliderChanged, NULL);
	uiBoxAppend(page4, uiControl(slider), 0);

	pbar = uiNewProgressBar();
	uiBoxAppend(page4, uiControl(pbar), 0);

	uiBoxAppend(page4, uiControl(uiNewHorizontalSeparator()), 0);

	hbox = newHorizontalBox();
	xsb = uiNewSpinbox(-40, 40);
	uiBoxAppend(hbox, uiControl(xsb), 0);
	b = uiNewButton("Bad Low");
	uiButtonOnClicked(b, setSpinboxTooLow, xsb);
	uiBoxAppend(hbox, uiControl(b), 0);
	b = uiNewButton("Bad High");
	uiButtonOnClicked(b, setSpinboxTooHigh, xsb);
	uiBoxAppend(hbox, uiControl(b), 0);
	uiBoxAppend(page4, uiControl(hbox), 0);

	hbox = newHorizontalBox();
	xsl = uiNewSlider(-40, 40);
	uiBoxAppend(hbox, uiControl(xsl), 0);
	b = uiNewButton("Bad Low");
	uiButtonOnClicked(b, setSliderTooLow, xsl);
	uiBoxAppend(hbox, uiControl(b), 0);
	b = uiNewButton("Bad High");
	uiButtonOnClicked(b, setSliderTooHigh, xsl);
	uiBoxAppend(hbox, uiControl(b), 0);
	uiBoxAppend(page4, uiControl(hbox), 0);

	uiBoxAppend(page4, uiControl(uiNewHorizontalSeparator()), 0);

	cbox = uiNewCombobox();
	uiComboboxAppend(cbox, "Item 1");
	uiComboboxAppend(cbox, "Item 2");
	uiComboboxAppend(cbox, "Item 3");
	uiBoxAppend(page4, uiControl(cbox), 0);

	return page4;
}
