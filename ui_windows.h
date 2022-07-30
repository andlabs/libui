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
	HWND (*Handle)(uiControl *c, void *implData);
	HWND (*ParentHandleForChild)(uiControl *c, void *implData, uiControl *child);
	HRESULT (*SetControlPos)(uiControl *c, void *implData, const RECT *r);
};

uiprivExtern HWND uiWindowsControlHandle(uiControl *c);
uiprivExtern HWND uiWindowsControlParentHandle(uiControl *c);
uiprivExtern HRESULT uiWindowsControlSetControlPos(uiControl *c, const RECT *r);

uiprivExtern HRESULT uiWindowsSetControlHandlePos(HWND hwnd, const RECT *r);

#ifdef __cplusplus
}
#endif

#endif
