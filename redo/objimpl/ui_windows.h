// 7 april 2015

/*
This file assumes that you have included <windows.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls in Windows.
*/

#ifndef __UI_UI_WINDOWS_H__
#define __UI_UI_WINDOWS_H__

// TODO write comments for these
_UI_EXTERN HWND uiWindowsUtilCreateControlHWND(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, HINSTANCE hInstance, LPVOID lpParam, BOOL useStandardControlFont);
_UI_EXTERN void uiWindowsUtilDestroy(HWND hwnd);
_UI_EXTERN void uiWindowsUtilSetParent(HWND hwnd, uiControl *parent);
_UI_EXTERN void uiWindowsUtilResize(HWND hwnd, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d);
_UI_EXTERN void uiWIndowsUtilShow(HWND hwnd);
_UI_EXTERN void uiWindowsUtilHide(HWND hwnd);
_UI_EXTERN void uiWIndowsUtilEnable(HWND hwnd);
_UI_EXTERN void uiWindowsUtilDisable(HWND hwnd);
_UI_EXTERN void uiWindowsUtilSysFunc(HWND hwnd, uiControlSysFuncParams *p);
_UI_EXTERN void uiWindowsUtilStartZOrder(HWND hwnd, uiControlSysFuncParams *p);
_UI_EXTERN void uiWindowsMakeSingleHWNDControl(uiControl *c, uintmax_t type);

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
extern uiSizing *uiWindowsSizing(uiControl *);

// and use this if you need the text of the window width
_UI_EXTERN intmax_t uiWindowsWindowTextWidth(HWND hwnd);

// these functions get and set the window text for such a uiControl
// the value returned should be freed with uiFreeText()
_UI_EXTERN char *uiWindowsControlText(uiControl *);
_UI_EXTERN void uiWindowsControlSetText(uiControl *, const char *);

struct uiControlSysFuncParams {
	int Func;
	BOOL HasTabStops;
	HWND InsertAfter;
};

enum {
	// These should enable and disable the uiControl while preserving the user enable/disable setting.
	// These are needed because while disabling a parent window does cause children to stop receiving events, they are not shown as disabled, which is not what we want.
	uiWindowsSysFuncContainerEnable,
	uiWindowsSysFuncContainerDisable,
	// This is interpreted by controls that are tab stops; the control should set HasTabStops to TRUE if so, and *LEAVE IT ALONE* if not.
	// You only need this if implementing your own uiControl.
	// Controls created with uiWindowsMakeControl() check for the window being enabled and the presence of WS_TABSTOP.
	// The name is "has tab stops" because it is used by uiTabs to say "does the current tab page have tab stops?".
	uiWindowsSysFuncHasTabStops,
	// This tells the current control to set its Z order to be after the control in the InsertAfter field.
	// You should also set your own handle to the InsertAfter field for the next control.
	uiWindowsSysFuncSetZOrder,
};

#endif
