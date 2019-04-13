// 6 april 2015

#ifndef uiprivIncludeGuard_ui_h
#define uiprivIncludeGuard_ui_h

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef uiprivBuildingLibui
#if defined(_WIN32) && !defined(uiStatic)
#define uiprivExtern __declspec(dllexport) extern
#elif deffined(_WIN32)
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

uiprivExtern int uiInit(void *options, uiInitError *err);
uiprivExtern void uiUninit(void);

#ifdef __cplusplus
}
#endif

#endif
