// 6 april 2015

#ifndef uiprivIncludeGuard_ui_h
#define uiprivIncludeGuard_ui_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef uiprivBuildingLibui
#if defined(_WIN32) && !defined(uiStatic)
#define uiprivExtern __declspec(dllexport) extern
#elif defined(_WIN32)
#define uiprivExtern extern
#else
#define uiprivExtern __attribute__((visibility("default"))) extern
#endif
#else
#if defined(_WIN32) && !defined(uiStatic)
#define uiprivExtern __declspec(dllimport) extern
#else
#define uiprivExtern extern
#endif
#endif

// C++ is really really really really really really dumb about enums, so screw that and just make them anonymous
// This has the advantage of being ABI-able should we ever need an ABI...
#define uiprivEnum(s) typedef unsigned int s; enum

typedef struct uiInitError uiInitError;

struct uiInitError {
	size_t Size;
	char Message[256];
};

uiprivExtern bool uiInit(void *options, uiInitError *err);
uiprivExtern void uiMain(void);
uiprivExtern void uiQuit(void);
uiprivExtern void uiQueueMain(void (*f)(void *data), void *data);

typedef struct uiControl uiControl;
typedef struct uiControlVtable uiControlVtable;
typedef struct uiControlOSVtable uiControlOSVtable;

uiprivExtern uint32_t uiControlType(void);
#define uiControl(obj) ((uiControl *) uiCheckControlType((obj), uiControlType()))

struct uiControlVtable {
	size_t Size;
	bool (*Init)(uiControl *c, void *implData, void *initData);
	void (*Free)(uiControl *c, void *implData);
	void (*ParentChanging)(uiControl *c, void *implData, uiControl *oldParent);
	void (*ParentChanged)(uiControl *c, void *implData, uiControl *newParent);
};

uiprivExtern uint32_t uiRegisterControlType(const char *nane, const uiControlVtable *vtable, const uiControlOSVtable *osVtable, size_t implDataSize);
uiprivExtern void *uiCheckControlType(void *c, uint32_t type);

uiprivExtern uiControl *uiNewControl(uint32_t type, void *initData);
uiprivExtern void uiControlFree(uiControl *c);
uiprivExtern void uiControlSetParent(uiControl *c, uiControl *parent);
uiprivExtern void *uiControlImplData(uiControl *c);

typedef uiControl uiWindow;
uiprivExtern uint32_t uiWindowType(void);
#define uiWindow(obj) ((uiWindow *) uiCheckControlType((obj), uiWindowType()))
// TODO provide events for window close button clicked

uiprivExtern uiWindow *uiNewWindow(void);
uiprivExtern const char *uiWindowTitle(uiWindow *w);
uiprivExtern void uiWindowSetTitle(uiWindow *w, const char *title);
uiprivExtern uiControl *uiWindowChild(uiWindow *w);
uiprivExtern void uiWindowSetChild(uiWindow *w, uiControl *child);

#ifdef __cplusplus
}
#endif

#endif
