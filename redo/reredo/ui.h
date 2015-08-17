// 6 april 2015

#ifndef __LIBUI_UI_H__
#define __LIBUI_UI_H__

#include <stddef.h>
#include <stdint.h>

#ifndef _UI_EXTERN
#define _UI_EXTERN extern
#endif

typedef struct uiInitOptions uiInitOptions;

struct uiInitOptions {
	size_t Size;
};

_UI_EXTERN const char *uiInit(uiInitOptions *options);
_UI_EXTERN void uiUninit(void);
_UI_EXTERN void uiFreeInitError(const char *err);

_UI_EXTERN void uiMain(void);
_UI_EXTERN void uiQuit(void);

_UI_EXTERN void uiOnShouldQuit(int (*f)(void *data), void *data);

_UI_EXTERN void uiFreeText(char *text);

_UI_EXTERN uintmax_t uiRegisterType(const char *name, uintmax_t parent, size_t size);
_UI_EXTERN void *uiIsA(void *p, uintmax_t type, int fail);
typedef struct uiTyped uiTyped;
struct uiTyped {
	uintmax_t Type;
};
#define uiTyped(this) ((uiTyped *) (this))

/* TODO only on the windows backend now
typedef struct uiSizing uiSizing;
typedef struct uiSizingSys uiSizingSys;

struct Sizing {
	field XPadding intmax_t;
	field YPadding intmax_t;
	field Sys *uiSizingSys;
};
*/

typedef struct uiControl uiControl;

struct uiControl {
	uiTyped t;
	void *Internal;		// for use by libui only
	void (*CommitDestroy)(uiControl *);
	uintptr_t (*Handle)(uiControl *);
	void (*ContainerUpdateState)(uiControl *);
};
_UI_EXTERN uintmax_t uiControlType(void);
#define uiControl(this) ((uiControl *) uiIsA((this), uiControlType(), 1))
_UI_EXTERN void uiControlDestroy(uiControl *);
_UI_EXTERN void uiControlSetParent(uiControl *, uiControl *);
_UI_EXTERN void uiControlShow(uiControl *);
_UI_EXTERN void uiControlHide(uiControl *);
_UI_EXTERN void uiControlEnable(uiControl *);
_UI_EXTERN void uiControlDisable(uiControl *);

/* TODO
func NewControl(type uintmax_t) *Control;

raw "#define uiDefineControlType(typename, funcname, realtype) \\";
raw "	static uintmax_t type_ ## typename = 0; \\";
raw "	uintmax_t funcname(void) \\";
raw "	{ \\";
raw "		if (type_ ## typename == 0) \\";
raw "			type_ ## typename = uiRegisterType(#typename, uiTypeControl(), sizeof (realtype)); \\";
raw "		return type_ ## typename; \\";
raw "	}";

func FreeSizing(d *Sizing);

func MakeContainer(c *Control) uintptr_t;
*/

typedef struct uiWindow uiWindow;
_UI_EXTERN uintmax_t uiWindowType(void);
#define uiWindow(this) ((uiWindow *) uiIsA((this), uiWindowType(), 1))
_UI_EXTERN char *uiWindowTitle(uiWindow *w);
_UI_EXTERN void uiWindowSetTitle(uiWindow *w, const char *title);
_UI_EXTERN void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *w, void *data), void *data);
_UI_EXTERN void uiWindowSetChild(uiWindow *w, uiControl *child);
_UI_EXTERN int uiWindowMargined(uiWindow *w);
_UI_EXTERN void uiWindowSetMargined(uiWindow *w, int margined);
_UI_EXTERN uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar);

typedef struct uiButton uiButton;
_UI_EXTERN uintmax_t uiButtonType(void);
#define uiButton(this) ((uiButton *) uiIsA((this), uiButtonType(), 1))
_UI_EXTERN char *uiButtonText(uiButton *b);
_UI_EXTERN void uiButtonSetText(uiButton *b, const char *text);
_UI_EXTERN void uiButtonOnClicked(uiButton *b, void (*f)(uiButton *b, void *data), void *data);
_UI_EXTERN uiButton *uiNewButton(const char *text);

typedef struct uiBox uiBox;
_UI_EXTERN uintmax_t uiBoxType(void);
#define uiBox(this) ((uiBox *) uiIsA((this), uiBoxType(), 1))
_UI_EXTERN void uiBoxAppend(uiBox *b, uiControl *child, int stretchy);
_UI_EXTERN void uiBoxDelete(uiBox *b, uintmax_t index);
_UI_EXTERN int uiBoxPadded(uiBox *b);
_UI_EXTERN void uiBoxSetPadded(uiBox *b, int padded);
_UI_EXTERN uiBox *uiNewHorizontalBox(void);
_UI_EXTERN uiBox *uiNewVerticalBox(void);

