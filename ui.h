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

typedef struct uiEvent uiEvent;
typedef struct uiEventOptions uiEventOptions;

typedef void (*uiEventHandler)(void *sender, void *args, void *data);

struct uiEventOptions {
	size_t Size;
	bool Global;
};

uiprivExtern uiEvent *uiNewEvent(const uiEventOptions *options);
uiprivExtern void uiEventFree(uiEvent *e);
uiprivExtern int uiEventAddHandler(uiEvent *e, uiEventHandler handler, void *sender, void *data);
uiprivExtern void uiEventDeleteHandler(uiEvent *e, int id);
uiprivExtern void uiEventFire(uiEvent *e, void *sender, void *args);
uiprivExtern bool uiEventHandlerBlocked(const uiEvent *e, int id);
uiprivExtern void uiEventSetHandlerBlocked(uiEvent *e, int id, bool blocked);

typedef struct uiControl uiControl;
typedef struct uiControlVtable uiControlVtable;

uiprivExtern uint32_t uiControlType(void);
#define uiControl(obj) ((uiControl *) uiCheckControlType((obj), uiControlType()))

struct uiControlVtable {
	void (*Free)(uiControl *c, void *implData);
};

uiprivExtern void uiControlFree(uiControl *c);

#ifdef __cplusplus
}
#endif

#endif
