// 6 april 2015
#include "uipriv_windows.h"

struct uiWindow {
	HWND hwnd;
	uiControl *child;
	BOOL shownOnce;
	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
	int margined;
};

#define uiWindowClass L"uiWindowClass"

// from https://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define windowMargin 7

static LRESULT CALLBACK uiWindowWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	uiWindow *w;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	LRESULT lResult;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	RECT r, margin;

	w = (uiWindow *) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (w == NULL) {
		if (uMsg == WM_CREATE)
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) (cs->lpCreateParams));
		// fall through to DefWindowProc() anyway
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}
	if (sharedWndProc(hwnd, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;
	switch (uMsg) {
	case WM_WINDOWPOSCHANGED:
		if ((wp->flags & SWP_NOSIZE) != 0)
			break;
		// fall through
	case msgUpdateChild:
		if (w->child == NULL)
			break;
		if (GetClientRect(w->hwnd, &r) == 0)
			logLastError("error getting window client rect for resize in uiWindowWndProc()");
		margin.left = 0;
		margin.top = 0;
		margin.right = 0;
		margin.bottom = 0;
		if (w->margined) {
			margin.left = windowMargin;
			margin.top = windowMargin;
			margin.right = windowMargin;
			margin.bottom = windowMargin;
		}
		resize(w->child, w->hwnd, r, margin);
		return 0;
	case WM_CLOSE:
		if (!(*(w->onClosing))(w, w->onClosingData))
			return 0;
		break;		// fall through to DefWindowProcW()
	case WM_DESTROY:
		if (w->child != NULL)
			uiControlDestroy(w->child);
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

#define exstyle 0
#define style WS_OVERLAPPEDWINDOW

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 1;
}

uiWindow *uiNewWindow(char *title, int width, int height)
{
	uiWindow *w;
	RECT adjust;
	WCHAR *wtitle;

	w = uiNew(uiWindow);
	w->onClosing = defaultOnClosing;

	adjust.left = 0;
	adjust.top = 0;
	adjust.right = width;
	adjust.bottom = height;
	if (AdjustWindowRectEx(&adjust, style, FALSE, exstyle) == 0)
		logLastError("error getting real window coordinates in uiWindow()");

	wtitle = toUTF16(title);
	w->hwnd = CreateWindowExW(exstyle,
		uiWindowClass, wtitle,
		style,
		CW_USEDEFAULT, CW_USEDEFAULT,
		adjust.right - adjust.left, adjust.bottom - adjust.top,
		NULL, NULL, hInstance, w);
	if (w->hwnd == NULL)
		logLastError("error creating window in uiWindow()");

	uiFree(wtitle);
	return w;
}

void uiWindowDestroy(uiWindow *w)
{
	DestroyWindow(w->hwnd);
}

uintptr_t uiWindowHandle(uiWindow *w)
{
	return (uintptr_t) (w->hwnd);
}

char *uiWindowTitle(uiWindow *w)
{
	WCHAR *wtext;
	char *text;

	wtext = windowText(w->hwnd);
	text = toUTF8(wtext);
	uiFree(wtext);
	return text;
}

void uiWindowSetTitle(uiWindow *w, const char *text)
{
	WCHAR *wtext;

	wtext = toUTF16(text);
	if (SetWindowTextW(w->hwnd, wtext) == 0)
		logLastError("error setting window title in uiWindowSetTitle()");
	uiFree(wtext);
}

void uiWindowShow(uiWindow *w)
{
	if (w->shownOnce) {
		ShowWindow(w->hwnd, SW_SHOW);
		return;
	}
	w->shownOnce = TRUE;
	ShowWindow(w->hwnd, nCmdShow);
	if (UpdateWindow(w->hwnd) == 0)
		logLastError("error calling UpdateWindow() after showing uiWindow for the first time");
}

void uiWindowHide(uiWindow *w)
{
	ShowWindow(w->hwnd, SW_HIDE);
}

void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *, void *), void *data)
{
	w->onClosing = f;
	w->onClosingData = data;
}

void uiWindowSetChild(uiWindow *w, uiControl *c)
{
	w->child = c;
	uiControlSetParent(w->child, (uintptr_t) (w->hwnd));
}

int uiWindowMargined(uiWindow *w)
{
	return w->margined;
}

void uiWindowSetMargined(uiWindow *w, int margined)
{
	w->margined = margined;
	updateParent((uintptr_t) (w->hwnd));
}
