// 27 april 2015
#include "uipriv_windows.h"

int binHasOSParent(uiBin *b)
{
	HWND hwnd;

	hwnd = (HWND) uiControlHandle(uiControl(b));
	return GetAncestor(hwnd, GA_PARENT) != initialParent;
}

void binSetOSParent(uiBin *b, uintptr_t osParent)
{
	HWND hwnd;
	HWND parent = (HWND) osParent;

	hwnd = (HWND) uiControlHandle(uiControl(b));
	if (SetParent(hwnd, parent) == 0)
		logLastError("error setting bin OS parent in binSetOSParent()");
}

void binRemoveOSParent(uiBin *b)
{
	binSetOSParent(b, (uintptr_t) initialParent);
}

void binResizeRootAndUpdate(uiBin *b, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
	HWND hwnd;

	hwnd = (HWND) uiControlHandle(uiControl(b));
	moveWindow(hwnd, x, y, width, height);
	uiContainerUpdate(uiContainer(b));
}

void binTranslateMargins(uiBin *b, intmax_t *left, intmax_t *top, intmax_t *right, intmax_t *bottom, uiSizing *d)
{
	*left = uiWindowsDlgUnitsToX(*left, d->Sys->BaseX);
	*top = uiWindowsDlgUnitsToY(*top, d->Sys->BaseY);
	*right = uiWindowsDlgUnitsToX(*right, d->Sys->BaseX);
	*bottom = uiWindowsDlgUnitsToY(*bottom, d->Sys->BaseY);
}
