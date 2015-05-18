// 16 may 2015
#include "uipriv_windows.h"

// TODO
// - comctl5 on real windows: tabs get drawn behind checkbox

struct tab {
	uiTab t;
	HWND hwnd;
	struct ptrArray *pages;
	void (*baseResize)(uiControl *, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
};

struct tabPage {
	uiControl *control;
	int margined;
};

// utility functions

static LRESULT curpage(struct tab *t)
{
	return SendMessageW(t->hwnd, TCM_GETCURSEL, 0, 0);
}

static void showHidePage(struct tab *t, LRESULT which, int hide)
{
	struct tabPage *page;

	if (which == (LRESULT) (-1))
		return;
	page = ptrArrayIndex(t->pages, struct tabPage *, which);
	if (hide)
		uiControlContainerHide(page->control);
	else {
		uiControlContainerShow(page->control);
		// we only resize the current page, so we have to do this here
		uiControlQueueResize(page->control);
	}
}

// control implementation

static BOOL onWM_COMMAND(uiControl *c, WORD code, LRESULT *lResult)
{
	return FALSE;
}

static BOOL onWM_NOTIFY(uiControl *c, NMHDR *nm, LRESULT *lResult)
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

static void onDestroy(void *data)
{
	// TODO
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
#define tabMargin 7

static void tabPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	// TODO
}

static void tabResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	struct tab *t = (struct tab *) c;
	LRESULT n;
	struct tabPage *page;
	RECT r;
	uiSizing *dchild;

	(*(t->baseResize))(uiControl(t), x, y, width, height, d);
	n = curpage(t);
	if (n == (LRESULT) (-1))
		return;
	page = ptrArrayIndex(t->pages, struct tabPage *, n);

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

	if (page->margined) {
		r.left += uiWindowsDlgUnitsToX(tabMargin, d->Sys->BaseX);
		r.top += uiWindowsDlgUnitsToY(tabMargin, d->Sys->BaseY);
		r.right -= uiWindowsDlgUnitsToX(tabMargin, d->Sys->BaseX);
		r.bottom -= uiWindowsDlgUnitsToY(tabMargin, d->Sys->BaseY);
	}
	uiControlResize(page->control, r.left, r.top, r.right - r.left, r.bottom - r.top, dchild);

	uiFreeSizing(dchild);
}

static void tabAppend(uiTab *tt, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) tt;

	uiTabInsertAt(tt, name, t->pages->len, child);
}

static void tabInsertAt(uiTab *tt, const char *name, uintmax_t n, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	struct tabPage *page;
	LRESULT hide, show;
	TCITEMW item;
	WCHAR *wname;

	// see below
	hide = curpage(t);

	page = uiNew(struct tabPage);
	page->control = child;
	uiControlSetParent(page->control, uiControl(t));
	// and make it invisible at first; we show it later if needed
	uiControlContainerHide(page->control);
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
}

static void tabDelete(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;
	struct tabPage *page;

	// first delete the tab from the tab control
	// if this is the current tab, no tab will be selected, which is good
	if (SendMessageW(t->hwnd, TCM_DELETEITEM, (WPARAM) n, 0) == FALSE)
		logLastError("error deleting uiTab tab in tabDelete()");

	// now delete the page itself
	page = ptrArrayIndex(t->pages, struct tabPage *, n);
	ptrArrayDelete(t->pages, n);

	// and keep the page control alive
	uiControlSetParent(page->control, NULL);
	// and show it again, as we don't know where it will go next
	uiControlContainerShow(page->control);

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

static void tabSetMargined(uiTab *tt, uintmax_t n, int margined)
{
	struct tab *t = (struct tab *) tt;
	struct tabPage *page;

	page = ptrArrayIndex(t->pages, struct tabPage *, n);
	page->margined = margined;
	uiControlQueueResize(page->control);
}

uiTab *uiNewTab(void)
{
	struct tab *t;
	uiWindowsMakeControlParams p;

	t = uiNew(struct tab);
	uiTyped(t)->Type = uiTypeTab();

	p.dwExStyle = 0;		// don't set WS_EX_CONTROLPARENT yet; we do that dynamically in the message loop (see main_windows.c)
	p.lpClassName = WC_TABCONTROLW;
	p.lpWindowName = L"";
	p.dwStyle = TCS_TOOLTIPS | WS_TABSTOP;		// start with this; we will alternate between this and WS_EX_CONTROLPARENT as needed (see main.c and msgHasTabStops above and the toggling functions below)
	p.hInstance = hInstance;
	p.lpParam = NULL;
	p.useStandardControlFont = TRUE;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onDestroy = onDestroy;
	p.onDestroyData = t;
	uiWindowsMakeControl(uiControl(t), &p);

	t->hwnd = (HWND) uiControlHandle(uiControl(t));

	t->pages = newPtrArray();

	uiControl(t)->PreferredSize = tabPreferredSize;
	t->baseResize = uiControl(t)->Resize;
	uiControl(t)->Resize = tabResize;

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
	setStyle(hwnd, getStyle(hwnd) | WS_TABSTOP);
}
