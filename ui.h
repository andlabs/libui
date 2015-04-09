// 6 april 2015

#ifndef __UI_UI_H__
#define __UI_UI_H__

#include <stdint.h>

typedef struct uiInitError uiInitError;
typedef struct uiInitOptions uiInitOptions;

uiInitError *uiInit(uiInitOptions *);
const char *uiInitErrorMessage(uiInitError *);
void uiInitErrorFree(uiInitError *);

void uiMain(void);
void uiQuit(void);

void uiFreeText(char *);

typedef struct uiControl uiControl;
void uiControlDestroy(uiControl *);
uintptr_t uiControlHandle(uiControl *);

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
// TODO uiWindowMargined()
void uiWindowSetMargined(uiWindow *, int);

uiControl *uiNewButton(const char *);
char *uiButtonText(uiControl *);
void uiButtonSetText(uiControl *, const char *);
void uiButtonOnClicked(uiControl *, void (*)(uiControl *, void *), void *);

uiControl *uiNewHorizontalStack(void);
uiControl *uiNewVerticalStack(void);
void uiStackAdd(uiControl *, uiControl *, int);
// TODO get padded
void uiStackSetPadded(uiControl *, int);

uiControl *uiNewEntry(void);
char *uiEntryText(uiControl *);
void uiEntrySetText(uiControl *, const char *);

uiControl *uiNewCheckbox(const char *);
char *uiCheckboxText(uiControl *);
void uiCheckboxSetText(uiControl *, const char *);
void uiCheckboxOnToggled(uiControl *, void (*)(uiControl *, void *), void *);
int uiCheckboxChecked(uiControl *);
// TODO should this trigger an event?
void uiCheckboxSetChecked(uiControl *, int);

#endif
