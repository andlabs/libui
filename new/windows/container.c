// 26 april 2015
#include "uipriv_windows.h"

#define containerClass L"libui_uiContainerClass"

HWND initialParent;

struct container {
	HWND hwnd;
	uiContainer *parent;
};

// from https://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing and https://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
// this X value is really only for buttons but I don't see a better one :/
#define winXPadding 4
#define winYPadding 4

static void resize(uiContainer *cc, RECT *r)
{
	struct container *c = (struct container *) (cc->Internal);
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
	dc = GetDC(c->hwnd);
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
	if (ReleaseDC(c->hwnd, dc) == 0)
		logLastError("error releasing DC in resize()");
	d.xPadding = uiDlgUnitsToX(winXPadding, sys.baseX);
	d.yPadding = uiDlgUnitsToY(winYPadding, sys.baseY);
	d.sys = &sys;
	uiContainerResizeChildren(cc, r.left, r.top, r.right - r.left, r.bottom - r.top, &d);
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

// subclasses override this and call back here when all children are destroyed
static void containerDestroy(uiControl *cc)
{
	struct container *c = (struct container *) (cc->Internal);

	if (c->parent != NULL)
		complain("attempt to destroy uiContainer %p while it has a parent", cc);
	if (DestroyWindow(c->hwnd) == 0)
		logLastError("error destroying uiContainer window in containerDestroy()");
	uiFree(c);
}

static uintptr_t containerHandle(uiControl *cc)
{
	struct container *c = (struct container *) (cc->Internal);

	return (uinptr_t) (c->hwnd);
}

static void containerSetParent(uiControl *cc, uiContainer *parent)
{
	struct contianer *c = (struct container *) cc;
	uiContainer *oldparent;
	HWND newparent;

	oldparent = c->parent;
	c->parent = parent;
	newparent = initialParent;
	if (c->parent != NULL)
		newparent = (HWND) uiControlHandle(uiControl(c->parent));
	if (SetParent(c->hwnd, newparent) == 0)
		logLastError("error changing uiContainer parent in containerSetParent()");
	if (oldparent != NULL)
		uiContainerUpdate(oldparent);
	if (c->parent != NULL)
		uiContainerUpdate(c->parent);
}

static void containerResize(uiControl *cc, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	struct container *c = (struct container *) (cc->Internal);

	if (MoveWindow(c->hwnd, x, y, width, height, TRUE) == 0)
		logLastError("error resizing uiContainer in containerResize()");
}

static void containerShow(uiControl *cc)
{
	struct container *c = (struct container *) (cc->Internal);

	ShowWindow(c->hwnd, SW_SHOW);
}

static void containerHide(uiControl *cc)
{
	struct container *c = (struct container *) (cc->Internal);

	ShowWindow(c->hwnd, SW_HIDE);
}

static void containerEnable(uiControl *cc)
{
	struct container *c = (struct container *) (cc->Internal);

	EnableWindow(c->hwnd, TRUE);
}

static void containerDisable(uiControl *cc)
{
	struct container *c = (struct container *) (cc->Internal);

	EnableWindow(c->hwnd, FALSE);
}

static void containerUpdate(uiContainer *cc)
{
	struct container *c = (struct container *) (uiControl(cc)->Internal);

	SendMessageW(c->hwnd, msgUpdateChild, 0, 0);
}

void uiMakeContainer(uiContainer *cc)
{
	struct container *c;

	c = uiNew(struct container);

	c->hwnd = CreateWindowExW(0,
		containerClass, L"",
		WS_CHILD | WS_VISIBLE,
		0, 0,
		100, 100,
		initialParent, NULL, hInstance, cc);
	if (c->hwnd == NULL)
		logLastError("error creating uiContainer window in uiMakeContainer()");

	uiControl(cc)->Internal = c;
	uiControl(cc)->Destroy = containerDestroy;
	uiControl(cc)->Handle = containerHandle;
	uiControl(cc)->SetParent = containerSetParent;
	// PreferredSize() is provided by subclasses
	uiControl(cc)->Resize = containerResize;
	uiControl(cc)->Show = containerShow;
	uiControl(cc)->Hide = containerHide;
	uiControl(cc)->Enable = containerEnable;
	uiControl(cc)->Disable = containerDisable;

	// ResizeChildren() is provided by subclasses
	uiContainer(cc)->Update = containerUpdate;
}
