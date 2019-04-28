// 14 may 2015
#include "uipriv_windows.hpp"

// The utility window is a special window that performs certain tasks internal to libui.
// TODO should it be message-only?

#define utilWindowClass L"libui_utilWindowClass"

HWND uiprivUtilWindow = NULL;

static LRESULT CALLBACK utilWindowWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	void (*qf)(void *);

	switch (uMsg) {
	case uiprivUtilWindowMsgQueueMain:
		qf = (void (*)(void *)) wParam;
		(*qf)((void *) lParam);
		return 0;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

HRESULT uiprivInitUtilWindow(HICON hDefaultIcon, HCURSOR hDefaultCursor)
{
	WNDCLASSW wc;
	HRESULT hr;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = utilWindowClass;
	wc.lpfnWndProc = utilWindowWndProc;
	wc.hInstance = uipriv_hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	hr = uiprivHrRegisterClassW(&wc);
	if (hr != S_OK)
		return hr;
	return uiprivHrCreateWindowExW(0,
		utilWindowClass, L"libui utility window",
		WS_OVERLAPPEDWINDOW,
		0, 0, 100, 100,
		NULL, NULL, uipriv_hInstance, NULL,
		&uiprivUtilWindow);
}
