// 6 april 2015
#include "uipriv_windows.h"

#define MBTWC(str, wstr, bufsiz) MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, bufsiz)

WCHAR *toUTF16(const char *str)
{
	WCHAR *wstr;
	int n;

	n = MBTWC(str, NULL, 0);
	if (n == 0)
		logLastError("error figuring out number of characters to convert to in toUTF16()");
	wstr = (WCHAR *) uiAlloc(n * sizeof (WCHAR));
	if (MBTWC(str, wstr, n) != n)
		logLastError("error converting from UTF-8 to UTF-16 in toUTF16()");
	return wstr;
}

// TODO this and resize(): initialize size and other values to avoid garbage on failure
intmax_t uiWindowsWindowTextWidth(HWND hwnd)
{
	int len;
	WCHAR *text;
	HDC dc;
	HFONT prevfont;

	// TODO check for error
	len = GetWindowTextLengthW(hwnd);
	if (len == 0)		// no text; nothing to do
		return 0;
	text = (WCHAR *) uiAlloc((len + 1) * sizeof (WCHAR));
	if (GetWindowText(hwnd, text, len + 1) == 0)		// should only happen on error given explicit test for len == 0 above
		logLastError("error getting window text in uiWindowsWindowTextWidth()");
	dc = GetDC(parent);
	if (dc == NULL)
		logLastError("error getting DC in uiWindowsWindowTextWidth()");
	prevFont = (HFONT) SelectObject(dc, hMessageFont);
	if (prevFont == NULL)
		logLastError("error loading control font into device context in uiWindowsWindowTextWidth()");
	if (GetTextExtentPoint32W(dc, text, len, &size) == 0)
		logLastError("error getting text extent point in uiWindowsWindowTextWidth()");
	if (SelectObject(dc, prevFont) != hMessageFont)
		logLastError("error restoring previous font into device context in uiWindowsWindowTextWidth()");
	if (ReleaseDC(parent, dc) == 0)
		logLastError("error releasing DC in uiWindowsWindowTextWidth()");
	uiFree(text);
	return size.cx;
}
