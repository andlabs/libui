// 7 june 2020
#include "test_windows.h"

// Do not put any test cases in this file; they will not be run.

uint16_t *testGetWindowText(HWND hwnd)
{
	LRESULT n;
	WCHAR *text;

	n = SendMessageW(hwnd, WM_GETTEXTLENGTH, 0, 0);
	// WM_GETTEXTLENGTH does not include the null terminator
	text = (WCHAR *) malloc((n + 1) * sizeof (WCHAR));
	if (text == NULL)
		TestFatalf("memory exhausted allocating space for window text");
	// note the comparison: the size includes the null terminator, but the return does not
	if (GetWindowTextW(hwnd, text, n + 1) != n) {
		DWORD le;

		le = GetLastError();
		TestFatalf("error getting window text; last error %d", le);
	}
	return (uint16_t *) text;
}
