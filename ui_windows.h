// 7 april 2015

/*
This file assumes that you have included <windows.h> and "ui.h" beforehand. It provides API-specific functions for interfacing with foreign controls in Windows.
*/

#ifndef __UI_UI_WINDOWS_H__
#define __UI_UI_WINDOWS_H__

// uiWindowsNewControl() creates a new uiControl with the given Windows API control inside.
typedef struct uiWindowsNewControlParams uiWindowsNewControlParams;
struct uiWindowsNewControlParams {
	// These match the CreateWindowExW() function.
	DWORD dwExStyle;
	LPCWSTR lpClassName;
//TODO?	LPCWSTR lpWindowName;
	DWORD dwStyle;		// WS_CHILD and WS_VISIBLE are automatically applied.
	HINSTANCE hInstance;

	// These are called when the control sends a WM_COMMAND or WM_NOTIFY (respectively) to its parent.
	// ui redirects the message back and calls these functions.
	// Store the result in *lResult and return any non-FALSE value (such as TRUE) to return the given result; return FALSE to pass the notification up to your window procedure.
	// Note that these are only issued if they come from the uiControl itself; notifications from children of the uiControl (such as a header control) will be received normally.
	BOOL (*onWM_COMMAND)(uiControl *c, WPARAM wParam, LPARAM lParam, void *data, LRESULT *lResult);
	BOOL (*onWM_NOTIFY)(uiControl *c, WPARAM wParam, LPARAM lParam, void *data, LRESULT *lResult);
	// This is the data parameter to both of the above.
	void *onCommandNotifyData;

	// This function is called when ui needs to know how to rearrange controls in a window.
	// baseX and baseY are the base units used to convert between dialog units and pixels.
	// internalLeading is the internal leading of the control font.
	void (*preferredSize)(uiControl *c, int baseX, int baseY, LONG internalLeading, intmax_t *width, intmax_t *height);
};
uiControl *uiWindowsNewControl(uiWindowsNewControlParams *);

#endif
