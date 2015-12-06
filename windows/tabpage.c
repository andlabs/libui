// 30 may 2015
#include "uipriv_windows.h"

// This just defines the implementation of the tab page HWND itself.
// The actual work of hosting a tab page's control is in child.c.

// from http://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
#define tabMargin 7

void tabPageMargins(HWND hwnd, intmax_t *left, intmax_t *top, intmax_t *right, intmax_t *bottom)
{
	uiWindowsSizing *d;

	d = uiWindowsNewSizing(hwnd);
	*left = uiWindowsDlgUnitsToX(tabMargin, d->BaseX);
	*top = uiWindowsDlgUnitsToY(tabMargin, d->BaseY);
	*right = *left;
	*bottom = *top;
	uiWindowsFreeSizing(d);
}

// dummy dialog procedure; see below for details
// let's handle parent messages here to avoid needing to subclass
// TODO do we need to handle DM_GETDEFID/DM_SETDEFID here too because of the ES_WANTRETURN stuff at http://blogs.msdn.com/b/oldnewthing/archive/2007/08/20/4470527.aspx? what about multiple default buttons (TODO)?
static INT_PTR CALLBACK dlgproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;

	if (handleParentMessages(hwnd, uMsg, wParam, lParam, &lResult) != FALSE) {
		SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, (LONG_PTR) lResult);
		return TRUE;
	}
	// unthemed dialogs don't respond to WM_PRINTCLIENT
	// fortunately they don't have any special painting
	if (uMsg == WM_PRINTCLIENT) {
		// don't worry about the return value; hopefully DefWindowProcW() caught it (if not the dialog procedure itself)
		// we COULD paint the dialog background brush ourselves but meh, it works
		SendMessageW(hwnd, WM_ERASEBKGND, wParam, lParam);
		// and pretend we did nothing just so the themed dialog can still paint its content
		return FALSE;
	}
	if (uMsg == WM_INITDIALOG)
		return TRUE;
	return FALSE;
}

HWND newTabPage(void)
{
	HWND hwnd;
	HRESULT hr;

	// unfortunately this needs to be a proper dialog for EnableThemeDialogTexture() to work; CreateWindowExW() won't suffice
	hwnd = CreateDialogW(hInstance, MAKEINTRESOURCE(rcTabPageDialog),
		utilWindow, dlgproc);
	if (hwnd == NULL)
		logLastError("error creating tab page in newTabPage()");

	hr = EnableThemeDialogTexture(hwnd, ETDT_ENABLE | ETDT_USETABTEXTURE | ETDT_ENABLETAB);
	if (hr != S_OK)
		logHRESULT("error setting tab page background in newTabPage()", hr);

	// and start the tab page hidden
	ShowWindow(hwnd, SW_HIDE);

	return hwnd;
}
