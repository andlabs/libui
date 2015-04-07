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
