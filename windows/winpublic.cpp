// 6 april 2015
#include "uipriv_windows.hpp"

void uiWindowsEnsureDestroyWindow(HWND hwnd)
{
	if (DestroyWindow(hwnd) == 0)
		logLastError(L"error destroying window");
}

void uiWindowsEnsureSetParentHWND(HWND hwnd, HWND parent)
{
	if (parent == NULL)
		parent = utilWindow;
	if (SetParent(hwnd, parent) == 0)
		logLastError(L"error setting window parent");
}

void uiWindowsEnsureAssignControlIDZOrder(HWND hwnd, LONG_PTR controlID, HWND insertAfter)
{
	SetWindowLongPtrW(hwnd, GWLP_ID, controlID);
	setWindowInsertAfter(hwnd, insertAfter);
}

void uiWindowsEnsureMoveWindowDuringResize(HWND hwnd, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
	RECT r;

	r.left = x;
	r.top = y;
	r.right = x + width;
	r.bottom = y + height;
	if (SetWindowPos(hwnd, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER) == 0)
		logLastError(L"error moving window");
}
