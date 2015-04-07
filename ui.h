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

typedef struct uiControl uiControl;

typedef struct uiWindow uiWindow;
uiWindow *uiNewWindow(char *, int, int);
void uiWindowDestroy(uiWindow *);
uintptr_t uiWindowHandle(uiWindow *);
// TODO titles
void uiWindowShow(uiWindow *);
void uiWindowHide(uiWindow *);
void uiWindowOnClosing(uiWindow *, int (*)(uiWindow *, void *), void *);
void uiWindowSetChild(uiWindow *, uiControl *);

uiControl *uiNewButton(const char *);
// TODO text
void uiButtonOnClicked(uiControl *, void (*)(uiControl *, void *), void *);

#endif
