// 12 april 2015
#include "uipriv_windows.h"

struct tab {
	uiTab t;
	HWND hwnd;
	uiContainer **pages;
	int *margined;
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
		n = SendMessageW(t->hwnd, TCM_GETCURSEL, 0, 0);
		if (n != (LRESULT) (-1))		// if we're changing to a real tab
			uiControlHide(uiControl(t->pages[n]));
		*lResult = FALSE;			// and allow the change
		return TRUE;
	case TCN_SELCHANGE:
		n = SendMessageW(t->hwnd, TCM_GETCURSEL, 0, 0);
		if (n != (LRESULT) (-1)) {		// if we're changing to a real tab
			uiControlShow(uiControl(t->pages[n]));
			// because we only resize the current child on resize, we'll need to trigger an update here
			// don't call uiParentUpdate(); doing that won't size the content area (so we'll still have a 0x0 content area, for instance)
			SendMessageW(t->hwnd, msgUpdateChild, 0, 0);
		}
		*lResult = 0;
		return TRUE;
	}
	return FALSE;
}

static void onDestroy(void *data)
{
	struct tab *t = (struct tab *) data;
	uintmax_t i;

	// first, hide the widget to avoid flicker
	ShowWindow(t->hwnd, SW_HIDE);
	// because the pages don't have by a libui paent, we can simply destroy them
	// we don't have to worry about the Windows tab control holding a reference to our bin; there is no reference holding anyway
	for (i = 0; i < t->len; i++)
		uiControlDestroy(uiControl(t->pages[i]));
	// and finally destroy ourselves
	uiFree(t->pages);
	uiFree(t->margined);
	uiFree(t);
}

static void preferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct tab *t = (struct tab *) c;
	LRESULT current;
	uiContainer *curpage;
	intmax_t curwid, curht;
	RECT r;

	r.left = 0;
	r.top = 0;
	r.right = 0;
	r.bottom = 0;
	if (t->len != 0) {
		current = SendMessageW(t->hwnd, TCM_GETCURSEL, 0, 0);
		if (current != (LRESULT) (-1)) {
			curpage = t->pages[current];
			uiControlPreferredSize(uiControl(curpage), d, &curwid, &curht);
			r.right = curwid;
			r.bottom = curht;
		}
	}
	// otherwise just use the rect [0 0 0 0]
	// the following will take the tabs themselves into account
	SendMessageW(t->hwnd, TCM_ADJUSTRECT, (WPARAM) TRUE, (LPARAM) (&r));
	*width = r.right - r.left;
	*height = r.bottom - r.top;
}

// common code for resizes
static void resizeTab(struct tab *t, LONG width, LONG height)
{
	LRESULT n;
	RECT r;
	HWND binHWND;

	n = SendMessageW(t->hwnd, TCM_GETCURSEL, 0, 0);
	if (n == (LRESULT) (-1))		// no child selected; do nothing
		return;

	// make a rect at (0, 0) of the given window size
	// this should give us the correct client coordinates
	r.left = 0;
	r.top = 0;
	r.right = width;
	r.bottom = height;
	// convert to the display rectangle
	SendMessageW(t->hwnd, TCM_ADJUSTRECT, FALSE, (LPARAM) (&r));

	binHWND = (HWND) uiControlHandle(uiControl(t->pages[n]));
	if (MoveWindow(binHWND, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE) == 0)
		logLastError("error resizing uiTab page in resizeTab()");
}

// and finally, because we have to resize parents, we have to handle resizes and updates
// TODO see if this approach is /really/ necessary and we can get by with an alteration to the above function and overriding uiControlResize()
static LRESULT CALLBACK tabSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	struct tab *t = (struct tab *) dwRefData;
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
		resizeTab(t, wp->cx, wp->cy);
		return lResult;
	case msgUpdateChild:
		if (GetWindowRect(t->hwnd, &r) == 0)
			logLastError("error getting Tab window rect for synthesized resize message in tabSubProc()");
		// these are in screen coordinates, which match what WM_WINDOWPOSCHANGED gave us (see http://stackoverflow.com/questions/29598334/are-the-coordinates-in-windowpos-on-wm-windowposchanged-in-parent-coordinates-or)
		resizeTab(t, r.right - r.left, r.bottom - r.top);
		return 0;
	case WM_NCDESTROY:
		if ((*fv_RemoveWindowSubclass)(hwnd, tabSubProc, uIdSubclass) == FALSE)
			logLastError("error removing Tab resize handling subclass in tabSubProc()");
		break;
	}
	return (*fv_DefSubclassProc)(hwnd, uMsg, wParam, lParam);
}

