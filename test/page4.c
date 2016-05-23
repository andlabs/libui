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

static uiCombobox *cbox;
static uiEditableCombobox *editable;
static uiRadioButtons *rb;

static void appendCBRB(uiButton *b, void *data)
{
	uiComboboxAppend(cbox, "New Item");
	uiEditableComboboxAppend(editable, "New Item");
	uiRadioButtonsAppend(rb, "New Item");
}

static void onCBChanged(uiCombobox *c, void *data)
{
	printf("%s combobox changed to %d\n",
		(char *) data,
		(int) uiComboboxSelected(c));
	uiEditableComboboxSetText(editable, "changed");
}

static void onECBChanged(uiEditableCombobox *c, void *data)
{
	char *t;

	t = uiEditableComboboxText(c);
	printf("%s combobox changed to %s\n",
		(char *) data,
		t);
	uiFreeText(t);
}

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
	uiComboboxOnSelected(cbox, onCBChanged, "noneditable");
	uiBoxAppend(page4, uiControl(cbox), 0);

	editable = uiNewEditableCombobox();
	uiEditableComboboxAppend(editable, "Editable Item 1");
	uiEditableComboboxAppend(editable, "Editable Item 2");
	uiEditableComboboxAppend(editable, "Editable Item 3");
	uiEditableComboboxOnChanged(editable, onECBChanged, "editable");
	uiBoxAppend(page4, uiControl(editable), 0);

	rb = uiNewRadioButtons();
	uiRadioButtonsAppend(rb, "Item 1");
	uiRadioButtonsAppend(rb, "Item 2");
	uiRadioButtonsAppend(rb, "Item 3");
	uiBoxAppend(page4, uiControl(rb), 0);

	hbox = newHorizontalBox();
	b = uiNewButton("Append");
	uiButtonOnClicked(b, appendCBRB, NULL);
	uiBoxAppend(hbox, uiControl(b), 0);
	uiBoxAppend(page4, uiControl(hbox), 0);

	uiBoxAppend(page4, uiControl(uiNewHorizontalSeparator()), 0);

	uiBoxAppend(page4, uiControl(uiNewDateTimePicker()), 0);
	uiBoxAppend(page4, uiControl(uiNewDatePicker()), 0);
	uiBoxAppend(page4, uiControl(uiNewTimePicker()), 0);

	return page4;
}
