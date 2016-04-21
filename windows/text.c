// 9 april 2015
#include "uipriv_windows.h"


WCHAR *windowText(HWND hwnd)
{
	LRESULT n;
	WCHAR *text;

	n = SendMessageW(hwnd, WM_GETTEXTLENGTH, 0, 0);
	// WM_GETTEXTLENGTH does not include the null terminator
	text = (WCHAR *) uiAlloc((n + 1) * sizeof (WCHAR), "WCHAR[]");
	// note the comparison: the size includes the null terminator, but the return does not
	if (GetWindowTextW(hwnd, text, n + 1) != n)
		logLastError("error getting window text in windowText()");
	return text;
}

void uiFreeText(char *text)
{
	uiFree(text);
}

intmax_t uiWindowsWindowTextWidth(HWND hwnd)
{
	LRESULT len;
	WCHAR *text;
	HDC dc;
	HFONT prevfont;
	SIZE size;

	size.cx = 0;
	size.cy = 0;

	// first we need the window text
	// this copies what windowText() does because we need the len
	// we could replace this with a call to windowText() but then we'd also need a call to wcslen() (or some other function that both this and windowText() would call to do the real work)
	len = SendMessageW(hwnd, WM_GETTEXTLENGTH, 0, 0);
	if (len == 0)		// no text; nothing to do
		return 0;
	text = (WCHAR *) uiAlloc((len + 1) * sizeof (WCHAR), "WCHAR[]");
	// note the comparison: the size includes the null terminator, but the return does not
	if (GetWindowText(hwnd, text, len + 1) != len)
		logLastError("error getting window text in uiWindowsWindowTextWidth()");

	// now we can do the calculations
	dc = GetDC(hwnd);
	if (dc == NULL)
		logLastError("error getting DC in uiWindowsWindowTextWidth()");
	prevfont = (HFONT) SelectObject(dc, hMessageFont);
	if (prevfont == NULL)
		logLastError("error loading control font into device context in uiWindowsWindowTextWidth()");
	if (GetTextExtentPoint32W(dc, text, len, &size) == 0)
		logLastError("error getting text extent point in uiWindowsWindowTextWidth()");
	if (SelectObject(dc, prevfont) != hMessageFont)
		logLastError("error restoring previous font into device context in uiWindowsWindowTextWidth()");
	if (ReleaseDC(hwnd, dc) == 0)
		logLastError("error releasing DC in uiWindowsWindowTextWidth()");
	uiFree(text);

	return size.cx;
}
