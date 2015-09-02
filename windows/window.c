// 27 april 2015
#include "uipriv_windows.h"

#define windowClass L"libui_uiWindowClass"

struct uiWindow {
	uiWindowsControl c;
	HWND hwnd;
	HMENU menubar;
	struct child *child;
	BOOL shownOnce;
	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
	int margined;
	BOOL hasMenubar;
};

static void onDestroy(uiWindow *);

uiWindowsDefineControlWithOnDestroy(
	uiWindow,							// type name
	uiWindowType,							// type function
	onDestroy(this);						// on destroy
)

static LRESULT CALLBACK windowWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ww;
	uiWindow *w;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	MINMAXINFO *mmi = (MINMAXINFO *) lParam;
	uiWindowsControl *c;
	intmax_t width, height;
	LRESULT lResult;

	ww = GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (ww == 0) {
		if (uMsg == WM_CREATE)
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) (cs->lpCreateParams));
		// fall through to DefWindowProc() anyway
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}
	w = uiWindow((void *) ww);
	if (handleParentMessages(hwnd, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;
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
		if (w->child != NULL)
			childQueueRelayout(w->child);
		return 0;
	case WM_GETMINMAXINFO:
		// ensure the user cannot resize the window smaller than its minimum size
		lResult = DefWindowProcW(hwnd, uMsg, wParam, lParam);
		if (w->child == NULL)
			return lResult;
		c = uiWindowsControl(w);
		(*(c->MinimumSize))(c, NULL, &width, &height);
		// width and height are in client coordinates; ptMinTrackSize is in window coordinates
		clientSizeToWindowSize(w->hwnd, &width, &height, w->hasMenubar);
		mmi->ptMinTrackSize.x = width;
		mmi->ptMinTrackSize.y = height;
		return lResult;
	case WM_PRINTCLIENT:
		// we do no special painting; just erase the background
		// don't worry about the return value; we let DefWindowProcW() handle this message
		SendMessageW(hwnd, WM_ERASEBKGND, wParam, lParam);
		return 0;
	case WM_CLOSE:
		if ((*(w->onClosing))(w, w->onClosingData))
			uiControlDestroy(uiControl(w));
		return 0;		// we destroyed it already
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

ATOM registerWindowClass(HICON hDefaultIcon, HCURSOR hDefaultCursor)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = windowClass;
	wc.lpfnWndProc = windowWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	return RegisterClassW(&wc);
}

void unregisterWindowClass(void)
{
	if (UnregisterClassW(windowClass, hInstance) == 0)
		logLastError("error unregistering uiWindow window class in unregisterWindowClass()");
}

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 0;
}

static void onDestroy(uiWindow *w)
{
	// first hide ourselves
	ShowWindow(w->hwnd, SW_HIDE);
	// now destroy the child
	if (w->child != NULL)
		childDestroy(w->child);
	// now free the menubar, if any
	if (w->menubar != NULL)
		freeMenubar(w->menubar);
	// and finally destroy ourselves
	dialogHelperUnregisterWindow(w->hwnd);
}

static void windowCommitShow(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	if (w->shownOnce) {
		ShowWindow(w->hwnd, SW_SHOW);
		return;
	}
	w->shownOnce = TRUE;
	// make sure the child is the correct size
	uiWindowsControlQueueRelayout(uiWindowsControl(w));
	ShowWindow(w->hwnd, nCmdShow);
	if (UpdateWindow(w->hwnd) == 0)
		logLastError("error calling UpdateWindow() after showing uiWindow for the first time in windowShow()");
}

static void windowContainerUpdateState(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	if (w->child != NULL)
		childUpdateState(w->child);
}

// from https://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define windowMargin 7

static void minimumSize(uiWindowsControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	uiWindow *w = uiWindow(c);

	*width = 0;
	*height = 0;
	d = uiWindowsNewSizing(w->hwnd);
	if (w->child != NULL)
		childMinimumSize(w->child, d, width, height);
	if (w->margined) {
		*width += 2 * uiWindowsDlgUnitsToX(windowMargin, d->BaseX);
		*height += 2 * uiWindowsDlgUnitsToY(windowMargin, d->BaseY);
	}
	uiWindowsFreeSizing(d);
}