typedef struct uiEntry uiEntry;
_UI_EXTERN uintmax_t uiEntryType(void);
#define uiEntry(this) ((uiEntry *) uiIsA((this), uiEntryType(), 1))
_UI_EXTERN char *uiEntryText(uiEntry *e);
_UI_EXTERN void uiEntrySetText(uiEntry *e, const char *text);
_UI_EXTERN void uiEntryOnChanged(uiEntry *e, void (*f)(uiEntry *e, void *data), void *data);
_UI_EXTERN int uiEntryReadOnly(uiEntry *e);
_UI_EXTERN void uiEntrySetReadOnly(uiEntry *e, int readonly);
_UI_EXTERN uiEntry *uiNewEntry(void);

typedef struct uiCheckbox uiCheckbox;
_UI_EXTERN uintmax_t uiCheckboxType(void);
#define uiCheckbox(this) ((uiCheckbox *) uiIsA((this), uiCheckboxType(), 1))
_UI_EXTERN char *uiCheckboxText(uiCheckbox *c);
_UI_EXTERN void uiCheckboxSetText(uiCheckbox *c, const char *text);
_UI_EXTERN void uiCheckboxOnToggled(uiCheckbox *c, void (*f)(uiCheckbox *c, void *data), void *data);
_UI_EXTERN int uiCheckboxChecked(uiCheckbox *c);
_UI_EXTERN void uiCheckboxSetChecked(uiCheckbox *c, int checked);
_UI_EXTERN uiCheckbox *uiNewCheckbox(const char *text);

typedef struct uiLabel uiLabel;
_UI_EXTERN uintmax_t uiLabelType(void);
#define uiLabel(this) ((uiLabel *) uiIsA((this), uiLabelType(), 1))
_UI_EXTERN char *uiLabelText(uiLabel *l);
_UI_EXTERN void uiLabelSetText(uiLabel *l, const char *text);
_UI_EXTERN uiLable *uiNewLabel(const char *text);

typedef struct uiTab uiTab;
_UI_EXTERN uintmax_t uiTabType(void);
#define uiTab(this) ((uiTab *) uiIsA((this), uiTabType(), 1))
_UI_EXTERN void uiTabAppend(uITab *t, const char *name uiControl *c);
_UI_EXTERN void uiTabInsertAt(uiTab *t, const char *name, uintmax_t before, uiControl *c);
_UI_EXTERN void uiTabDelete(uiTab *t, uintmax_t index);
_UI_EXTERN uintmax_t uiTabNumPages(uiTab *t);
_UI_EXTERN int uiTabMargined(uiTab *t, uintmax_t page);
_UI_EXTERN void uiTabSetMargined(uiTab *t, uintmax_t page, int margined);
_UI_EXTERN uiTab *uiNewTab(void);

typedef struct uiGroup uiGroup;
_UI_EXTERN uintmax_t uiGroupType(void);
#define uiGroup(this) ((uiGroup *) uiIsA((this), uiGroupType(), 1))
_UI_EXTERN char *uiGroupTitle(uiGroup *g);
_UI_EXTERN void uiGroupSetTitle(uiGroup *g, const char *title);
_UI_EXTERN void uiGroupSetChild(uiGroup *g, uiControl *c);
_UI_EXTERN int uiGroupMargined(uiGroup *g);
_UI_EXTERN void uiGroupSetMargined(uiGroup *g, int margined);
_UI_EXTERN uiGroup *uiNewGroup(const char *title);

// spinbox/slider rules:
// setting value outside of range will automatically clamp
// initial value is minimum
// complaint if min >= max?

typedef struct uiSpinbox uiSpinbox;
_UI_EXTERN uintmax_t uiSpinboxType(void);
#define uiSpinbox(this) ((uiSpinbox *) uiIsA((this), uiSpinboxType(), 1))
_UI_EXTERN intmx_t uiSpinboxValue(uiSpinbox *s);
_UI_EXTERN void uiSpinboxSetValue(uiSpinbox *s, intmax_t value);
_UI_EXTERN void uiSpinboxOnChanged(uiSpinbox *s, void (*f)(uiSpinbox *s, void *data), void *data);
_UI_EXTERN uiSpinbox *uiNewSpinbox(intmax_t min, intmax_t max);

