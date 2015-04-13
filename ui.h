// 6 april 2015

#ifndef __UI_UI_H__
#define __UI_UI_H__

#include <stdint.h>

typedef struct uiInitOptions uiInitOptions;

// TODO note that should be initialized to zero
struct uiInitOptions {
	// TODO cbSize

	// If nonzero, allocations will be logged to stderr.
	// See leaks.awk.
	int debugLogAllocations;
};

const char *uiInit(uiInitOptions *);
void uiFreeInitError(const char *);

void uiMain(void);
void uiQuit(void);

void uiFreeText(char *);

typedef struct uiSizing uiSizing;
typedef struct uiSizingSys uiSizingSys;
struct uiSizing {
	intmax_t xPadding;
	intmax_t yPadding;
	uiSizingSys *sys;
};

typedef strut uiContainer uiContainer;

typedef struct uiControl uiControl;
struct uiControl {
	void *data;			// for use by implementations only
	void *internal;			// for use by ui only
	void (*destroy)(uiControl *);
	uintptr_t (*handle)(uiControl *);
	void (*setParent)(uiControl *, uiParent *);
	void (*removeParent)(uiControl *);
	void (*preferredSize)(uiControl *, uiSizing *, intmax_t *, intmax_t *);
	void (*resize)(uiControl *, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
	int (*visible)(uiControl *);
	void (*show)(uiControl *);
	void (*hide)(uiControl *);
	void (*containerShow)(uiControl *);
	void (*containerHide)(uiControl *);
	void (*enable)(uiControl *);
	void (*disable)(uiControl *);
	void (*containerEnable)(uiControl *);
	void (*containerDisable)(uiControl *);
};
void uiControlDestroy(uiControl *);
uintptr_t uiControlHandle(uiControl *);
void uiControlSetParent(uiControl *, uiParent *);
void uiControlRemoveParent(uiControl *);
void uiControlPreferredSize(uiControl *, uiSizing *, intmax_t *width, intmax_t *height);
void uiControlResize(uiControl *, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
int uiControlVisible(uiControl *);
void uiControlShow(uiControl *);
void uiControlHide(uiControl *);
void uiControlContainerShow(uiControl *);
void uiControlContainerHide(uiControl *);
void uiControlEnable(uiControl *);
void uiControlDisable(uiControl *);
void uiControlContainerEnable(uiControl *);
void uiControlContainerDisable(uiControl *);

// uiParent represents an OS control that hosts other OS controls.
// It is used internally by package ui and by implementations.
// uiWindow, uiTab, and uiGroup all use uiParents to store their controls.
struct uiParent {
	// Internal points to internal data.
	// Do not access or alter this field.
	void *Internal;

	// Handle returns the window handle of the uiParent.
	// On Windows, this is a HWND.
	// On GTK+, this is a GtkContainer.
	// On Mac OS X, this is a NSView.
	uintptr_t (*Handle)(uiParent *p);
#define uiParentHandle(p) ((*((p)->Handle))((p)))

	// SetChild sets the uiControl that this uiParent relegates.
	// It calls uiControl.SetParent() which should, in turn, call uiParent.Update().
	// The uiParent should already not have a child and the uiControl should already not have a parent.
	// 
	// child can be NULL, in which case the uiParent has no children.
	// This form should be called by uiControl.RemoveParent().
	void (*SetChild)(uiParent *p, uiControl *child);
#define uiParentSetChild(p, child) ((*((p)->SetChild))((p), (child)))

	// SetMargins sets the margins of the uiParent to the given margins.
	// It does not call uiParent.Update(); its caller must.
	// The units of the margins are backend-defined.
	// The initial margins are all 0.
	void (*SetMargins)(uiParent *p, intmax_t left, intmax_t top, intmax_t right, intmax_t bottom);
#define uiParentSetMargins(p, left, top, right, bottom) ((*((p)->SetMargins))((p), (left), (top), (right), (bottom)))

	// TODO Resize?

	// Update tells the uiParent to re-layout its children immediately.
	// It is called when a widget is shown or hidden or when a control is added or removed from a container such as uiStack.
	void (*Update)(uiParent *p);
#define uiParentUpdate(p) ((*((p)->Update))((p)))
};
uiParent *uiNewParent(uintptr_t);

typedef struct uiWindow uiWindow;
uiWindow *uiNewWindow(char *, int, int);
void uiWindowDestroy(uiWindow *);
uintptr_t uiWindowHandle(uiWindow *);
char *uiWindowTitle(uiWindow *);
void uiWindowSetTitle(uiWindow *, const char *);
void uiWindowShow(uiWindow *);
void uiWindowHide(uiWindow *);
void uiWindowOnClosing(uiWindow *, int (*)(uiWindow *, void *), void *);
void uiWindowSetChild(uiWindow *, uiControl *);
int uiWindowMargined(uiWindow *);
void uiWindowSetMargined(uiWindow *, int);

uiControl *uiNewButton(const char *);
char *uiButtonText(uiControl *);
void uiButtonSetText(uiControl *, const char *);
void uiButtonOnClicked(uiControl *, void (*)(uiControl *, void *), void *);

uiControl *uiNewHorizontalStack(void);
uiControl *uiNewVerticalStack(void);
void uiStackAdd(uiControl *, uiControl *, int);
int uiStackPadded(uiControl *);
void uiStackSetPadded(uiControl *, int);

uiControl *uiNewEntry(void);
char *uiEntryText(uiControl *);
void uiEntrySetText(uiControl *, const char *);

uiControl *uiNewCheckbox(const char *);
char *uiCheckboxText(uiControl *);
void uiCheckboxSetText(uiControl *, const char *);
void uiCheckboxOnToggled(uiControl *, void (*)(uiControl *, void *), void *);
int uiCheckboxChecked(uiControl *);
void uiCheckboxSetChecked(uiControl *, int);

uiControl *uiNewLabel(const char *);
char *uiLabelText(uiControl *);
void uiLabelSetText(uiControl *, const char *);

uiControl *uiNewTab(void);
void uiTabAddPage(uiControl *, const char *, uiControl *);

#endif
