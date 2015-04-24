// 6 april 2015
#include "uipriv_windows.h"

struct window {
	uiWindow w;
	HWND hwnd;
	uiOSContainer *content;
	BOOL shownOnce;
	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
	int margined;
	BOOL canDestroy;
};

#define uiWindowClass L"uiWindowClass"

static LRESULT CALLBACK uiWindowWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct window *w;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	RECT r;
	HWND contenthwnd;
	const uiMenuItem *item;

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
/*TODO		item = menuIDToItem(LOWORD(wParam));
		printf("%d", item->Type);
		if (item->Type == uiMenuItemTypeCommand)
			printf(" %s", item->Name);
		printf("\n");
*/		return 0;
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

#define exstyle 0
#define style WS_OVERLAPPEDWINDOW

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 1;
}

static void windowDestroy(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	// first destroy the content
	uiOSContainerDestroy(w->content);
	// then mark that we're ready to destroy
	w->canDestroy = TRUE;
	// and finally destroy
	// TODO check for errors
	DestroyWindow(w->hwnd);
	// no need to explicitly destroy the menubar, if any; that's done automatically during window destruction
}

static uintptr_t windowHandle(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	return (uintptr_t) (w->hwnd);
}

static char *windowTitle(uiWindow *ww)
{
	struct window *w = (struct window *) ww;
	WCHAR *wtext;
	char *text;

	wtext = windowText(w->hwnd);
	text = toUTF8(wtext);
	uiFree(wtext);
	return text;
}

static void windowSetTitle(uiWindow *ww, const char *text)
{
	struct window *w = (struct window *) ww;
	WCHAR *wtext;

	wtext = toUTF16(text);
	if (SetWindowTextW(w->hwnd, wtext) == 0)
		logLastError("error setting window title in uiWindowSetTitle()");
	uiFree(wtext);
}

static void windowShow(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	if (w->shownOnce) {
		ShowWindow(w->hwnd, SW_SHOW);
		return;
	}
	w->shownOnce = TRUE;
	ShowWindow(w->hwnd, nCmdShow);
	if (UpdateWindow(w->hwnd) == 0)
		logLastError("error calling UpdateWindow() after showing uiWindow for the first time");
}

static void windowHide(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	ShowWindow(w->hwnd, SW_HIDE);
}

static void windowOnClosing(uiWindow *ww, int (*f)(uiWindow *, void *), void *data)
{
	struct window *w = (struct window *) ww;

	w->onClosing = f;
	w->onClosingData = data;
}

static void windowSetChild(uiWindow *ww, uiControl *c)
{
	struct window *w = (struct window *) ww;

	uiOSContainerSetMainControl(w->content, c);
	// don't call uiOSContainerUpdate(); instead, synthesize a resize
	// otherwise, we'll have a 0x0 content area at first
	SendMessageW(w->hwnd, msgUpdateChild, 0, 0);
}

static int windowMargined(uiWindow *ww)
{
	struct window *w = (struct window *) ww;

	return w->margined;
}

// from https://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define windowMargin 7

static void windowSetMargined(uiWindow *ww, int margined)
{
	struct window *w = (struct window *) ww;

	w->margined = margined;
	if (w->margined)
		uiOSContainerSetMargins(w->content, windowMargin, windowMargin, windowMargin, windowMargin);
	else
		uiOSContainerSetMargins(w->content, 0, 0, 0, 0);
	uiOSContainerUpdate(w->content);
}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	struct window *w;
	RECT adjust;
	WCHAR *wtitle;
	BOOL hasMenubarBOOL;
	HMENU hmenu;

	w = uiNew(struct window);
	w->onClosing = defaultOnClosing;

	hasMenubarBOOL = FALSE;
	if (hasMenubar)
		hasMenubarBOOL = TRUE;

	adjust.left = 0;
	adjust.top = 0;
	adjust.right = width;
	adjust.bottom = height;
	// TODO does not handle menu wrapping; see http://blogs.msdn.com/b/oldnewthing/archive/2003/09/11/54885.aspx
	if (AdjustWindowRectEx(&adjust, style, hasMenubarBOOL, exstyle) == 0)
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

	w->content = uiNewOSContainer((uintptr_t) (w->hwnd));

	if (hasMenubar) {
		hmenu = makeMenubar();
		if (SetMenu(w->hwnd, hmenu) == 0)
			logLastError("error giving menu to window in uiNewWindow()");
	}

	uiWindow(w)->Destroy = windowDestroy;
	uiWindow(w)->Handle = windowHandle;
	uiWindow(w)->Title = windowTitle;
	uiWindow(w)->SetTitle = windowSetTitle;
	uiWindow(w)->Show = windowShow;
	uiWindow(w)->Hide = windowHide;
	uiWindow(w)->OnClosing = windowOnClosing;
	uiWindow(w)->SetChild = windowSetChild;
	uiWindow(w)->Margined = windowMargined;
	uiWindow(w)->SetMargined = windowSetMargined;

	return uiWindow(w);
}
