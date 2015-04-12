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

typedef struct uiControl uiControl;
struct uiControl {
	void *data;			// for use by implementations only
	void *internal;			// for use by ui only
	void (*destroy)(uiControl *);
	uintptr_t (*handle)(uiControl *);
	void (*setParent)(uiControl *, uintptr_t);
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
void uiControlSetParent(uiControl *, uintptr_t);
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

#endif
