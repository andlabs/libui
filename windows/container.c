// 26 april 2015
#include "uipriv_windows.h"

// Code for containers. uiMakeContainer() creates a singleHWND of this window class.

static LRESULT CALLBACK containerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	HDC dc;
	PAINTSTRUCT ps;
	LRESULT lResult;

	if (handleParentMessages(hwnd, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;
	switch (uMsg) {
	case WM_PAINT:
		dc = BeginPaint(hwnd, &ps);
		if (dc == NULL)
			logLastError("error beginning container paint in containerWndProc()");
		r = ps.rcPaint;
		paintContainerBackground(hwnd, dc, &r);
		EndPaint(hwnd, &ps);
		return 0;
	// tab controls use this to draw the background of the tab area
	case WM_PRINTCLIENT:
		if (GetClientRect(hwnd, &r) == 0)
			logLastError("error getting client rect in containerWndProc()");
		paintContainerBackground(hwnd, (HDC) wParam, &r);
		return 0;
	case WM_ERASEBKGND:
		// avoid some flicker
		// we draw the whole update area anyway
		return 1;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

ATOM initContainer(HICON hDefaultIcon, HCURSOR hDefaultCursor)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = containerClass;
	wc.lpfnWndProc = containerWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	return RegisterClassW(&wc);
}

void uninitContainer(void)
{
	if (UnregisterClassW(containerClass, hInstance) == 0)
		logLastError("error unregistering container window class in uninitContainer()");
}

uintptr_t uiMakeContainer(uiControl *c)
{
	setSingleHWNDFuncs(c);
	return (uintptr_t) uiWindowsUtilCreateControlHWND(WS_EX_CONTROLPARENT,
		containerClass, L"",
		0,
		hInstance, NULL,
		FALSE);
}
