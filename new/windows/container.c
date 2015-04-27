// 26 april 2015
#include "uipriv_windows.h"

#define containerClass L"libui_uiContainerClass"

HWND initialParent;

// from https://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing and https://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
// this X value is really only for buttons but I don't see a better one :/
#define winXPadding 4
#define winYPadding 4

static void resize(uiContainer *c, RECT *r)
{
	uiSizing d;
	uiSizingSys sys;
	HDC dc;
	HFONT prevfont;
	TEXTMETRICW tm;
	SIZE size;

	// TODO clean this up a bit
	size.cx = 0;
	size.cy = 0;
	ZeroMemory(&tm, sizeof (TEXTMETRICW));
	dc = GetDC(parent);
	if (dc == NULL)
		logLastError("error getting DC in resize()");
	prevfont = (HFONT) SelectObject(dc, hMessageFont);
	if (prevfont == NULL)
		logLastError("error loading control font into device context in resize()");
	if (GetTextMetricsW(dc, &tm) == 0)
		logLastError("error getting text metrics in resize()");
	if (GetTextExtentPoint32W(dc, L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &size) == 0)
		logLastError("error getting text extent point in resize()");
	sys.baseX = (int) ((size.cx / 26 + 1) / 2);
	sys.baseY = (int) tm.tmHeight;
	sys.internalLeading = tm.tmInternalLeading;
	if (SelectObject(dc, prevfont) != hMessageFont)
		logLastError("error restoring previous font into device context in resize()");
	if (ReleaseDC(parent, dc) == 0)
		logLastError("error releasing DC in resize()");
	d.xPadding = uiDlgUnitsToX(winXPadding, sys.baseX);
	d.yPadding = uiDlgUnitsToY(winYPadding, sys.baseY);
	d.sys = &sys;
	uiContainerResizeChildren(c, r.left, r.top, r.right - r.left, r.bottom - r.top, &d);
}

static LRESULT CALLBACK containerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	uiContainer *c;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	WINDOWPOSW *wp = (WINDOWPOSW *) lParam;
	RECT r;

	c = uiContainer(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
	if (c == NULL)
		if (uMsg == WM_NCCREATE)
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) (cs->lpCreateParams));
		// DO NOT RETURN DEFWINDOWPROC() HERE
		// see the next block of comments as to why
		// instead, we simply check if c == NULL again later

	switch (uMsg) {
	// these must always be run, even on the initial parent
	// why? http://blogs.msdn.com/b/oldnewthing/archive/2010/03/16/9979112.aspx
	// TODO
	// these are only run if c is not NULL
	case WM_WINDOWPOSCHANGED:
		if ((wp->flags & SWP_NOSIZE) != 0)
			break;
		// fall through
	case msgUpdateChild:
		if (c == NULL)
			break;
		if (GetClientRect(pp->hwnd, &r) == 0)
			logLastError("error getting client rect for resize in parentWndProc()");
		resize(c, &r);
		return 0;
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

const char *initContainer(void)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = containerClass;
	wc.lpfnWndProc = containerWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	if (RegisterClassW(&wc) == 0)
		return "registering uiContainer window class";

	initialParent = CreateWindowExW(0,
		uiOSContainerClass, L"",
		WS_OVERLAPPEDWINDOW,
		0, 0,
		100, 100,
		NULL, NULL, hInstance, NULL);
	if (initialParent == NULL)
		return "creating initial parent window";

	// just to be safe, disable the initial parent so it can't be interacted with accidentally
	// if this causes issues for our controls, we can remove it
	EnableWindow(initialParent, FALSE);
	return NULL;
}

void uiMakeContainer(uiContainer *c)
{
	// TODO
}
