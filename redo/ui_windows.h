// 7 april 2015

/*
This file assumes that you have included <windows.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls in Windows.
*/

#ifndef __UI_UI_WINDOWS_H__
#define __UI_UI_WINDOWS_H__

// uiWindowsMakeControl() initializes the given uiControl with the given Windows API control inside.
// You will need to provide the preferredSize() method yourself.
typedef struct uiWindowsMakeControlParams uiWindowsMakeControlParams;
struct uiWindowsMakeControlParams {
	// These match the CreateWindowExW() function.
	DWORD dwExStyle;
	LPCWSTR lpClassName;
	LPCWSTR lpWindowName;
	DWORD dwStyle;		// WS_CHILD and WS_VISIBLE are automatically applied.
	HINSTANCE hInstance;
	LPVOID lpParam;

	// Set this to non-FALSE to use the standard control font used by other ui controls.
	BOOL useStandardControlFont;

	// These are called when the control sends a WM_COMMAND, WM_NOTIFY, or WM_HSCROLL (respectively) to its parent.
	// ui redirects the message back and calls these functions.
	// Store the result in *lResult and return any non-FALSE value (such as TRUE) to return the given result; return FALSE to pass the notification up to your window procedure.
	// Note that these are only issued if they come from the uiControl itself; notifications from children of the uiControl (such as a header control) will be received normally.
	BOOL (*onWM_HSCROLL)(uiControl *c, WORD code, LRESULT *lResult);

	// This is called when the widget is ready to be destroyed.
	void (*onDestroy)(void *data);
	void *onDestroyData;
};
_UI_EXTERN void uiWindowsMakeControl(uiControl *c, uiWindowsMakeControlParams *p);

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