#define tabCapGrow 32

static void tabAppendPage(uiTab *tt, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	TCITEMW item;
	LRESULT n;
	uiContainer *page;
	WCHAR *wname;

	if (t->len >= t->cap) {
		t->cap += tabCapGrow;
		t->pages = (uiContainer **) uiRealloc(t->pages, t->cap * sizeof (uiContainer *));
		t->margined = (int *) uiRealloc(t->margined, t->cap * sizeof (int));
	}

	n = SendMessageW(t->hwnd, TCM_GETITEMCOUNT, 0, 0);

	page = newBin();
	binSetMainControl(page, child);
	binSetParent(page, (uintptr_t) (t->hwnd));
	if (n != 0)		// if this isn't the first page, we have to hide the other controls
		uiControlHide(uiControl(page));
	t->pages[t->len] = page;
	t->margined[t->len] = 0;		// TODO should not be necessary but blah blah blah realloc
	t->len++;

	ZeroMemory(&item, sizeof (TCITEMW));
	item.mask = TCIF_TEXT;
	wname = toUTF16(name);
	item.pszText = wname;
	// MSDN's example code uses the first invalid index directly for this
	if (SendMessageW(t->hwnd, TCM_INSERTITEM, (WPARAM) n, (LPARAM) (&item)) == (LRESULT) -1)
		logLastError("error adding tab to Tab in uiTabAddPage()");
	uiFree(wname);

	// if this is the first tab, Windows will automatically show it /without/ sending a TCN_SELCHANGE notification
	// so we need to manually resize the tab ourselves
	// don't use uiUpdateParent() for the same reason as in the TCN_SELCHANGE handler
	SendMessageW(t->hwnd, msgUpdateChild, 0, 0);
}

static void tabDeletePage(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;
	uiContainer *page;
	uintmax_t i;

	// first delete the tab from the tab control
	// if this is the current tab, no tab will be selected, which is good
	if (SendMessageW(t->hwnd, TCM_DELETEITEM, (WPARAM) n, 0) == FALSE)
		logLastError("error deleting Tab page in tabDeletePage()");

	// now delete the page itself
	page = t->pages[n];
	for (i = n; i < t->len - 1; i++) {
		t->pages[i] = t->pages[i + 1];
		t->margined[i] = t->margined[i + 1];
	}
	t->pages[i] = NULL;
	t->margined[i] = 0;
	t->len--;

	// make sure the page's control isn't destroyed
	binSetMainControl(page, NULL);

	// see tabDestroy() above for details
	uiControlDestroy(uiControl(page));
}

static uintmax_t tabNumPages(uiTab *tt)
{
	struct tab *t = (struct tab *) tt;

	return t->len;
}

static int tabMargined(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;

	return t->margined[n];
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
#define tabMargin 7

static void tabSetMargined(uiTab *tt, uintmax_t n, int margined)
{
	struct tab *t = (struct tab *) tt;

	t->margined[n] = margined;
	if (t->margined[n])
		binSetMargins(t->pages[n], tabMargin, tabMargin, tabMargin, tabMargin);
	else
		binSetMargins(t->pages[n], 0, 0, 0, 0);
}

uiTab *uiNewTab(void)
{
	struct tab *t;
	uiWindowsMakeControlParams p;

	t = uiNew(struct tab);

	p.dwExStyle = 0;		// don't set WS_EX_CONTROLPARENT yet; we do that dynamically in the message loop (see main_windows.c)
	p.lpClassName = WC_TABCONTROLW;
	p.lpWindowName = L"";
	p.dwStyle = TCS_TOOLTIPS | WS_TABSTOP;
	p.hInstance = hInstance;
	p.useStandardControlFont = TRUE;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onDestroy = onDestroy;
	p.onDestroyData = t;
	uiWindowsMakeControl(uiControl(t), &p);

	t->hwnd = (HWND) uiControlHandle(uiControl(t));

	if ((*fv_SetWindowSubclass)(t->hwnd, tabSubProc, 0, (DWORD_PTR) t) == FALSE)
		logLastError("error subclassing Tab to give it its own resize handler in uiNewTab()");

	uiControl(t)->PreferredSize = preferredSize;

	uiTab(t)->AppendPage = tabAppendPage;
	uiTab(t)->DeletePage = tabDeletePage;
	uiTab(t)->NumPages = tabNumPages;
	uiTab(t)->Margined = tabMargined;
	uiTab(t)->SetMargined = tabSetMargined;

	return uiTab(t);
}
