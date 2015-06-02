// 16 may 2015
#include "uipriv_windows.h"

// TODO
// - dialog doesn't respond to WM_PRINTCLIENT in Windows Classic

struct tab {
	uiTab t;
	HWND hwnd;
	struct ptrArray *pages;
	void (*baseResize)(uiControl *, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
	void (*baseCommitDestroy)(uiControl *);
};

uiDefineControlType(uiTab, uiTypeTab, struct tab)

// utility functions

static LRESULT curpage(struct tab *t)
{
	return SendMessageW(t->hwnd, TCM_GETCURSEL, 0, 0);
}

static void showHidePage(struct tab *t, LRESULT which, int hide)
{
	uiControl *page;

	if (which == (LRESULT) (-1))
		return;
	page = ptrArrayIndex(t->pages, uiControl *, which);
	if (hide)
		uiControlHide(page);
	else {
		uiControlShow(page);
		// we only resize the current page, so we have to do this here
		uiControlQueueResize(page);
	}
}

// see below for info
static void updateWS_TABSTOP(HWND hwnd)
{
	LRESULT n;
	DWORD le;

	SetLastError(0);
	n = SendMessageW(hwnd, TCM_GETITEMCOUNT, 0, 0);
	le = GetLastError();
	SetLastError(le);		// just to be safe
	if (n != 0)
		setStyle(hwnd, getStyle(hwnd) | WS_TABSTOP);
	else if (le == 0)			// truly no tabs
		setStyle(hwnd, getStyle(hwnd) & ~WS_TABSTOP);
	else
		logLastError("error getting number of tabs in updateWS_TABSTOP()");
}

// control implementation

static BOOL onWM_NOTIFY(uiControl *c, HWND hwnd, NMHDR *nm, LRESULT *lResult)
{
	struct tab *t = (struct tab *) c;

	if (nm->code != TCN_SELCHANGING && nm->code != TCN_SELCHANGE)
		return FALSE;
	showHidePage(t, curpage(t), nm->code == TCN_SELCHANGING);
	*lResult = 0;
	if (nm->code == TCN_SELCHANGING)
		*lResult = FALSE;
	return TRUE;
}

static void tabCommitDestroy(uiControl *c)
{
	struct tab *t = (struct tab *) c;
	uiControl *page;

	while (t->pages->len != 0) {
		page = ptrArrayIndex(t->pages, uiControl *, 0);
		ptrArrayDelete(t->pages, 0);
		tabPageDestroyChild(page);
		uiControlSetParent(page, NULL);
		uiControlDestroy(page);
	}
	ptrArrayDestroy(t->pages);
	uiWindowsUnregisterWM_NOTIFYHandler(t->hwnd);
	(*(t->baseCommitDestroy))(uiControl(t));
}

static uintptr_t tabHandle(uiControl *c)
{
	struct tab *t = (struct tab *) c;

	return (uintptr_t) (t->hwnd);
}

static void tabPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct tab *t = (struct tab *) c;
	intmax_t maxwid, maxht;
	intmax_t pagewid, pageht;
	uiControl *page;
	uintmax_t i;
	RECT r;

	maxwid = 0;
	maxht = 0;
	for (i = 0; i < t->pages->len; i++) {
		page = ptrArrayIndex(t->pages, uiControl *, i);
		uiControlPreferredSize(page, d, &pagewid, &pageht);
		if (maxwid < pagewid)
			maxwid = pagewid;
		if (maxht < pageht)
			maxht = pageht;
	}

	r.left = 0;
	r.top = 0;
	r.right = maxwid;
	r.bottom = maxht;
	// this also includes the tabs themselves
	SendMessageW(t->hwnd, TCM_ADJUSTRECT, (WPARAM) TRUE, (LPARAM) (&r));
	*width = r.right - r.left;
	*height = r.bottom - r.top;
}

static void tabResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	struct tab *t = (struct tab *) c;
	LRESULT n;
	uiControl *page;
	RECT r;
	uiSizing *dchild;

	(*(t->baseResize))(uiControl(t), x, y, width, height, d);
	n = curpage(t);
	if (n == (LRESULT) (-1))
		return;
	page = ptrArrayIndex(t->pages, uiControl *, n);

	dchild = uiControlSizing(uiControl(t));

	// now we need to figure out what rect the child goes
	// this rect needs to be in toplevel window coordinates, but TCM_ADJUSTRECT wants a window rect, which is screen coordinates
	r.left = x;
	r.top = y;
	r.right = x + width;
	r.bottom = y + height;
	mapWindowRect(dchild->Sys->CoordFrom, NULL, &r);
	SendMessageW(t->hwnd, TCM_ADJUSTRECT, (WPARAM) FALSE, (LPARAM) (&r));
	mapWindowRect(NULL, dchild->Sys->CoordFrom, &r);

	uiControlResize(page, r.left, r.top, r.right - r.left, r.bottom - r.top, dchild);

	uiFreeSizing(dchild);
}

static void tabContainerUpdateState(uiControl *c)
{
	struct tab *t = (struct tab *) c;
	uiControl *page;
	uintmax_t i;

	for (i = 0; i < t->pages->len; i++) {
		page = ptrArrayIndex(t->pages, uiControl *, i);
		uiControlUpdateState(page);
	}
}

static void tabAppend(uiTab *tt, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) tt;

	uiTabInsertAt(tt, name, t->pages->len, child);
}

