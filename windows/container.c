// 26 april 2015
#include "uipriv_windows.h"


static LRESULT CALLBACK containerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	uiContainer *cc;
	struct container *c;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	HWND control;
	NMHDR *nm = (NMHDR *) lParam;
	RECT r;
	HDC dc;
	PAINTSTRUCT ps;

	cc = uiContainer(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
	if (cc == NULL)
		if (uMsg == WM_NCCREATE)
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) (cs->lpCreateParams));
		// DO NOT RETURN DEFWINDOWPROC() HERE
		// see the next block of comments as to why
		// instead, we simply check if c == NULL again later

	switch (uMsg) {

	// these are only run if c is not NULL

	case WM_PAINT:
		if (cc == NULL)
			break;
		c = (struct container *) (uiControl(cc)->Internal);
		dc = BeginPaint(c->hwnd, &ps);
		if (dc == NULL)
			logLastError("error beginning container paint in containerWndProc()");
		r = ps.rcPaint;
		paintContainerBackground(c->hwnd, dc, &r);
		EndPaint(c->hwnd, &ps);
		return 0;
	// tab controls use this to draw the background of the tab area
	case WM_PRINTCLIENT:
		if (cc == NULL)
			break;
		c = (struct container *) (uiControl(cc)->Internal);
		if (GetClientRect(c->hwnd, &r) == 0)
			logLastError("error getting client rect in containerWndProc()");
		paintContainerBackground(c->hwnd, (HDC) wParam, &r);
		return 0;
	case WM_ERASEBKGND:
		// avoid some flicker
		// we draw the whole update area anyway
		return 1;
	case msgUpdateChild:
		if (cc == NULL)
			break;
		c = (struct container *) (uiControl(cc)->Internal);
		if (GetClientRect(c->hwnd, &r) == 0)
			logLastError("error getting client rect for resize in containerWndProc()");
		resize(cc, &r);
		// we used SWP_NOREDRAW for each resize so we can do this here
		if (InvalidateRect(c->hwnd, NULL, TRUE) == 0)
			logLastError("error queueing redraw after resize in containerWndProc()");
		return 0;

	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
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

	return (uintptr_t) (c->hwnd);
}

static void containerSetParent(uiControl *cc, uiContainer *parent)
{
	struct container *c = (struct container *) (cc->Internal);
	uiContainer *oldparent;
	HWND newparent;

	oldparent = c->parent;
	c->parent = parent;
	newparent = initialParent;
	if (c->parent != NULL)
		newparent = (HWND) uiControlHandle(uiControl(c->parent));
	if (SetParent(c->hwnd, newparent) == 0)
		logLastError("error changing uiContainer parent in containerSetParent()");
}

static void containerResize(uiControl *cc, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	struct container *c = (struct container *) (cc->Internal);

	moveAndReorderWindow(c->hwnd, d->Sys->InsertAfter, x, y, width, height);
	d->Sys->InsertAfter = c->hwnd;
	SendMessageW(c->hwnd, msgUpdateChild, 0, 0);
}

static int containerVisible(uiControl *cc)
{
	struct container *c = (struct container *) (cc->Internal);

	return !c->hidden;
}

static void containerShow(uiControl *cc)
{
	struct container *c = (struct container *) (cc->Internal);

	ShowWindow(c->hwnd, SW_SHOW);
	// hidden controls don't count in boxes and grids
	c->hidden = 0;
	if (c->parent != NULL)
		uiContainerUpdate(c->parent);
}

static void containerHide(uiControl *cc)
{
	struct container *c = (struct container *) (cc->Internal);

	ShowWindow(c->hwnd, SW_HIDE);
	c->hidden = 1;
	if (c->parent != NULL)
		uiContainerUpdate(c->parent);
}

static void containerEnable(uiControl *cc)
{
	struct container *c = (struct container *) (cc->Internal);
	uiControlSysFuncParams p;

	EnableWindow(c->hwnd, TRUE);
	p.Func = uiWindowsSysFuncContainerEnable;
	uiControlSysFunc(cc, &p);
}

static void containerDisable(uiControl *cc)
{
	struct container *c = (struct container *) (cc->Internal);
	uiControlSysFuncParams p;

	EnableWindow(c->hwnd, FALSE);
	p.Func = uiWindowsSysFuncContainerDisable;
	uiControlSysFunc(cc, &p);
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

	c->hwnd = CreateWindowExW(WS_EX_CONTROLPARENT,
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
	uiControl(cc)->Visible = containerVisible;
	uiControl(cc)->Show = containerShow;
	uiControl(cc)->Hide = containerHide;
	uiControl(cc)->Enable = containerEnable;
	uiControl(cc)->Disable = containerDisable;

	// ResizeChildren() is provided by subclasses
	uiContainer(cc)->Update = containerUpdate;
}
