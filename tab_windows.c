// 12 april 2015
#include "uipriv_windows.h"

struct tab {
	struct tabPage *pages;
	uintmax_t len;
	uintmax_t cap;
};

struct tabPage {
	uiControl *child;
};

static BOOL onWM_COMMAND(uiControl *c, WORD code, LRESULT *lResult)
{
	return FALSE;
}

// we have to handle hiding and showing of tab pages ourselves
static BOOL onWM_NOTIFY(uiControl *c, NMHDR *nm, LRESULT *lResult)
{
	struct tab *t = (struct tab *) (c->data);
	LRESULT n;

	switch (nm->code) {
	case TCN_SELCHANGING:
		n = SendMessageW((HWND) uiControlHandle(c), TCM_GETCURSEL, 0, 0);
		if (n != (LRESULT) (-1))		// if we're changing to a real tab
			uiControlContainerHide(t->pages[n].child);
		*lResult = FALSE;			// and allow the change
		return TRUE;
	case TCN_SELCHANGE:
		n = SendMessageW((HWND) uiControlHandle(c), TCM_GETCURSEL, 0, 0);
		if (n != (LRESULT) (-1)) {		// if we're changing to a real tab
			uiControlContainerShow(t->pages[n].child);
			// because we only resize the current child on resize, we'll need to trigger an update here
			updateParent(uiControlHandle(c));
		}
		*lResult = 0;
		return TRUE;
	}
	return FALSE;
}

static void onWM_DESTROY(uiControl *c)
{
	struct tab *t = (struct tab *) (c->data);

	uiFree(t->pages);
	uiFree(t);
}

static void preferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	// TODO
}

// common code for resizes
static void resizeTab(uiControl *c, LONG width, LONG height)
{
	struct tab *t = (struct tab *) (c->data);
	HWND hwnd;
	LRESULT n;
	RECT r, margin;

	hwnd = (HWND) uiControlHandle(c);

	n = SendMessageW(hwnd, TCM_GETCURSEL, 0, 0);
	if (n == (LRESULT) (-1))		// no child selected; do nothing
		return;

	// make a rect at (0, 0) of the given window size
	// this should give us the correct client coordinates
	r.left = 0;
	r.top = 0;
	r.right = width;
	r.bottom = height;
	// convert to the display rectangle
	SendMessageW(hwnd, TCM_ADJUSTRECT, FALSE, (LPARAM) (&r));

	margin.left = 0;
	margin.top = 0;
	margin.right = 0;
	margin.bottom = 0;
/*TODO	if (w->margined) {
		margin.left = windowMargin;
		margin.top = windowMargin;
		margin.right = windowMargin;
		margin.bottom = windowMargin;
	}
*/	resize(t->pages[n].child, hwnd, r, margin);
}

// and finally, because we are a container, we have to handle resizes and updates
static LRESULT CALLBACK tabSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	uiControl *c = (uiControl *) dwRefData;
	LRESULT lResult;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	RECT r;

	if (sharedWndProc(hwnd, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;
	switch (uMsg) {
	case WM_WINDOWPOSCHANGED:
		if ((wp->flags & SWP_NOSIZE) != 0)
			break;
		// first, let the tab control handle it
		lResult = (*fv_DefSubclassProc)(hwnd, uMsg, wParam, lParam);
		// we have the window rect width as part of the WINDOWPOS; resize
		resizeTab(c, wp->cx, wp->cy);
		return lResult;
	case msgUpdateChild:
		if (GetWindowRect((HWND) uiControlHandle(c), &r) == 0)
			logLastError("error getting window rect for Tab resize in tabSubProc()");
		// though these are in screen coordinates, we only need the width and height
		resizeTab(c, r.right - r.left, r.bottom - r.top);
		return 0;
	case WM_NCDESTROY:
		if ((*fv_RemoveWindowSubclass)(hwnd, tabSubProc, uIdSubclass) == FALSE)
			logLastError("error removing Tab resize handling subclass in tabSubProc()");
		break;
	}
	return (*fv_DefSubclassProc)(hwnd, uMsg, wParam, lParam);
}

uiControl *uiNewTab(void)
{
	uiControl *c;
	struct tab *t;
	uiWindowsNewControlParams p;
	HWND hwnd;

	p.dwExStyle = 0;		// don't set WS_EX_CONTROLPARENT yet; we do that dynamically in the message loop (see main_windows.c)
	p.lpClassName = WC_TABCONTROLW;
	p.lpWindowName = L"";
	p.dwStyle = TCS_TOOLTIPS | WS_TABSTOP;
	p.hInstance = hInstance;
	p.useStandardControlFont = TRUE;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onWM_DESTROY = onWM_DESTROY;
	c = uiWindowsNewControl(&p);

	c->preferredSize = preferredSize;

	t = uiNew(struct tab);
	c->data = t;

	hwnd = (HWND) uiControlHandle(c);
	if ((*fv_SetWindowSubclass)(hwnd, tabSubProc, 0, (DWORD_PTR) c) == FALSE)
		logLastError("error subclassing Tab to give it its own resize handler in uiNewTab()");

	return c;
}

#define tabCapGrow 32

void uiTabAddPage(uiControl *c, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) (c->data);
	HWND hwnd;
	TCITEMW item;
	LRESULT n;
	WCHAR *wname;

	if (t->len >= t->cap) {
		t->cap += tabCapGrow;
		t->pages = (struct tabPage *) uiRealloc(t->pages, t->cap * sizeof (struct tabPage), "struct tabPage[]");
	}

	hwnd = (HWND) uiControlHandle(c);
	n = SendMessageW(hwnd, TCM_GETITEMCOUNT, 0, 0);

	t->pages[t->len].child = child;
	uiControlSetParent(t->pages[t->len].child, (uintptr_t) hwnd);
	if (n != 0)		// if this isn't the first page, we have to hide the other controls
		uiControlContainerHide(t->pages[t->len].child);
	t->len++;

	ZeroMemory(&item, sizeof (TCITEMW));
	item.mask = TCIF_TEXT;
	wname = toUTF16(name);
	item.pszText = wname;
	// MSDN's example code uses the first invalid index directly for this
	if (SendMessageW(hwnd, TCM_INSERTITEM, (WPARAM) n, (LPARAM) (&item)) == (LRESULT) -1)
		logLastError("error adding tab to Tab in uiTabAddPage()");
	uiFree(wname);
}
