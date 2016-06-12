// 2 september 2015
#include <stdio.h>
#include <string.h>
#include "../../ui.h"

static int onClosing(uiWindow *w, void *data)
{
	uiQuit();
	return 1;
}

static int onShouldQuit(void *data)
{
	uiWindow *mainwin = uiWindow(data);

	uiControlDestroy(uiControl(mainwin));
	return 1;
}

static uiControl *makeBasicControlsPage(void)
{
	uiBox *vbox;
	uiBox *hbox;
	uiGroup *group;
	uiForm *entryForm;

	vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);

	hbox = uiNewHorizontalBox();
	uiBoxSetPadded(hbox, 1);
	uiBoxAppend(vbox, uiControl(hbox), 0);

	uiBoxAppend(hbox,
		uiControl(uiNewButton("Button")),
		0);
	uiBoxAppend(hbox,
		uiControl(uiNewCheckbox("Checkbox")),
		0);

	uiBoxAppend(vbox,
		uiControl(uiNewLabel("This is a label. Right now, labels can only span one line.")),
		0);

	uiBoxAppend(vbox,
		uiControl(uiNewHorizontalSeparator()),
		0);

	group = uiNewGroup("Entries");
	uiGroupSetMargined(group, 1);
	uiBoxAppend(vbox, uiControl(group), 1);

	entryForm = uiNewForm();
	uiFormSetPadded(entryForm, 1);
	uiGroupSetChild(group, uiControl(entryForm));

	uiFormAppend(entryForm,
		"Entry",
		uiControl(uiNewEntry()),
		0);
	uiFormAppend(entryForm,
		"Password Entry",
		uiControl(uiNewPasswordEntry()),
		0);
	uiFormAppend(entryForm,
		"Search Entry",
		uiControl(uiNewSearchEntry()),
		0);
	uiFormAppend(entryForm,
		"Multiline Entry",
		uiControl(uiNewMultilineEntry()),
		1);
	uiFormAppend(entryForm,
		"Multiline Entry No Wrap",
		uiControl(uiNewNonWrappingMultilineEntry()),
		1);

	return uiControl(vbox);
}

int main(void)
{
	uiInitOptions options;
	const char *err;
	uiWindow *mainwin;
	uiTab *tab;

	memset(&options, 0, sizeof (uiInitOptions));
	err = uiInit(&options);
	if (err != NULL) {
		fprintf(stderr, "error initializing libui: %s", err);
		uiFreeInitError(err);
		return 1;
	}

	mainwin = uiNewWindow("libui Control Gallery", 640, 480, 1);
	uiWindowOnClosing(mainwin, onClosing, NULL);
	uiOnShouldQuit(onShouldQuit, mainwin);

	tab = uiNewTab();
	uiWindowSetChild(mainwin, uiControl(tab));
	uiWindowSetMargined(mainwin, 1);

	uiTabAppend(tab, "Basic Controls", makeBasicControlsPage());
	uiTabSetMargined(tab, 0, 1);

	uiControlShow(uiControl(mainwin));
	uiMain();
	return 0;
}

#if 0

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
	uiBox *hbox;
	uiGroup *group;
	uiBox *inner;
	uiBox *inner2;
	uiEntry *entry;
	uiCombobox *cbox;
	uiEditableCombobox *ecbox;
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

	hbox = uiNewHorizontalBox();
	uiBoxSetPadded(hbox, 1);
	uiBoxAppend(box, uiControl(hbox), 1);

	group = uiNewGroup("Basic Controls");
	uiGroupSetMargined(group, 1);
	uiBoxAppend(hbox, uiControl(group), 0);

	inner = uiNewVerticalBox();
	uiBoxSetPadded(inner, 1);
	uiGroupSetChild(group, uiControl(inner));

	uiBoxAppend(inner,
		uiControl(uiNewButton("Button")),
		0);
	uiBoxAppend(inner,
		uiControl(uiNewCheckbox("Checkbox")),
		0);
	entry = uiNewEntry();
	uiEntrySetText(entry, "Entry");
	uiBoxAppend(inner,
		uiControl(entry),
		0);
	uiBoxAppend(inner,
		uiControl(uiNewLabel("Label")),
		0);

	uiBoxAppend(inner,
		uiControl(uiNewHorizontalSeparator()),
		0);

	uiBoxAppend(inner,
		uiControl(uiNewDatePicker()),
		0);
	uiBoxAppend(inner,
		uiControl(uiNewTimePicker()),
		0);
	uiBoxAppend(inner,
		uiControl(uiNewDateTimePicker()),
		0);

	uiBoxAppend(inner,
		uiControl(uiNewFontButton()),
		0);

	uiBoxAppend(inner,
		uiControl(uiNewColorButton()),
		0);

	inner2 = uiNewVerticalBox();
	uiBoxSetPadded(inner2, 1);
	uiBoxAppend(hbox, uiControl(inner2), 1);

	group = uiNewGroup("Numbers");
	uiGroupSetMargined(group, 1);
	uiBoxAppend(inner2, uiControl(group), 0);

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
	uiBoxAppend(inner2, uiControl(group), 0);

	inner = uiNewVerticalBox();
	uiBoxSetPadded(inner, 1);
	uiGroupSetChild(group, uiControl(inner));

	cbox = uiNewCombobox();
	uiComboboxAppend(cbox, "Combobox Item 1");
	uiComboboxAppend(cbox, "Combobox Item 2");
	uiComboboxAppend(cbox, "Combobox Item 3");
	uiBoxAppend(inner, uiControl(cbox), 0);

	ecbox = uiNewEditableCombobox();
	uiEditableComboboxAppend(ecbox, "Editable Item 1");
	uiEditableComboboxAppend(ecbox, "Editable Item 2");
	uiEditableComboboxAppend(ecbox, "Editable Item 3");
	uiBoxAppend(inner, uiControl(ecbox), 0);

	rb = uiNewRadioButtons();
	uiRadioButtonsAppend(rb, "Radio Button 1");
	uiRadioButtonsAppend(rb, "Radio Button 2");
	uiRadioButtonsAppend(rb, "Radio Button 3");
	uiBoxAppend(inner, uiControl(rb), 1);

	tab = uiNewTab();
	uiTabAppend(tab, "Page 1", uiControl(uiNewHorizontalBox()));
	uiTabAppend(tab, "Page 2", uiControl(uiNewHorizontalBox()));
	uiTabAppend(tab, "Page 3", uiControl(uiNewHorizontalBox()));
	uiBoxAppend(inner2, uiControl(tab), 1);

	uiControlShow(uiControl(mainwin));
	uiMain();
	uiUninit();
	return 0;
}

#endif