static void windowRelayout(uiWindowsControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
	uiWindow *w = uiWindow(c);
	uiWindowsSizing *d;

	if (w->child == NULL)
		return;
	d = uiWindowsNewSizing(w->hwnd);
	if (w->margined) {
		x += uiWindowsDlgUnitsToX(windowMargin, d->BaseX);
		y += uiWindowsDlgUnitsToY(windowMargin, d->BaseY);
		width -= 2 * uiWindowsDlgUnitsToX(windowMargin, d->BaseX);
		height -= 2 * uiWindowsDlgUnitsToY(windowMargin, d->BaseY);
	}
	childRelayout(w->child, x, y, width, height);
	uiWindowsFreeSizing(d);
}

static void windowArrangeChildrenControlIDsZOrder(uiWindowsControl *c)
{
	uiWindow *w = uiWindow(c);

	if (w->child != NULL)
		childSetSoleControlID(w->child);
}

char *uiWindowTitle(uiWindow *w)
{
	return uiWindowsUtilText(w->hwnd);
}

void uiWindowSetTitle(uiWindow *w, const char *title)
{
	uiWindowsUtilSetText(w->hwnd, title);
	// don't queue resize; the caption isn't part of what affects layout and sizing of the client area (it'll be ellipsized if too long)
}

void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *, void *), void *data)
{
	w->onClosing = f;
	w->onClosingData = data;
}

void uiWindowSetChild(uiWindow *w, uiControl *child)
{
	if (w->child != NULL)
		childRemove(w->child);
	w->child = newChild(child, uiControl(w), w->hwnd);
	if (w->child != NULL) {
		childSetSoleControlID(w->child);
		childQueueRelayout(w->child);
	}
}

int uiWindowMargined(uiWindow *w)
{
	return w->margined;
}

void uiWindowSetMargined(uiWindow *w, int margined)
{
	w->margined = margined;
	uiWindowsControlQueueRelayout(uiWindowsControl(w));
}

// see http://blogs.msdn.com/b/oldnewthing/archive/2003/09/11/54885.aspx and http://blogs.msdn.com/b/oldnewthing/archive/2003/09/13/54917.aspx
static void setClientSize(uiWindow *w, int width, int height, BOOL hasMenubar, DWORD style, DWORD exstyle)
{
	RECT window;

	window.left = 0;
	window.top = 0;
	window.right = width;
	window.bottom = height;
	if (AdjustWindowRectEx(&window, style, hasMenubar, exstyle) == 0)
		logLastError("error getting real window coordinates in setClientSize()");
	if (hasMenubar) {
		RECT temp;

		temp = window;
		temp.bottom = 0x7FFF;		// infinite height
		SendMessageW(w->hwnd, WM_NCCALCSIZE, (WPARAM) FALSE, (LPARAM) (&temp));
		window.bottom += temp.top;
	}
	if (SetWindowPos(w->hwnd, NULL, 0, 0, window.right - window.left, window.bottom - window.top, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER) == 0)
		logLastError("error resizing window in setClientSize()");
}

uiWindow *uiNewWindow(const char *title, int width, int height, int hasMenubar)
{
	uiWindow *w;
	WCHAR *wtitle;
	BOOL hasMenubarBOOL;

	w = (uiWindow *) uiNewControl(uiWindowType());

	hasMenubarBOOL = FALSE;
	if (hasMenubar)
		hasMenubarBOOL = TRUE;
	w->hasMenubar = hasMenubarBOOL;

#define style WS_OVERLAPPEDWINDOW
#define exstyle 0

	wtitle = toUTF16(title);
	w->hwnd = CreateWindowExW(exstyle,
		windowClass, wtitle,
		style,
		CW_USEDEFAULT, CW_USEDEFAULT,
		// use the raw width and height for now
		// this will get CW_USEDEFAULT (hopefully) predicting well
		// even if it doesn't, we're adjusting it later
		width, height,
		NULL, NULL, hInstance, w);
	if (w->hwnd == NULL)
		logLastError("error creating window in uiWindow()");
	uiFree(wtitle);

	dialogHelperRegisterWindow(w->hwnd);

	if (hasMenubar) {
		w->menubar = makeMenubar();
		if (SetMenu(w->hwnd, w->menubar) == 0)
			logLastError("error giving menu to window in uiNewWindow()");
	}

	// and use the proper size
	setClientSize(w, width, height, hasMenubarBOOL, style, exstyle);

	uiWindowOnClosing(w, defaultOnClosing, NULL);

	uiWindowsFinishNewControl(w, uiWindow);
	uiControl(w)->CommitShow = windowCommitShow;
	uiControl(w)->ContainerUpdateState = windowContainerUpdateState;
	uiWindowsControl(w)->Relayout = windowRelayout;
	uiWindowsControl(w)->ArrangeChildrenControlIDsZOrder = windowArrangeChildrenControlIDsZOrder;

	return w;
}
