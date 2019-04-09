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
	BA(uiNewPasswordEntry());
	BA(uiNewSearchEntry());
	BA(uiNewVerticalSeparator());

	uiControlShow(uiControl(w));
}

static void buttonClicked(uiButton *b, void *data)
{
	openTestWindow((uiBox *(*)(void)) data);
}

static void entryChanged(uiEntry *e, void *data)
{
	char *text;

	text = uiEntryText(e);
	printf("%s entry changed: %s\n", (const char *) data, text);
	uiFreeText(text);
}

static void showHide(uiButton *b, void *data)
{
	uiControl *c = uiControl(data);

	if (uiControlVisible(c))
		uiControlHide(c);
	else
		uiControlShow(c);
}

static void setIndeterminate(uiButton *b, void *data)
{
	uiProgressBar *p = uiProgressBar(data);
	int value;

	value = uiProgressBarValue(p);
	if (value == -1)
		value = 50;
	else
		value = -1;
	uiProgressBarSetValue(p, value);
}

static void deleteFirst(uiButton *b, void *data)
{
	uiForm *f = uiForm(data);

	uiFormDelete(f, 0);
}

uiBox *makePage13(void)
{
	uiBox *page13;
	uiRadioButtons *rb;
	uiButton *b;
	uiForm *f;
	uiEntry *e;
	uiProgressBar *p;

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

	f = newForm();

	e = uiNewPasswordEntry();
	uiEntryOnChanged(e, entryChanged, "password");
	uiFormAppend(f, "Password Entry", uiControl(e), 0);

	e = uiNewSearchEntry();
	uiEntryOnChanged(e, entryChanged, "search");
	uiFormAppend(f, "Search Box", uiControl(e), 0);

	uiFormAppend(f, "MLE", uiControl(uiNewMultilineEntry()), 1);

	p = uiNewProgressBar();
	uiProgressBarSetValue(p, 50);
	uiBoxAppend(page13, uiControl(p), 0);
	b = uiNewButton("Toggle Indeterminate");
	uiButtonOnClicked(b, setIndeterminate, p);
	uiBoxAppend(page13, uiControl(b), 0);

	b = uiNewButton("Show/Hide");
	uiButtonOnClicked(b, showHide, e);
	uiBoxAppend(page13, uiControl(b), 0);
	b = uiNewButton("Delete First");
	uiButtonOnClicked(b, deleteFirst, f);
	uiBoxAppend(page13, uiControl(b), 0);
	uiBoxAppend(page13, uiControl(f), 1);

	return page13;
}
