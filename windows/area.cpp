// 8 september 2015
#include "uipriv_windows.hpp"
#include "area.hpp"

static LRESULT CALLBACK areaWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	uiArea *a;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	RECT client;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	LRESULT lResult;

	a = (uiArea *) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (a == NULL) {
		if (uMsg == WM_CREATE) {
			a = (uiArea *) (cs->lpCreateParams);
			// assign a->hwnd here so we can use it immediately
			a->hwnd = hwnd;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) a);
		}
		// fall through to DefWindowProcW() anyway
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	// always recreate the render target if necessary
	if (a->rt == NULL)
		a->rt = makeHWNDRenderTarget(a->hwnd);

	if (areaDoDraw(a, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;

	if (uMsg == WM_WINDOWPOSCHANGED) {
		if ((wp->flags & SWP_NOSIZE) != 0)
			return DefWindowProcW(hwnd, uMsg, wParam, lParam);
		uiWindowsEnsureGetClientRect(a->hwnd, &client);
		areaDrawOnResize(a, &client);
		areaScrollOnResize(a, &client);
		return 0;
	}

	if (areaDoScroll(a, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;
	if (areaDoEvents(a, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;

	// nothing done
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// control implementation

uiWindowsControlAllDefaults(uiArea)

static void uiAreaMinimumSize(uiWindowsControl *c, intmax_t *width, intmax_t *height)
{
	// TODO
	*width = 0;
	*height = 0;
}

ATOM registerAreaClass(HICON hDefaultIcon, HCURSOR hDefaultCursor)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = areaClass;
	wc.lpfnWndProc = areaWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	// this is just to be safe; see the InvalidateRect() call in the WM_WINDOWPOSCHANGED handler for more details
	wc.style = CS_HREDRAW | CS_VREDRAW;
	return RegisterClassW(&wc);
}

void unregisterArea(void)
{
	if (UnregisterClassW(areaClass, hInstance) == 0)
		logLastError(L"error unregistering uiArea window class");
}

void uiAreaSetSize(uiArea *a, intmax_t width, intmax_t height)
{
	a->scrollWidth = width;
	a->scrollHeight = height;
	areaUpdateScroll(a);
}

void uiAreaQueueRedrawAll(uiArea *a)
{
	// don't erase the background; we do that ourselves in doPaint()
	invalidateRect(a->hwnd, NULL, FALSE);
}

void uiAreaScrollTo(uiArea *a, double x, double y, double width, double height)
{
	// TODO
}

uiArea *uiNewArea(uiAreaHandler *ah)
{
	uiArea *a;

	uiWindowsNewControl(uiArea, a);

	a->ah = ah;
	a->scrolling = FALSE;
	clickCounterReset(&(a->cc));

	// a->hwnd is assigned in areaWndProc()
	uiWindowsEnsureCreateControlHWND(0,
		areaClass, L"",
		0,
		hInstance, a,
		FALSE);

	return a;
}

uiArea *uiNewScrollingArea(uiAreaHandler *ah, intmax_t width, intmax_t height)
{
	uiArea *a;

	uiWindowsNewControl(uiArea, a);

	a->ah = ah;
	a->scrolling = TRUE;
	a->scrollWidth = width;
	a->scrollHeight = height;
	clickCounterReset(&(a->cc));

	// a->hwnd is assigned in areaWndProc()
	uiWindowsEnsureCreateControlHWND(0,
		areaClass, L"",
		WS_HSCROLL | WS_VSCROLL,
		hInstance, a,
		FALSE);

	// set initial scrolling parameters
	areaUpdateScroll(a);

	return a;
}
