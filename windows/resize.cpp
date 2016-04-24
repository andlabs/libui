// 14 may 2015
#include "uipriv_windows.hpp"

static std::map<uiWindowsControl *, bool> resizes;

// TODO clicking buttons doesn't get rid of anything?
void uiWindowsControlQueueRelayout(uiWindowsControl *c)
{
	HWND hwnd;
	HWND parent;
	uintmax_t i;

	// resizing a control requires us to reocmpute the sizes of everything in the top-level window
	hwnd = (HWND) uiControlHandle(uiControl(c));
	// TODO what if this is toplevel
	parent = parentToplevel(hwnd);
	if (parent == utilWindow)		// not in a parent
		return;
	c = uiWindowsControl(SendMessageW(parent, msgGetuiWindow, 0, 0));
	resizes[c] = true;
}

void doResizes(void)
{
	uiWindowsControl *w;
	HWND hwnd;
	RECT r;

	for (const auto &iter : resizes) {
		w = iter.first;
		// don't clip content if content dynamically changed (tab page changed, etc.)
		// do this BEFORE removing w from the queue list to avoid double queueing
		ensureMinimumWindowSize(uiWindow(w));
		hwnd = (HWND) uiControlHandle(uiControl(w));
		if (GetClientRect(hwnd, &r) == 0) {
			logLastError(L"error getting uiWindow client rect");
			// give up on this one; move on to the next one
			continue;
		}
		(*(w->Relayout))(w, r.left, r.top, r.right - r.left, r.bottom - r.top);
		// we used SWP_NOREDRAW; we need to queue a redraw ourselves
		// force all controls to be redrawn; this fixes things like the date-time picker's up-down not showing up until hovered over (and bypasses complications caused by WS_CLIPCHILDREN and WS_CLIPSIBLINGS, which we don't use but other controls might)
		if (RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN) == 0)
			logLastError(L"error redrawing controls after a resize");			// and keep going anyway
	}
	// and wipe the list
	resizes.clear();
}

///////////////////////
// TODO REEVALUATE EVERYTHING BEYOND THIS POINT
// if we do have to keep, verify the error handling
///////////////////////

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
		logLastError(L"error getting DC");
	prevfont = (HFONT) SelectObject(dc, hMessageFont);
	if (prevfont == NULL)
		logLastError(L"error loading control font into device context");

	ZeroMemory(&tm, sizeof (TEXTMETRICW));
	if (GetTextMetricsW(dc, &tm) == 0)
		logLastError(L"error getting text metrics");
	if (GetTextExtentPoint32W(dc, L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &size) == 0)
		logLastError(L"error getting text extent point");

	d->BaseX = (int) ((size.cx / 26 + 1) / 2);
	d->BaseY = (int) tm.tmHeight;
	d->InternalLeading = tm.tmInternalLeading;

	if (SelectObject(dc, prevfont) != hMessageFont)
		logLastError(L"error restoring previous font into device context");
	if (ReleaseDC(hwnd, dc) == 0)
		logLastError(L"error releasing DC");

	d->XPadding = uiWindowsDlgUnitsToX(winXPadding, d->BaseX);
	d->YPadding = uiWindowsDlgUnitsToY(winYPadding, d->BaseY);

	return d;
}

void uiWindowsFreeSizing(uiWindowsSizing *d)
{
	uiFree(d);
}
