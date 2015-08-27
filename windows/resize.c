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

static uiControl *findToplevel(uiControl *c)
{
	for (;;) {
		if (uiIsWindow(c))
			break;
		if (uiControlParent(c) == NULL)		// not in a window
			return NULL;
		c = uiControlParent(c);
	}
	return c;
}

void queueResize(uiControl *c)
{
	uintmax_t i;
	uiControl *d;

	// resizing a control requires us to reocmpute the sizes of everything in the top-level window
	c = findToplevel(c);
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
	uiWindow *w;
	HWND hwnd;

	while (resizes->len != 0) {
		w = ptrArrayIndex(resizes, uiWindow *, 0);
		ptrArrayDelete(resizes, 0);
		uiWindowResizeChild(w);
		hwnd = (HWND) uiControlHandle(uiControl(w));
		// we used SWP_NOREDRAW; we need to queue a redraw ourselves
		// force all controls to be redrawn; this fixes things like the date-time picker's up-down not showing up until hovered over (and bypasses complications caused by WS_CLIPCHILDREN and WS_CLIPSIBLINGS, which we don't use but other controls might)
		if (RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN) == 0)
			logLastError("error redrawing controls after a resize in doResizes()");
	}
}

void moveWindow(HWND hwnd, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	RECT r;

	r.left = x;
	r.top = y;
	r.right = x + width;
	r.bottom = y + height;
	mapWindowRect(d->Sys->CoordFrom, d->Sys->CoordTo, &r);
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

uiSizing *uiWindowsSizing(uiControl *c)
{
	uiSizing *d;
	HDC dc;
	HWND hwnd;
	HFONT prevfont;
	TEXTMETRICW tm;
	SIZE size;
	uiControl *toplevel;

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

	toplevel = findToplevel(c);
	if (toplevel != NULL)
		d->Sys->CoordFrom = (HWND) uiControlHandle(toplevel);
	d->Sys->CoordTo = hwnd;

	return d;
}

void uiFreeSizing(uiSizing *d)
{
	uiFree(d->Sys);
	uiFree(d);
}