static void tabInsertAt(uiTab *tt, const char *name, uintmax_t n, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	uiControl *page;
	LRESULT hide, show;
	TCITEMW item;
	WCHAR *wname;

	// see below
	hide = curpage(t);

	page = newTabPage();
	tabPageSetChild(page, child);
	uiControlSetParent(page, uiControl(t));
	// and make it invisible at first; we show it later if needed
	uiControlHide(page);
	ptrArrayInsertAt(t->pages, n, page);

	ZeroMemory(&item, sizeof (TCITEMW));
	item.mask = TCIF_TEXT;
	wname = toUTF16(name);
	item.pszText = wname;
	if (SendMessageW(t->hwnd, TCM_INSERTITEM, (WPARAM) n, (LPARAM) (&item)) == (LRESULT) -1)
		logLastError("error adding tab to uiTab in uiTabInsertAt()");
	uiFree(wname);

	// we need to do this because adding the first tab doesn't send a TCN_SELCHANGE; it just shows the page
	show = curpage(t);
	if (show != hide) {
		showHidePage(t, hide, 1);
		showHidePage(t, show, 0);
	}

	updateWS_TABSTOP(t->hwnd);
}

static void tabDelete(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;
	uiControl *page;

	// first delete the tab from the tab control
	// if this is the current tab, no tab will be selected, which is good
	if (SendMessageW(t->hwnd, TCM_DELETEITEM, (WPARAM) n, 0) == FALSE)
		logLastError("error deleting uiTab tab in tabDelete()");

	// now delete the page itself
	page = ptrArrayIndex(t->pages, uiControl *, n);
	ptrArrayDelete(t->pages, n);

	// and free the page
	tabPagePreserveChild(page);
	uiControlSetParent(page, NULL);
	uiControlDestroy(page);

	updateWS_TABSTOP(t->hwnd);
}

static uintmax_t tabNumPages(uiTab *tt)
{
	struct tab *t = (struct tab *) tt;

	return t->pages->len;
}

static int tabMargined(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;
	uiControl *page;

	page = ptrArrayIndex(t->pages, uiControl *, n);
	return tabPageMargined(page);
}

static void tabSetMargined(uiTab *tt, uintmax_t n, int margined)
{
	struct tab *t = (struct tab *) tt;
	uiControl *page;

	page = ptrArrayIndex(t->pages, uiControl *, n);
	tabPageSetMargined(page, margined);
	uiControlQueueResize(page);
}

// this handles tab navigation; see main.c for details
static LRESULT CALLBACK tabSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	struct tab *t = (struct tab *) dwRefData;
	LRESULT n;
	uiControl *page;

	switch (uMsg) {
	case msgHasTabStops:
		n = SendMessageW(t->hwnd, TCM_GETCURSEL, 0, 0);
		if (n == (LRESULT) (-1))		// no current selection == no tab stops
			return FALSE;
		page = ptrArrayIndex(t->pages, uiControl *, n);
		if (uiControlHasTabStops(page))
			return TRUE;
		return FALSE;
	case WM_NCDESTROY:
		if (RemoveWindowSubclass(hwnd, tabSubProc, uIdSubclass) == FALSE)
			logLastError("error removing Tab resize handling subclass in tabSubProc()");
		break;
	}
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

uiTab *uiNewTab(void)
{
	struct tab *t;

	t = (struct tab *) uiWindowsNewSingleHWNDControl(uiTypeTab());

	t->hwnd = uiWindowsUtilCreateControlHWND(0,			// don't set WS_EX_CONTROLPARENT yet; we do that dynamically in the message loop (see main_windows.c)
		WC_TABCONTROLW, L"",
		// don't give WS_TABSTOP here; we only apply WS_TABSTOP if there are tabs
		TCS_TOOLTIPS,
		hInstance, NULL,
		TRUE);

	uiWindowsRegisterWM_NOTIFYHandler(t->hwnd, onWM_NOTIFY, uiControl(t));

	if (SetWindowSubclass(t->hwnd, tabSubProc, 0, (DWORD_PTR) t) == FALSE)
		logLastError("error subclassing Tab to assist in tab stop handling in uiNewTab()");

	t->pages = newPtrArray();

	uiControl(t)->Handle = tabHandle;
	uiControl(t)->PreferredSize = tabPreferredSize;
	t->baseResize = uiControl(t)->Resize;
	uiControl(t)->Resize = tabResize;
	t->baseCommitDestroy = uiControl(t)->CommitDestroy;
	uiControl(t)->CommitDestroy = tabCommitDestroy;
	uiControl(t)->ContainerUpdateState = tabContainerUpdateState;

	uiTab(t)->Append = tabAppend;
	uiTab(t)->InsertAt = tabInsertAt;
	uiTab(t)->Delete = tabDelete;
	uiTab(t)->NumPages = tabNumPages;
	uiTab(t)->Margined = tabMargined;
	uiTab(t)->SetMargined = tabSetMargined;

	return uiTab(t);
}

// unfortunately WS_TABSTOP and WS_EX_CONTROLPARENT are mutually exclusive, so we have to toggle between them
// see main.c for more details

void tabEnterTabNavigation(HWND hwnd)
{
	setStyle(hwnd, getStyle(hwnd) & ~WS_TABSTOP);
	setExStyle(hwnd, getExStyle(hwnd) | WS_EX_CONTROLPARENT);
}

void tabLeaveTabNavigation(HWND hwnd)
{
	setExStyle(hwnd, getExStyle(hwnd) & ~WS_EX_CONTROLPARENT);
	updateWS_TABSTOP(hwnd);
}
