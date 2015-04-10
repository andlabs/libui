// 9 april 2015
#include "uipriv_windows.h"

#define MBTWC(str, wstr, bufsiz) MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, bufsiz)

WCHAR *toUTF16(const char *str)
{
	WCHAR *wstr;
	int n;

	n = MBTWC(str, NULL, 0);
	if (n == 0)
		logLastError("error figuring out number of characters to convert to in toUTF16()");
	wstr = (WCHAR *) uiAlloc(n * sizeof (WCHAR), "WCHAR[]");
	// TODO verify return includes null terminator
	if (MBTWC(str, wstr, n) != n)
		logLastError("error converting from UTF-8 to UTF-16 in toUTF16()");
	return wstr;
}

#define WCTMB(wstr, str, bufsiz) WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, bufsiz, NULL, FALSE)

char *toUTF8(const WCHAR *wstr)
{
	char *str;
	int n;

	n = WCTMB(wstr, NULL, 0);
	if (n == 0)
		logLastError("error figuring out number of characters to convert to in toUTF8()");
	// TODO does n include the null terminator?
	str = (char *) uiAlloc((n + 1) * sizeof (char), "char[]");
	if (WCTMB(wstr, str, n + 1) != n)
		logLastError("error converting from UTF-16 to UTF-8 in toUTFF8()");
	return str;
}

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
