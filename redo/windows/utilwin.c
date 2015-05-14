// 14 may 2015
#include "uipriv_windows.h"

// The utility window is a special window that performs certain tasks internal to libui.
// It is not a message-only window, and it is always hidden and disabled.
// Its roles:
// - It is the initial parent of all controls. When a control loses its parent, it also becomes that control's parent.
// - It handles WM_QUERYENDSESSION and console end session requests.
// - It has a timer to run resizes.

#define utilWindowClass L"libui_utilWindowClass"

HWND utilWindow;

#define resizeTimerID 15				/* a safe number */
#define resizeTimerInterval 15

static LRESULT CALLBACK utilWindowWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;

	if (handleParentMessages(hwnd, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;
	switch (uMsg) {
	case WM_QUERYENDSESSION:
	case msgConsoleEndSession:
		// TODO block handler
		if (shouldQuit()) {
			uiQuit();
			return TRUE;
		}
		return FALSE;
	case WM_TIMER:
		if (wParam != resizeTimerID)
			break;
		if (SetTimer(utilWindow, resizeTimerID, resizeTimerInterval, NULL) == 0)
			logLastError("error resetting resize timer in utilWindowWndProc()");
		doResizes();
		return TODO;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

const char *initUtilWindow(HICON hDefaultIcon, HCURSOR hDefaultCursor)
{
	WNDCLASSW wc;

	ZeroMemoryW(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = utilWindowClass;
	wc.lpfnWndProc = utilWindowWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClass(&wc) == 0)
		return "registering utility window class";

	utilWindow = CreateWindowExW(0,
		utilWindowClass, L"libui utility window",
		WS_OVERLAPPEDWINDOW,
		0, 0, 100, 100,
		NULL, NULL, hInstance, NULL);
	if (utilWindow == NULL)
		return "creating utility window";
	// and just to be safe
	EnableWindow(utilWindow, FALSE);

	if (SetTimer(utilWindow, resizeTimerID, resizeTimerInterval, NULL) == 0)
		return "starting resize timer";
}

void uninitUtilWindow(void)
{
	if (KillTimer(utilWindow, resizeTimerID) == 0)
		logLastError("error stopping resize timer in uninitUtilWindow()");
	if (DestroyWindow(utilWindow) == 0)
		logLastError("error destroying utility window in uninitUtilWindow()");
	if (UnregisterClass(utilWindowClass, hInstance) == 0)
		logLastError("error unregistering utility window class in uninitUtilWindow()");
}
