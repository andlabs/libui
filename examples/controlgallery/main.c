// 2 september 2015
#include <stdio.h>
#include <string.h>
#include "../../ui.h"

static uiWindow *mainwin;

static int onClosing(uiWindow *w, void *data)
{
	uiControlDestroy(uiControl(mainwin));
	uiQuit();
	return 0;
}

static int shouldQuit(void *data)
{
	uiControlDestroy(uiControl(mainwin));
	return 1;
}

static void openClicked(uiMenuItem *item, uiWindow *w, void *data)
{
	char *filename;

	filename = uiOpenFile(mainwin);
	if (filename == NULL) {
		uiMsgBoxError(mainwin, "No file selected", "Don't be alarmed!");
		return;
	}
	uiMsgBox(mainwin, "File selected", filename);
	uiFreeText(filename);
}

static void saveClicked(uiMenuItem *item, uiWindow *w, void *data)
{
	char *filename;

	filename = uiSaveFile(mainwin);
	if (filename == NULL) {
		uiMsgBoxError(mainwin, "No file selected", "Don't be alarmed!");
		return;
	}
	uiMsgBox(mainwin, "File selected (don't worry, it's still there)", filename);
	uiFreeText(filename);
}

static uiSpinbox *spinbox;
static uiSlider *slider;
static uiProgressBar *progressbar;

static void update(intmax_t value)
{
	uiSpinboxSetValue(spinbox, value);
	uiSliderSetValue(slider, value);
	uiProgressBarSetValue(progressbar, value);
}

static void onSpinboxChanged(uiSpinbox *s, void *data)
{
	update(uiSpinboxValue(spinbox));
}

static void onSliderChanged(uiSlider *s, void *data)
{
	update(uiSliderValue(slider));
}

int main(void)
{
	uiInitOptions o;
	const char *err;
	uiMenu *menu;
	uiMenuItem *item;
	uiBox *box;
	uiGroup *group;
	uiBox *inner;
	uiBox *inner2;
	uiEntry *entry;
	uiCombobox *cbox;
	uiRadioButtons *rb;
	uiTab *tab;

	memset(&o, 0, sizeof (uiInitOptions));
	err = uiInit(&o);
	if (err != NULL) {
		fprintf(stderr, "error initializing ui: %s\n", err);
		uiFreeInitError(err);
		return 1;
	}

	menu = uiNewMenu("File");
	item = uiMenuAppendItem(menu, "Open");
	uiMenuItemOnClicked(item, openClicked, NULL);
	item = uiMenuAppendItem(menu, "Save");
	uiMenuItemOnClicked(item, saveClicked, NULL);
	item = uiMenuAppendQuitItem(menu);
	uiOnShouldQuit(shouldQuit, NULL);

	menu = uiNewMenu("Edit");
	item = uiMenuAppendCheckItem(menu, "Checkable Item");
	uiMenuAppendSeparator(menu);
	item = uiMenuAppendItem(menu, "Disabled Item");
	uiMenuItemDisable(item);
	item = uiMenuAppendPreferencesItem(menu);

	menu = uiNewMenu("Help");
	item = uiMenuAppendItem(menu, "Help");
	item = uiMenuAppendAboutItem(menu);

	mainwin = uiNewWindow("libui Control Gallery", 640, 480, 1);
	uiWindowSetMargined(mainwin, 1);
	uiWindowOnClosing(mainwin, onClosing, NULL);

	box = uiNewVerticalBox();
	uiBoxSetPadded(box, 1);
	uiWindowSetChild(mainwin, uiControl(box));

	group = uiNewGroup("Buttons and Text");
	uiGroupSetMargined(group, 1);
	uiBoxAppend(box, uiControl(group), 0);

	inner = uiNewHorizontalBox();
	uiBoxSetPadded(inner, 1);
	uiGroupSetChild(group, uiControl(inner));

	inner2 = uiNewVerticalBox();
	uiBoxSetPadded(inner2, 1);
	uiBoxAppend(inner, uiControl(inner2), 1);

	uiBoxAppend(inner2,
		uiControl(uiNewButton("Button")),
		0);
	uiBoxAppend(inner2,
		uiControl(uiNewCheckbox("Checkbox")),
		0);
	entry = uiNewEntry();
	uiEntrySetText(entry, "Entry");
	uiBoxAppend(inner2,
		uiControl(entry),
		0);
	uiBoxAppend(inner2,
		uiControl(uiNewLabel("Label")),
		0);

	inner2 = uiNewVerticalBox();
	uiBoxSetPadded(inner2, 1);
	uiBoxAppend(inner, uiControl(inner2), 1);

	uiBoxAppend(inner2,
		uiControl(uiNewDatePicker()),
		0);
	uiBoxAppend(inner2,
		uiControl(uiNewTimePicker()),
		0);
	uiBoxAppend(inner2,
		uiControl(uiNewDateTimePicker()),
		0);

	group = uiNewGroup("Numbers");
	uiGroupSetMargined(group, 1);
	uiBoxAppend(box, uiControl(group), 0);

	inner = uiNewVerticalBox();
	uiBoxSetPadded(inner, 1);
	uiGroupSetChild(group, uiControl(inner));

	spinbox = uiNewSpinbox(0, 100);
	uiSpinboxOnChanged(spinbox, onSpinboxChanged, NULL);
	uiBoxAppend(inner, uiControl(spinbox), 0);

	slider = uiNewSlider(0, 100);
	uiSliderOnChanged(slider, onSliderChanged, NULL);
	uiBoxAppend(inner, uiControl(slider), 0);

	progressbar = uiNewProgressBar();
	uiBoxAppend(inner, uiControl(progressbar), 0);

	group = uiNewGroup("Lists");
	uiGroupSetMargined(group, 1);
	uiBoxAppend(box, uiControl(group), 0);

	inner = uiNewHorizontalBox();
	uiBoxSetPadded(inner, 1);
	uiGroupSetChild(group, uiControl(inner));

	inner2 = uiNewVerticalBox();
	uiBoxSetPadded(inner2, 1);
	uiBoxAppend(inner, uiControl(inner2), 1);

	cbox = uiNewCombobox();
	uiComboboxAppend(cbox, "Combobox Item 1");
	uiComboboxAppend(cbox, "Combobox Item 2");
	uiComboboxAppend(cbox, "Combobox Item 3");
	uiBoxAppend(inner2, uiControl(cbox), 0);

	cbox = uiNewEditableCombobox();
	uiComboboxAppend(cbox, "Editable Item 1");
	uiComboboxAppend(cbox, "Editable Item 2");
	uiComboboxAppend(cbox, "Editable Item 3");
	uiBoxAppend(inner2, uiControl(cbox), 0);

	rb = uiNewRadioButtons();
	uiRadioButtonsAppend(rb, "Radio Button 1");
	uiRadioButtonsAppend(rb, "Radio Button 2");
	uiRadioButtonsAppend(rb, "Radio Button 3");
	uiBoxAppend(inner, uiControl(rb), 1);

	tab = uiNewTab();
	uiTabAppend(tab, "Page 1", uiControl(uiNewHorizontalBox()));
	uiTabAppend(tab, "Page 2", uiControl(uiNewHorizontalBox()));
	uiTabAppend(tab, "Page 3", uiControl(uiNewHorizontalBox()));
	uiBoxAppend(box, uiControl(tab), 1);

	uiControlShow(uiControl(mainwin));
	uiMain();
	uiUninit();
	return 0;
}
