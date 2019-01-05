// 14 may 2015
#include "uipriv_windows.hpp"

// The utility window is a special window that performs certain tasks internal to libui.
// It is not a message-only window, and it is always hidden and disabled.
// Its roles:
// - It is the initial parent of all controls. When a control loses its parent, it also becomes that control's parent.
// - It handles WM_QUERYENDSESSION requests.
// - It handles WM_WININICHANGE and forwards the message to any child windows that request it.
// - It handles executing functions queued to run by uiQueueMain().
// TODO explain why it isn't message-only

#define utilWindowClass L"libui_utilWindowClass"

HWND utilWindow;

static LRESULT CALLBACK utilWindowWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	void (*qf)(void *);
	LRESULT lResult;
	uiprivTimer *timer;

	if (handleParentMessages(hwnd, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;
	switch (uMsg) {
	case WM_QUERYENDSESSION:
		// TODO block handler (TODO figure out if this meant the Vista-style block handler or not)
		if (uiprivShouldQuit()) {
			uiQuit();
			return TRUE;
		}
		return FALSE;
	case WM_WININICHANGE:
		issueWM_WININICHANGE(wParam, lParam);
		return 0;
	case msgQueued:
		qf = (void (*)(void *)) wParam;
		(*qf)((void *) lParam);
		return 0;
	case WM_TIMER:
		timer = (uiprivTimer *) wParam;
		if (!(*(timer->f))(timer->data)) {
			if (KillTimer(utilWindow, (UINT_PTR) timer) == 0)
				logLastError(L"error calling KillTimer() to end uiTimer() procedure");
			uiprivFreeTimer(timer);
		}
		return 0;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

const char *initUtilWindow(HICON hDefaultIcon, HCURSOR hDefaultCursor)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = utilWindowClass;
	wc.lpfnWndProc = utilWindowWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClass(&wc) == 0)
		// see init.cpp for an explanation of the =s
		return "=registering utility window class";

	utilWindow = CreateWindowExW(0,
		utilWindowClass, L"libui utility window",
		WS_OVERLAPPEDWINDOW,
		0, 0, 100, 100,
		NULL, NULL, hInstance, NULL);
	if (utilWindow == NULL)
		return "=creating utility window";
	// and just to be safe
	EnableWindow(utilWindow, FALSE);

	return NULL;
}

void uninitUtilWindow(void)
{
	if (DestroyWindow(utilWindow) == 0)
		logLastError(L"error destroying utility window");
	if (UnregisterClass(utilWindowClass, hInstance) == 0)
		logLastError(L"error unregistering utility window class");
}
