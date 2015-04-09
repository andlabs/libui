// 7 april 2015
#include "uipriv_windows.h"

/*
all container windows (including the message-only window, hence this is not in container_windows.c) have to call the sharedWndProc() to ensure messages go in the right place and control colors are handled properly
*/

/*
all controls that have events receive the events themselves through subclasses
to do this, all container windows (including the message-only window; see http://support.microsoft.com/default.aspx?scid=KB;EN-US;Q104069) forward WM_COMMAND to each control with this function, WM_NOTIFY with forwardNotify, etc.
*/
static LRESULT forwardCommand(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND control = (HWND) lParam;

	// don't generate an event if the control (if there is one) is unparented (a child of the initial parent window)
	if (control != NULL && IsChild(initialParent, control) == 0)
		return SendMessageW(control, msgCOMMAND, wParam, lParam);
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static LRESULT forwardNotify(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	NMHDR *nmhdr = (NMHDR *) lParam;
	HWND control = nmhdr->hwndFrom;

	// don't generate an event if the control (if there is one) is unparented (a child of the initial parent window)
	if (control != NULL && IsChild(initialParent, control) == 0)
		return SendMessageW(control, msgNOTIFY, wParam, lParam);
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

BOOL sharedWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lResult)
{
	switch (uMsg) {
	case WM_COMMAND:
		*lResult = forwardCommand(hwnd, uMsg, wParam, lParam);
		return TRUE;
	case WM_NOTIFY:
		*lResult = forwardNotify(hwnd, uMsg, wParam, lParam);
		return TRUE;
/*TODO	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
		// read-only TextFields and Textboxes are exempt
		// this is because read-only edit controls count under WM_CTLCOLORSTATIC
		if (windowClassOf((HWND) lParam, L"edit", NULL) == 0)
			if (textfieldReadOnly((HWND) lParam))
				return FALSE;
		if (SetBkMode((HDC) wParam, TRANSPARENT) == 0)
			xpanic("error setting transparent background mode to Labels", GetLastError());
		paintControlBackground((HWND) lParam, (HDC) wParam);
		*lResult = (LRESULT) hollowBrush;
		return TRUE;
*/	}
	return FALSE;
}

void resize(uiControl *control, HWND parent, RECT r)
{
	uiSizing d;
	HDC dc;
	HFONT prevfont;
	TEXTMETRICW tm;
	SIZE size;

	size.cx = 0;
	size.cy = 0;
	ZeroMemory(&tm, sizeof (TEXTMETRICW));
	dc = GetDC(parent);
	if (dc == NULL)
		logLastError("error getting DC in resize()");
	prevfont = (HFONT) SelectObject(dc, hMessageFont);
	if (prevfont == NULL)
		logLastError("error loading control font into device context in resize()");
	if (GetTextMetricsW(dc, &tm) == 0)
		logLastError("error getting text metrics in resize()");
	if (GetTextExtentPoint32W(dc, L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &size) == 0)
		logLastError("error getting text extent point in resize()");
	d.baseX = (int) ((size.cx / 26 + 1) / 2);
	d.baseY = (int) tm.tmHeight;
	d.internalLeading = tm.tmInternalLeading;
	if (SelectObject(dc, prevfont) != hMessageFont)
		logLastError("error restoring previous font into device context in resize()");
	if (ReleaseDC(parent, dc) == 0)
		logLastError("error releasing DC in resize()");
	(*(control->resize))(control, r.left, r.top, r.right - r.left, r.bottom - r.top, &d);
}

void updateParent(uintptr_t h)
{
	HWND hwnd;
	RECT r;

	if (h == 0)		// no parent
		return;
	hwnd = (HWND) h;
	// TODO is there a better way?
	if (GetWindowRect(hwnd, &r) == 0)
		logLastError("error getting window rect for dummy move in updateParent()");
	if (MoveWindow(hwnd, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE) == 0)
		logLastError("error moving window in updateParent()");
	// TODO invalidate rect?
}
