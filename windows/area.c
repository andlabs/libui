// 8 september 2015
#include "uipriv_windows.h"
#include "area.h"

uiWindowsDefineControl(
	uiArea,								// type name
	uiAreaType							// type function
)

// I love COM interfaces that actually only work on C++
// ID2D1RenderTarget::GetSize is defined as returninig a structure
// with stdcall, this means it's an extra last argument
// the compiler tries to return it directly, and crashes
// I originally thought this was a bug in MinGW-w64, but it turns out it also affects MSVC! https://gcc.gnu.org/bugzilla/show_bug.cgi?id=64384
// So we have to work around it.
// TODO is the return type correct? or should we just use C++?
void renderTargetGetSize(ID2D1RenderTarget *rt, D2D1_SIZE_F *size)
{
	typedef void (STDMETHODCALLTYPE *fptr)(ID2D1RenderTarget *, D2D1_SIZE_F *);
	fptr f;

	f = (fptr) (rt->lpVtbl->GetSize);
	(*f)(rt, size);
}

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
		if (GetClientRect(a->hwnd, &client) == 0)
			logLastError("error getting client rect of uiArea for WM_WINDOWPOSCHANGED handling in areaWndProc()");
		areaDrawOnResize(a, &client);
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

static void minimumSize(uiWindowsControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
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
	// TODO specify CS_HREDRAW/CS_VREDRAW in addition to or instead of calling InvalidateRect(NULL) in WM_WINDOWPOSCHANGED above, or not at all?
	return RegisterClassW(&wc);
}

void unregisterArea(void)
{
	unregisterAreaFilter();
	if (UnregisterClassW(areaClass, hInstance) == 0)
		logLastError("error unregistering uiArea window class in unregisterArea()");
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
	if (InvalidateRect(a->hwnd, NULL, FALSE) == 0)
		logLastError("error queueing uiArea redraw in uiAreaQueueRedrawAll()");
}

uiArea *uiNewArea(uiAreaHandler *ah)
{
	uiArea *a;

	a = (uiArea *) uiNewControl(uiAreaType());

	a->ah = ah;
	a->scrolling = FALSE;
	clickCounterReset(&(a->cc));

	// a->hwnd is assigned in areaWndProc()
	uiWindowsEnsureCreateControlHWND(0,
		areaClass, L"",
		0,
		hInstance, a,
		FALSE);

	uiWindowsFinishNewControl(a, uiArea);

	return a;
}

uiArea *uiNewScrollingArea(uiAreaHandler *ah, intmax_t width, intmax_t height)
{
	uiArea *a;

	a = (uiArea *) uiNewControl(uiAreaType());

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

	uiWindowsFinishNewControl(a, uiArea);

	return a;
}
