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
	// TODO make this more robust
	ptrArrayAppend(resizes, c);
}

void doResizes(void)
{
	uiControl *c, *parent;
	intmax_t x, y, width, height;
	uiSizing d;
	uiSizngSys sys;

	while (resizes->len != 0) {
		c = ptrArrayIndex(resizes, uiControl *, 0);
		ptrArrayDelete(resizes, 0);
		parent = uiControlParent(c);
		if (parent == NULL)		// not in a parent; can't resize
			continue;			// this is for uiBox, etc.
		d.sys = &sys;
		uiControlComputeChildSizeArea(parent, &x, &y, &width, &height, &d);
		uiControlResize(c, x, y, width, height, &d);
		hwnd = (HWND) uiControlHandle(c);
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
