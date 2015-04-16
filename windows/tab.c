// 12 april 2015
#include "uipriv_windows.h"

// TODO
// - tab change notifications aren't being sent on wine (anymore...? TODO)
// - tell wine developers that tab controls do respond to parent changes on real windows (at least comctl6 tab controls do)

struct tab {
	uiTab t;
	uiParent **pages;
	uintmax_t len;
	uintmax_t cap;
};

static BOOL onWM_COMMAND(uiControl *c, WORD code, LRESULT *lResult)
{
	return FALSE;
}

// we have to handle hiding and showing of tab pages ourselves
static BOOL onWM_NOTIFY(uiControl *c, NMHDR *nm, LRESULT *lResult)
{
	struct tab *t = (struct tab *) c;
	LRESULT n;

	switch (nm->code) {
	case TCN_SELCHANGING:
		n = SendMessageW(uiControlHWND(c), TCM_GETCURSEL, 0, 0);
		if (n != (LRESULT) (-1))		// if we're changing to a real tab
			ShowWindow(uiParentHWND(t->pages[n]), SW_HIDE);
		*lResult = FALSE;			// and allow the change
		return TRUE;
	case TCN_SELCHANGE:
		n = SendMessageW(uiControlHWND(c), TCM_GETCURSEL, 0, 0);
		if (n != (LRESULT) (-1)) {		// if we're changing to a real tab
			ShowWindow(uiParentHWND(t->pages[n]), SW_SHOW);
			// because we only resize the current child on resize, we'll need to trigger an update here
			// don't call uiParentUpdate(); doing that won't size the content area (so we'll still have a 0x0 content area, for instance)
			SendMessageW(uiControlHWND(c), msgUpdateChild, 0, 0);
		}
		*lResult = 0;
		return TRUE;
	}
	return FALSE;
}

static void onWM_DESTROY(uiControl *c)
{
	struct tab *t = (struct tab *) c;

	// no need to worry about freeing the pages themselves; they'll destroy themselves after we return
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
	struct tab *t = (struct tab *) c;
	HWND hwnd;
	LRESULT n;
	RECT r;

	hwnd = uiControlHWND(c);

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

	if (MoveWindow(uiParentHWND(t->pages[n]), r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE) == 0)
		logLastError("error resizing current tab page in resizeTab()");
}

// and finally, because we have to resize parents, we have to handle resizes and updates
static LRESULT CALLBACK tabSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	uiControl *c = (uiControl *) dwRefData;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	LRESULT lResult;
	RECT r;

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
		if (GetWindowRect(uiControlHWND(c), &r) == 0)
			logLastError("error getting Tab window rect for synthesized resize message in tabSubProc()");
		// these are in screen coordinates, which match what WM_WINDOWPOSCHANGED gave us (thanks TODOTODOTODOTODOTODOTODOTODO)
		resizeTab(c, r.right - r.left, r.bottom - r.top);
		return 0;
	case WM_NCDESTROY:
		if ((*fv_RemoveWindowSubclass)(hwnd, tabSubProc, uIdSubclass) == FALSE)
			logLastError("error removing Tab resize handling subclass in tabSubProc()");
		break;
	}
	return (*fv_DefSubclassProc)(hwnd, uMsg, wParam, lParam);
}

#define tabCapGrow 32

void addPage(uiTab *tt, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	HWND hwnd;
	TCITEMW item;
	LRESULT n;
	uiParent *parent;
	WCHAR *wname;

	if (t->len >= t->cap) {
		t->cap += tabCapGrow;
		t->pages = (uiParent **) uiRealloc(t->pages, t->cap * sizeof (uiParent *), "uiParent *[]");
	}

	hwnd = uiControlHWND(uiControl(t));
	n = SendMessageW(hwnd, TCM_GETITEMCOUNT, 0, 0);

	parent = uiNewParent((uintptr_t) hwnd);
	uiParentSetChild(parent, child);
	uiParentUpdate(parent);
	if (n != 0)		// if this isn't the first page, we have to hide the other controls
		ShowWindow(uiParentHWND(parent), SW_HIDE);
	t->pages[t->len] = parent;
	t->len++;

	ZeroMemory(&item, sizeof (TCITEMW));
	item.mask = TCIF_TEXT;
	wname = toUTF16(name);
	item.pszText = wname;
	// MSDN's example code uses the first invalid index directly for this
	if (SendMessageW(hwnd, TCM_INSERTITEM, (WPARAM) n, (LPARAM) (&item)) == (LRESULT) -1)
		logLastError("error adding tab to Tab in uiTabAddPage()");
	uiFree(wname);

	// if this is the first tab, Windows will automatically show it /without/ sending a TCN_SELCHANGE notification
	// (TODO verify that)
	// so we need to manually resize the tab ourselves
	// don't use uiUpdateParent() for the same reason as in the TCN_SELCHANGE handler
	SendMessageW(uiControlHWND(c), msgUpdateChild, 0, 0);
}

uiTab *uiNewTab(void)
{
	struct tab *t;
	uiWindowsNewControlParams p;
	HWND hwnd;

	t = uiNew(struct tab);

	p.dwExStyle = 0;		// don't set WS_EX_CONTROLPARENT yet; we do that dynamically in the message loop (see main_windows.c)
	p.lpClassName = WC_TABCONTROLW;
	p.lpWindowName = L"";
	p.dwStyle = TCS_TOOLTIPS | WS_TABSTOP;
	p.hInstance = hInstance;
	p.useStandardControlFont = TRUE;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onWM_DESTROY = onWM_DESTROY;
	uiWindowsNewControl(uiControl(t), &p);

	hwnd = uiControlHWND(uiControl(t));
	if ((*fv_SetWindowSubclass)(hwnd, tabSubProc, 0, (DWORD_PTR) t) == FALSE)
		logLastError("error subclassing Tab to give it its own resize handler in uiNewTab()");

	uiControl(t)->PreferredSize = preferredSize;

	uiTab(t)->AddPage = addPage;

	return uiTab(t);
}
