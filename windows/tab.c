// 12 april 2015
#include "uipriv_windows.h"

struct tab {
	uiTab t;
	HWND hwnd;
	struct ptrArray *pages;
	void (*baseEnable)(uiControl *);
	void (*baseDisable)(uiControl *);
	void (*baseSysFunc)(uiControl *, uiControlSysFuncParams *);
};

struct tabPage {
	uiContainer *bin;
	int margined;
};

static BOOL onWM_COMMAND(uiControl *c, WORD code, LRESULT *lResult)
{
	return FALSE;
}

// we have to handle hiding and showing of tab pages ourselves
static BOOL onWM_NOTIFY(uiControl *c, NMHDR *nm, LRESULT *lResult)
{
	struct tab *t = (struct tab *) c;
	struct tabPage *page;
	LRESULT n;

	if (nm->code != TCN_SELCHANGING && nm->code != TCN_SELCHANGE)
		return FALSE;
	n = SendMessageW(t->hwnd, TCM_GETCURSEL, 0, 0);
	if (n == (LRESULT) (-1))		// not changing from/to a page; nothing to do
		return FALSE;
	page = ptrArrayIndex(t->pages, struct tabPage *, n);
	if (nm->code == TCN_SELCHANGING) {
		// changing from a real page
		uiControlHide(uiControl(page->bin));
		*lResult = FALSE;			// and allow the change
		return TRUE;
	}
	// otherwise it's TCN_SELCHANGE
	// and we're changing to a real page
	uiControlShow(uiControl(page->bin));
	// because we only resize the current child on resize, we'll need to trigger an update here
	// don't call uiParentUpdate(); doing that won't size the content area (so we'll still have a 0x0 content area, for instance)
	SendMessageW(t->hwnd, msgUpdateChild, 0, 0);
	*lResult = 0;
	return TRUE;
}

static void onDestroy(void *data)
{
	struct tab *t = (struct tab *) data;
	struct tabPage *p;

	// first, hide the widget to avoid flicker
	ShowWindow(t->hwnd, SW_HIDE);
	// because the pages don't have by a libui paent, we can simply destroy them
	// we don't have to worry about the Windows tab control holding a reference to our bin; there is no reference holding anyway
	while (t->pages->len != 0) {
		p = ptrArrayIndex(t->pages, struct tabPage *, 0);
		// we do have to remove the page from the tab control, though
		binSetParent(p->bin, 0);
		uiControlDestroy(uiControl(p->bin));
		ptrArrayDelete(t->pages, 0);
		uiFree(p);
	}
	// and finally destroy ourselves
	ptrArrayDestroy(t->pages);
	uiFree(t);
}

