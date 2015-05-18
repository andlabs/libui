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
	for (;;) {
		if (uiIsWindow(c))
			break;
		if (uiControlParent(c) == NULL)		// not in a window; don't bother resizing
			return;
		c = uiControlParent(c);
	}
	// make sure we're only queued once
	for (i = 0 ; i < resizes->len; i++) {
		d = ptrArrayIndex(resizes, uiControl *, i);
		if (c == d)
			return;
	}
	ptrArrayAppend(resizes, c);
}

// TODO dequeueResize

void doResizes(void)
{
	uiWindow *w;
	HWND hwnd;

	while (resizes->len != 0) {
		w = ptrArrayIndex(resizes, uiWindow *, 0);
		ptrArrayDelete(resizes, 0);
		uiWindowResizeChild(w);
		hwnd = (HWND) uiControlHandle(uiControl(w));
		// we used SWP_NOREDRAW; we need to queue a redraw ourselves
		// TODO use RedrawWindow() to bypass WS_CLIPCHILDREN complications
		if (InvalidateRect(hwnd, NULL, TRUE) == 0)
			logLastError("error redrawing controls after a resize in doResizes()");
	}
}

#define swpflags (SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW)

void moveWindow(HWND hwnd, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
	if (SetWindowPos(hwnd, NULL, x, y, width, height, swpflags | SWP_NOZORDER) == 0)
		logLastError("error moving window in moveWindow()");
}

void moveAndReorderWindow(HWND hwnd, HWND insertAfter, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
	if (SetWindowPos(hwnd, insertAfter, x, y, width, height, swpflags) == 0)
		logLastError("error moving and reordering window in moveAndReorderWindow()");
}

// from https://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing and https://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
// this X value is really only for buttons but I don't see a better one :/
#define winXPadding 4
#define winYPadding 4

uiSizing *uiWindowsSizing(uiControl *c)
{
	uiSizing *d;
	HWND hwnd;
	HDC dc;
	HFONT prevfont;
	TEXTMETRICW tm;
	SIZE size;

	d = uiNew(uiSizing);
	d->Sys = uiNew(uiSizingSys);

	hwnd = (HWND) uiControlHandle(c);

	dc = GetDC(hwnd);
	if (dc == NULL)
		logLastError("error getting DC in uiWindowsSizing()");
	prevfont = (HFONT) SelectObject(dc, hMessageFont);
	if (prevfont == NULL)
		logLastError("error loading control font into device context in uiWindowsSizing()");

	ZeroMemory(&tm, sizeof (TEXTMETRICW));
	if (GetTextMetricsW(dc, &tm) == 0)
		logLastError("error getting text metrics in uiWindowsSizing()");
	if (GetTextExtentPoint32W(dc, L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &size) == 0)
		logLastError("error getting text extent point in uiWindowsSizing()");

	d->Sys->BaseX = (int) ((size.cx / 26 + 1) / 2);
	d->Sys->BaseY = (int) tm.tmHeight;
	d->Sys->InternalLeading = tm.tmInternalLeading;

	if (SelectObject(dc, prevfont) != hMessageFont)
		logLastError("error restoring previous font into device context in uiWindowsSizing()");
	if (ReleaseDC(hwnd, dc) == 0)
		logLastError("error releasing DC in uiWindowsSizing()");

	d->XPadding = uiWindowsDlgUnitsToX(winXPadding, d->Sys->BaseX);
	d->YPadding = uiWindowsDlgUnitsToY(winYPadding, d->Sys->BaseY);
}

void uiFreeSizing(uiSizing *d)
{
	uiFree(d->Sys);
	uiFree(d);
}
