// 6 april 2015
#include "uipriv_windows.h"

intmax_t uiWindowsWindowTextWidth(HWND hwnd)
{
	int len;
	WCHAR *text;
	HDC dc;
	HFONT prevfont;
	SIZE size;

	size.cx = 0;
	size.cy = 0;
	// TODO check for error
	len = GetWindowTextLengthW(hwnd);
	if (len == 0)		// no text; nothing to do
		return 0;
	text = (WCHAR *) uiAlloc((len + 1) * sizeof (WCHAR), "WCHAR[]");
	if (GetWindowText(hwnd, text, len + 1) == 0)		// should only happen on error given explicit test for len == 0 above
		logLastError("error getting window text in uiWindowsWindowTextWidth()");
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