static void tabPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct tab *t = (struct tab *) c;
	LRESULT current;
	struct tabPage *curpage;
	intmax_t curwid, curht;
	RECT r;

	r.left = 0;
	r.top = 0;
	r.right = 0;
	r.bottom = 0;
	if (t->pages->len != 0) {
		current = SendMessageW(t->hwnd, TCM_GETCURSEL, 0, 0);
		if (current != (LRESULT) (-1)) {
			curpage = ptrArrayIndex(t->pages, struct tabPage *, current);
			uiControlPreferredSize(uiControl(curpage->bin), d, &curwid, &curht);
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

static void tabEnable(uiControl *c)
{
	struct tab *t = (struct tab *) c;
	struct tabPage *p;
	uintmax_t i;

	(*(t->baseEnable))(uiControl(t));
	for (i = 0; i < t->pages->len; i++) {
		p = ptrArrayIndex(t->pages, struct tabPage *, i);
		uiControlEnable(uiControl(p->bin));
	}
}

static void tabDisable(uiControl *c)
{
	struct tab *t = (struct tab *) c;
	struct tabPage *p;
	uintmax_t i;

	(*(t->baseDisable))(uiControl(t));
	for (i = 0; i < t->pages->len; i++) {
		p = ptrArrayIndex(t->pages, struct tabPage *, i);
		uiControlDisable(uiControl(p->bin));
	}
}

static void tabSysFunc(uiControl *c, uiControlSysFuncParams *p)
{
	struct tab *t = (struct tab *) c;
	struct tabPage *page;
	uintmax_t i;

	(*(t->baseSysFunc))(uiControl(t), p);
	for (i = 0; i < t->pages->len; i++) {
		page = ptrArrayIndex(t->pages, struct tabPage *, i);
		uiControlSysFunc(uiControl(page->bin), p);
	}
}

// common code for resizes
static void resizeTab(struct tab *t, LONG width, LONG height)
{
	LRESULT n;
	RECT r;
	struct tabPage *p;
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

	p = ptrArrayIndex(t->pages, struct tabPage *, n);
	binHWND = (HWND) uiControlHandle(uiControl(p->bin));
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
	struct tabPage *page;
	WCHAR *wname;

	page = uiNew(struct tabPage);
	n = SendMessageW(t->hwnd, TCM_GETITEMCOUNT, 0, 0);

	page->bin = newBin();
	binSetMainControl(page->bin, child);
	binSetParent(page->bin, (uintptr_t) (t->hwnd));
	if (n != 0)		// if this isn't the first page, we have to hide the other controls
		uiControlHide(uiControl(page->bin));

	ptrArrayAppend(t->pages, page);

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

static void tabInsertPageBefore(uiTab *tt, const char *name, uintmax_t n, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	TCITEMW item;
	struct tabPage *page;
	WCHAR *wname;

	page = uiNew(struct tabPage);

	page->bin = newBin();
	binSetMainControl(page->bin, child);
	binSetParent(page->bin, (uintptr_t) (t->hwnd));
	// always hide; the current tab doesn't change
	uiControlHide(uiControl(page->bin));

	ptrArrayInsertBefore(t->pages, n, page);

	ZeroMemory(&item, sizeof (TCITEMW));
	item.mask = TCIF_TEXT;
	wname = toUTF16(name);
	item.pszText = wname;
	if (SendMessageW(t->hwnd, TCM_INSERTITEM, (WPARAM) n, (LPARAM) (&item)) == (LRESULT) -1)
		logLastError("error adding tab to Tab in uiTabInsertPageBefore()");
	uiFree(wname);
}

static void tabDeletePage(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;
	struct tabPage *page;

	// first delete the tab from the tab control
	// if this is the current tab, no tab will be selected, which is good
	if (SendMessageW(t->hwnd, TCM_DELETEITEM, (WPARAM) n, 0) == FALSE)
		logLastError("error deleting Tab page in tabDeletePage()");

	// now delete the page itself
	page = ptrArrayIndex(t->pages, struct tabPage *, n);
	ptrArrayDelete(t->pages, n);

	// make sure the page's control isn't destroyed
	binSetMainControl(page->bin, NULL);

	// see tabDestroy() above for details
	binSetParent(page->bin, 0);
	uiControlDestroy(uiControl(page->bin));
	uiFree(page);
}

static uintmax_t tabNumPages(uiTab *tt)
{
	struct tab *t = (struct tab *) tt;

	return t->pages->len;
}

static int tabMargined(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;
	struct tabPage *page;

	page = ptrArrayIndex(t->pages, struct tabPage *, n);
	return page->margined;
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
#define tabMargin 7

static void tabSetMargined(uiTab *tt, uintmax_t n, int margined)
{
	struct tab *t = (struct tab *) tt;
	struct tabPage *page;

	page = ptrArrayIndex(t->pages, struct tabPage *, n);
	page->margined = margined;
	if (page->margined)
		binSetMargins(page->bin, tabMargin, tabMargin, tabMargin, tabMargin);
	else
		binSetMargins(page->bin, 0, 0, 0, 0);
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
	t->pages = newPtrArray();

	if ((*fv_SetWindowSubclass)(t->hwnd, tabSubProc, 0, (DWORD_PTR) t) == FALSE)
		logLastError("error subclassing Tab to give it its own resize handler in uiNewTab()");

	uiControl(t)->PreferredSize = tabPreferredSize;
	t->baseEnable = uiControl(t)->Enable;
	uiControl(t)->Enable = tabEnable;
	t->baseDisable = uiControl(t)->Disable;
	uiControl(t)->Disable = tabDisable;
	t->baseSysFunc = uiControl(t)->SysFunc;
	uiControl(t)->SysFunc = tabSysFunc;

	uiTab(t)->AppendPage = tabAppendPage;
	uiTab(t)->InsertPageBefore = tabInsertPageBefore;
	uiTab(t)->DeletePage = tabDeletePage;
	uiTab(t)->NumPages = tabNumPages;
	uiTab(t)->Margined = tabMargined;
	uiTab(t)->SetMargined = tabSetMargined;

	return uiTab(t);
}
