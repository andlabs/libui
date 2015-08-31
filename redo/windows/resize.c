// 14 may 2015
#include "uipriv_windows.h"

static struct ptrArray *resizes;

void initResizes(void)
{
	resizes = newPtrArray();
}

void uninitResizes(void)
{
	while (resizes->len != 0)
		ptrArrayDelete(resizes, 0);
	ptrArrayDestroy(resizes);
}

void queueResize(uiControl *c)
{
	uintmax_t i;
	uiControl *d;

	// resizing a control requires us to reocmpute the sizes of everything in the top-level window
	c = toplevelOwning(c);
	if (c == NULL)
		return;
	// make sure we're only queued once
	for (i = 0 ; i < resizes->len; i++) {
		d = ptrArrayIndex(resizes, uiControl *, i);
		if (c == d)
			return;
	}
	ptrArrayAppend(resizes, c);
}

void doResizes(void)
{
	uiWindowsControl *w;
	HWND hwnd;
	RECT r;

	while (resizes->len != 0) {
		w = ptrArrayIndex(resizes, uiWindowsControl *, 0);
		ptrArrayDelete(resizes, 0);
		hwnd = (HWND) uiControlHandle(uiControl(w));
		if (GetClientRect(hwnd, &r) == 0)
			logLastError("TODO write this");
		(*(w->Relayout))(w, r.left, r.top, r.right - r.left, r.bottom - r.top);
		// we used SWP_NOREDRAW; we need to queue a redraw ourselves
		// force all controls to be redrawn; this fixes things like the date-time picker's up-down not showing up until hovered over (and bypasses complications caused by WS_CLIPCHILDREN and WS_CLIPSIBLINGS, which we don't use but other controls might)
		if (RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN) == 0)
			logLastError("error redrawing controls after a resize in doResizes()");
	}
}

void moveWindow(HWND hwnd, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiWindowsSizing *d)
{
	RECT r;

	r.left = x;
	r.top = y;
	r.right = x + width;
	r.bottom = y + height;
	if (SetWindowPos(hwnd, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_NOZORDER) == 0)
		logLastError("error moving window in moveWindow()");
}

void setWindowInsertAfter(HWND hwnd, HWND insertAfter)
{
	if (SetWindowPos(hwnd, insertAfter, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOSIZE) == 0)
		logLastError("error reordering window in setWindowInsertAfter()");
}

// from https://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing and https://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
// this X value is really only for buttons but I don't see a better one :/
#define winXPadding 4
#define winYPadding 4

uiWindowsSizing *uiWindowsNewSizing(HWND hwnd)
{
	uiWindowsSizing *d;
	HDC dc;
	HFONT prevfont;
	TEXTMETRICW tm;
	SIZE size;

	d = uiNew(uiWindowsSizing);

	dc = GetDC(hwnd);
	if (dc == NULL)
		logLastError("error getting DC in uiWindowsNewSizing()");
	prevfont = (HFONT) SelectObject(dc, hMessageFont);
	if (prevfont == NULL)
		logLastError("error loading control font into device context in uiWindowsNewSizing()");

	ZeroMemory(&tm, sizeof (TEXTMETRICW));
	if (GetTextMetricsW(dc, &tm) == 0)
		logLastError("error getting text metrics in uiWindowsNewSizing()");
	if (GetTextExtentPoint32W(dc, L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &size) == 0)
		logLastError("error getting text extent point in uiWindowsNewSizing()");

	d->BaseX = (int) ((size.cx / 26 + 1) / 2);
	d->BaseY = (int) tm.tmHeight;
	d->InternalLeading = tm.tmInternalLeading;

	if (SelectObject(dc, prevfont) != hMessageFont)
		logLastError("error restoring previous font into device context in uiWindowsSizing()");
	if (ReleaseDC(hwnd, dc) == 0)
		logLastError("error releasing DC in uiWindowsSizing()");

	d->XPadding = uiWindowsDlgUnitsToX(winXPadding, d->BaseX);
	d->YPadding = uiWindowsDlgUnitsToY(winYPadding, d->BaseY);

	return d;
}

void uiWindowsFreeSizing(uiWindowsSizing *d)
{
	uiFree(d);
}
