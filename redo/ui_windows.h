// 7 april 2015

/*
This file assumes that you have included <windows.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls in Windows.
*/

#ifndef __UI_UI_WINDOWS_H__
#define __UI_UI_WINDOWS_H__

// This creates a HWND compatible with libui.
_UI_EXTERN HWND uiWindowsUtilCreateControlHWND(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, HINSTANCE hInstance, LPVOID lpParam, BOOL useStandardControlFont);

// These provide single-HWND implementations of uiControl methods you can use in yours.
_UI_EXTERN void uiWindowsUtilDestroy(HWND hwnd);
_UI_EXTERN void uiWindowsUtilSetParent(HWND hwnd, uiControl *parent);
_UI_EXTERN void uiWindowsUtilResize(HWND hwnd, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d);
_UI_EXTERN void uiWindowsUtilShow(HWND hwnd);
_UI_EXTERN void uiWindowsUtilHide(HWND hwnd);
_UI_EXTERN void uiWindowsUtilEnable(HWND hwnd);
_UI_EXTERN void uiWindowsUtilDisable(HWND hwnd);
_UI_EXTERN uintptr_t uiWindowsUtilStartZOrder(HWND hwnd);
_UI_EXTERN uintptr_t uiWindowsUtilSetZOrder(HWND hwnd, uintptr_t insertAfter);
_UI_EXTERN int uiWindowsUtilHasTabStops(HWND hwnd);

// This creates a uiControl with most uiControl methods filled out for controls that only require a single HWND.
// You must provide Handle() (which returns that HWND) and PreferredSize() yourself.
_UI_EXTERN uiControl *uiWindowsNewSingleHWNDControl(uintmax_t type);

// This contains the Windows-specific parts of the uiSizing structure.
// BaseX and BaseY are the dialog base units.
// InternalLeading is the standard control font's internal leading; labels in uiForms use this for correct Y positioning.
// CoordFrom and CoordTo are the window handles to convert coordinates passed to uiControlResize() from and to (viaa MapWindowRect()) before passing to one of the Windows API resizing functions.
struct uiSizingSys {
	int BaseX;
	int BaseY;
	LONG InternalLeading;
	HWND CoordFrom;
	HWND CoordTo;
};
// Use these in your preferredSize() implementation with baseX and baseY.
#define uiWindowsDlgUnitsToX(dlg, baseX) MulDiv((dlg), baseX, 4)
#define uiWindowsDlgUnitsToY(dlg, baseY) MulDiv((dlg), baseY, 8)
// Use this as your control's Sizing() implementation.
_UI_EXTERN uiSizing *uiWindowsSizing(uiControl *c);

// and use this if you need the text of the window width
_UI_EXTERN intmax_t uiWindowsWindowTextWidth(HWND hwnd);

// these functions get and set the window text for a single HWND
// the value returned should be freed with uiFreeText()
_UI_EXTERN char *uiWindowsUtilText(HWND);
_UI_EXTERN void uiWindowsUtilSetText(HWND, const char *);

// These provide event handling.
// For WM_COMMAND, the WORD parameter is the notification code.
// For WM_HSCROLL, the WORD parameter is the scroll operation.
extern void uiWindowsRegisterWM_COMMANDHandler(HWND, BOOL (*)(uiControl *, HWND, WORD, LRESULT *), uiControl *);
extern void uiWindowsRegisterWM_NOTIFYHandler(HWND, BOOL (*)(uiControl *, HWND, NMHDR *, LRESULT *), uiControl *);
extern void uiWindowsRegisterWM_HSCROLLHandler(HWND, BOOL (*)(uiControl *, HWND, WORD, LRESULT *), uiControl *);
extern void uiWindowsUnregisterWM_COMMANDHandler(HWND);
extern void uiWindowsUnregisterWM_NOTIFYHandler(HWND);
extern void uiWindowsUnregisterWM_HSCROLLHandler(HWND);
extern void uiWindowsRegisterReceiveWM_WININICHANGE(HWND);
extern void uiWindowsUnregisterReceiveWM_WININICHANGE(HWND);

#endif
