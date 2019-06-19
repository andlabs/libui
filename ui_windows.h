// 21 april 2016

/*
This file assumes that you have included <windows.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls in Windows.
*/

#ifndef uiprivIncludeGuard_ui_windows_h
#define uiprivIncludeGuard_ui_windows_h

#ifdef __cplusplus
extern "C" {
#endif

struct uiControlOSVtable {
	size_t Size;
};

#ifdef __cplusplus
}
#endif

#endif
