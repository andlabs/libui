// 28 may 2016
#include "test.h"

static int winClose(uiWindow *w, void *data)
{
	return 1;
}

static void openTestWindow(uiBox *(*mkf)(void))
{
	uiWindow *w;
	uiBox *b;
	uiCombobox *c;
	uiEditableCombobox *e;
	uiRadioButtons *r;

	w = uiNewWindow("Test", 100, 100, 0);
	uiWindowOnClosing(w, winClose, NULL);
	uiWindowSetMargined(w, 1);
	b = (*mkf)();
	uiWindowSetChild(w, uiControl(b));

#define BA(x) uiBoxAppend(b, uiControl(x), 0)
	BA(uiNewButton(""));
	BA(uiNewCheckbox(""));
	BA(uiNewEntry());
	BA(uiNewLabel(""));
	BA(uiNewSpinbox(0, 100));
	BA(uiNewProgressBar());
	BA(uiNewSlider(0, 100));
	BA(uiNewHorizontalSeparator());
	c = uiNewCombobox();
	uiComboboxAppend(c, "");
	BA(c);
	e = uiNewEditableCombobox();
	uiEditableComboboxAppend(e, "");
	BA(e);
	r = uiNewRadioButtons();
	uiRadioButtonsAppend(r, "");
	BA(r);
	BA(uiNewDateTimePicker());
	BA(uiNewDatePicker());
	BA(uiNewTimePicker());
	BA(uiNewMultilineEntry());
	// TODO nonscrolling and scrolling areas?
	BA(uiNewFontButton());
	BA(uiNewColorButton());

	uiControlShow(uiControl(w));
}

static void buttonClicked(uiButton *b, void *data)
{
	openTestWindow((uiBox *(*)(void)) data);
}

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
	uiButtonOnClicked(b, buttonClicked, uiNewHorizontalBox);
	uiBoxAppend(page13, uiControl(b), 0);

	b = uiNewButton("Vertical");
	uiButtonOnClicked(b, buttonClicked, uiNewVerticalBox);
	uiBoxAppend(page13, uiControl(b), 0);

	return page13;
}
