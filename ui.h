// 6 april 2015

#ifndef uiprivIncludeGuard_ui_h
#define uiprivIncludeGuard_ui_h

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef uiprivBuildingLibui
#ifdef _WIN32
#define uiprivExtern __declspec(dllexport) extern
#else
#define uiprivExtern __attribute__((visibility("default"))) extern
#endif
#else
// TODO add __declspec(dllimport) on windows, but only if not static#define uiprivExtern extern
#endif

// C++ is really really really really really really dumb about enums, so screw that and just make them anonymous
// This has the advantage of being ABI-able should we ever need an ABI...
#define uiprivEnum(s) typedef unsigned int s; enum

#ifdef __cplusplus
}
#endif

#endif
