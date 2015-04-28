// 6 april 2015
#include "uipriv_windows.h"

static LRESULT CALLBACK uiWindowWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct window *w;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	RECT r;
	HWND contenthwnd;

	w = (struct window *) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (w == NULL) {
		if (uMsg == WM_CREATE)
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) (cs->lpCreateParams));
		// fall through to DefWindowProc() anyway
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}
	switch (uMsg) {
	case WM_COMMAND:
		// not a menu
		if (lParam != 0)
			break;
		if (HIWORD(wParam) != 0)
			break;
		runMenuEvent(LOWORD(wParam), uiWindow(w));
		return 0;
	case WM_WINDOWPOSCHANGED:
		if ((wp->flags & SWP_NOSIZE) != 0)
			break;
		// fall through
	case msgUpdateChild:
		if (GetClientRect(w->hwnd, &r) == 0)
			logLastError("error getting window client rect for resize in uiWindowWndProc()");
		contenthwnd = uiOSContainerHWND(w->content);
		if (MoveWindow(contenthwnd, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE) == 0)
			logLastError("error resizing window content parent in uiWindowWndProc()");
		return 0;
	case WM_CLOSE:
		if (!(*(w->onClosing))(uiWindow(w), w->onClosingData))
			uiWindowDestroy(uiWindow(w));
		return 0;		// we destroyed it already
	case WM_DESTROY:
		if (!w->canDestroy)
			complain("attempt to destroy uiWindow at %p before uiWindowDestroy()", w);
		uiFree(w);
		break;		// fall through to DefWindowProcW()
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

ATOM registerWindowClass(HICON hDefaultIcon, HCURSOR hDefaultCursor)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = uiWindowClass;
	wc.lpfnWndProc = uiWindowWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	return RegisterClassW(&wc);
}