typedef struct uiProgressBar uiProgressBar;
_UI_EXTERN uintmax_t uiProgressBarType(void);
#define uiProgressBar(this) ((uiProgressBar *) uiIsA((this), uiProgressBarType(), 1))
// TODO uiProgressBarValue()
_UI_EXTERN uiProgressBarSetValue(uiProgressBar *p, int n);
_UI_EXTERN uiProgressBar *uiNewProgressBar(void);

typedef struct uiSlider uiSlider;
_UI_EXTERN uintmax_t uiSliderType(void);
#define uiSlider(this) ((uiSlider *) uiIsA((this), uiSliderType(), 1))
_UI_EXTERN intmx_t uiSliderValue(uiSlider *s);
_UI_EXTERN void uiSliderSetValue(uiSlider *s, intmax_t value);
_UI_EXTERN void uiSliderOnChanged(uiSlider *s, void (*f)(uiSlider *s, void *data), void *data);
_UI_EXTERN uiSlider *uiNewSlider(intmax_t min, intmax_t max);

typedef struct uiSeparator uiSeparator;
_UI_EXTERN uintmax_t uiSeparatorType(void);
#define uiSeparator(this) ((uiSeparator *) uiIsA((this), uiSeparatorType(), 1))
_UI_EXTERN uiSeparator *NewHorizontalSeparator(void);

typedef struct uiCombobox uiCombobox;
_UI_EXTERN uintmax_t uiComboboxType(void);
#define uiCombobox(this) ((uiCombobox *) uiIsA((this), uiComboboxType(), 1))
_UI_EXTERN void uiComboboxAppend(uiCombobox *c, const char *text);
_UI_EXTERN uiCombobox *uiNewCombobox(void);
_UI_EXTERN uiCombobox *uiNewEditableCombobox(void);

typedef struct uiRadioButtons uiRadioButtons;
_UI_EXTERN uintmax_t uiRadioButtonsType(void);
#define uiRadioButtons(this) ((uiRadioButtons *) uiIsA((this), uiRadioButtonsType(), 1))
_UI_EXTERN void uiRadioButtonsAppend(uiRadioButtons *r, const char *text);
_UI_EXTERN uiRadioButtons *uiNewRadioButtons(void);

typedef struct uiDateTimePicker uiDateTimePicker;
_UI_EXTERN uintmax_t uiDateTimePickerType(void);
#define uiDateTimePicker(this) ((uiDateTimePicker *) uiIsA((this), uiDateTimePickerType(), 1))
_UI_EXTERN uiDateTimePicker *uiNewDateTimePicker(void);
_UI_EXTERN uiDateTimePicker *uiNewDatePicker(void);
_UI_EXTERN uiDateTimePicker *uiNewTimePicker(void);

typedef struct uiMenu uiMenu;
typedef struct uiMenuItem uiMenuItem;
_UI_EXTERN uintmax_t uiMenuType(void);
#define uiMenu(this) ((uiMenu *) uiIsA((this), uiMenuType(), 1))
_UI_EXTERN uiMenuItem *uiMenuAppendItem(uiMenu *m, char *name);
_UI_EXTERN uiMenuItem *uiMenuAppendCheckItem(uiMenu *m, const char *name);
_UI_EXTERN uiMenuItem *uiMenuAppendQuitItem(uiMenu *m);
_UI_EXTERN uiMenuItem *uiMenuAppendPreferencesItem(uiMenu *m);
_UI_EXTERN uiMenuItem *uiMenuAppendAboutItem(uiMenu *m);
_UI_EXTERN void uiMenuAppendSeparator(uiMenu *m);
_UI_EXTERN uiMenu *uiNewMenu(const char *name);

_UI_EXTERN uintmax_t uiMenuItemType(void);
#define uiMenuItem(this) ((uiMenuItem *) uiIsA((this), uiMenuItemType(), 1))
_UI_EXTERN void uiMenuItemEnable(uiMenuItem *m);
_UI_EXTERN void uiMenuItemDisable(uiMenuItem *m);
_UI_EXTERN void uiMenuItemOnClicked(uiMenuItem *m, void (*f)(uiMenuItem *sender, uiWindow *window, void *data), void *data);
_UI_EXTERN int uiMenuItemChecked(uiMenuItem *m);
_UI_EXTERN void uiMenuItemSetChecked(uiMenuItem *m, int checked);

_UI_EXTERN char *OpenFile(void);
_UI_EXTERN char *SaveFile(void);
_UI_EXTERN void MsgBox(const char *title, const char *description);
_UI_EXTERN void MsgBoxError(const char *title, const char *description);

#endif
