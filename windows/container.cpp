// 26 april 2015
#include "uipriv_windows.hpp"

// Code for the HWND of the following uiControls:
// - uiBox
// - uiRadioButtons

struct containerInit {
	void (*onResize)(void *data);
	void *data;
};

static LRESULT CALLBACK containerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	HDC dc;
	PAINTSTRUCT ps;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	struct containerInit *init;
	void (*onResize)(void *);
	void *data;
	LRESULT lResult;

	if (handleParentMessages(hwnd, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;
	switch (uMsg) {
	case WM_CREATE:
		init = (struct containerInit *) (cs->lpParam);
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) (init->onResize));
		SetWindowLongPtrW(hwnd, 0, (LONG_PTR) (init->data));
		break;		// defer to DefWindowProc()
	case WM_WINDOWPOSCHANGED:
		if ((wp->flags & SWP_NOSIZE) != 0)
			break;	// defer to DefWindowProc();
		onResize = (void (*)(void *)) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
		data = (void *) GetWindowLongPtrW(hwnd, 0);
		(*(onResize))(data);
		return 0;
	case WM_PAINT:
		dc = BeginPaint(hwnd, &ps);
		if (dc == NULL) {
			logLastError(L"error beginning container paint");
			// bail out; hope DefWindowProc() catches us
			break;
		}
		r = ps.rcPaint;
		paintContainerBackground(hwnd, dc, &r);
		EndPaint(hwnd, &ps);
		return 0;
	// tab controls use this to draw the background of the tab area
	case WM_PRINTCLIENT:
		if (GetClientRect(hwnd, &r) == 0) {
			logLastError(L"error getting client rect");
			// likewise
			break;
		}
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
	wc.cbWndExtra = sizeof (void *);
	return RegisterClassW(&wc);
}

void uninitContainer(void)
{
	if (UnregisterClassW(containerClass, hInstance) == 0)
		logLastError(L"error unregistering container window class");
}

HWND uiWindowsMakeContainer(void (*onResize)(void *data), void *data)
{
	struct containerInit init;

	// TODO onResize cannot be NULL
	init.onResize = onResize;
	init.data = data;
	return uiWindowsEnsureCreateControlHWND(WS_EX_CONTROLPARENT,
		containerClass, L"",
		0,
		hInstance, (LPVOID) (&init),
		FALSE);
}
