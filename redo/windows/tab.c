// 16 may 2015
#include "uipriv_windows.h"

// TODO
// - can't seem to tab away anymore
// - removving the first page then pressing a BS_PUSHBUTTON button hangs

struct uiTab {
	uiWindowsControl c;
	HWND hwnd;
	struct ptrArray *pages;
};

static void onDestroy(uiTab *);

uiWindowsDefineControlWithOnDestroy(
	uiTab,								// type name
	uiTabType,							// type function
	onDestroy(this);						// on destroy
)

// utility functions

static LRESULT curpage(uiTab *t)
{
	return SendMessageW(t->hwnd, TCM_GETCURSEL, 0, 0);
}

static void showHidePage(uiTab *t, LRESULT which, int hide)
{
	struct child *page;

	if (which == (LRESULT) (-1))
		return;
	page = ptrArrayIndex(t->pages, struct child *, which);
	if (hide)
		ShowWindow(childTabPage(page), SW_HIDE);
	else {
		ShowWindow(childTabPage(page), SW_SHOW);
		// we only resize the current page, so we have to do this here
		childQueueRelayout(page);
	}
}

// control implementation

static BOOL onWM_NOTIFY(uiControl *c, HWND hwnd, NMHDR *nm, LRESULT *lResult)
{
	uiTab *t = uiTab(c);

	if (nm->code != TCN_SELCHANGING && nm->code != TCN_SELCHANGE)
		return FALSE;
	showHidePage(t, curpage(t), nm->code == TCN_SELCHANGING);
	*lResult = 0;
	if (nm->code == TCN_SELCHANGING)
		*lResult = FALSE;
	return TRUE;
}

static void onDestroy(uiTab *t)
{
	struct child *page;

	while (t->pages->len != 0) {
		page = ptrArrayIndex(t->pages, struct child *, 0);
		ptrArrayDelete(t->pages, 0);
		childDestroy(page);
	}
	ptrArrayDestroy(t->pages);
	uiWindowsUnregisterWM_NOTIFYHandler(t->hwnd);
}

static void minimumSize(uiWindowsControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
/* TODO
	uiTab *t = uiTab(c);
	intmax_t maxwid, maxht;
	intmax_t pagewid, pageht;
	uiControl *page;
	uintmax_t i;
	RECT r;

	maxwid = 0;
	maxht = 0;
	for (i = 0; i < t->pages->len; i++) {
		page = ptrArrayIndex(t->pages, struct child *, i);
		childMinimumSize(page, d, &pagewid, &pageht);
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
*/
}

static void tabRelayout(uiWindowsControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
	uiTab *t = uiTab(c);
	LRESULT n;
	struct child *page;
	RECT r;

	uiWindowsEnsureMoveWindow(t->hwnd, x, y, width, height);
	n = curpage(t);
	if (n == (LRESULT) (-1))
		return;
	page = ptrArrayIndex(t->pages, struct child *, n);

	// now we need to figure out what rect the child goes
	// this rect needs to be in parent window coordinates, but TCM_ADJUSTRECT wants a window rect, which is screen coordinates
printf("%d %d %d %d -> ", (int)x, (int)y, (int)width, (int)height);
	r.left = x;
	r.top = y;
	r.right = x + width;
	r.bottom = y + height;
	// TODO use the real parent; see below
	mapWindowRect(t->hwnd, NULL, &r);
	SendMessageW(t->hwnd, TCM_ADJUSTRECT, (WPARAM) FALSE, (LPARAM) (&r));
	mapWindowRect(NULL, t->hwnd, &r);
printf("%d %d %d %d\n", (int)r.left, (int)r.top, (int)r.right-r.left, (int)r.bottom-r.top);
	childRelayout(page, r.left, r.top, r.right - r.left, r.bottom - r.top);
}

static void tabContainerUpdateState(uiControl *c)
{
	uiTab *t = uiTab(c);
	struct child *page;
	uintmax_t i;

	for (i = 0; i < t->pages->len; i++) {
		page = ptrArrayIndex(t->pages, struct child *, i);
		childUpdateState(page);
	}
}

void uiTabAppend(uiTab *t, const char *name, uiControl *child)
{
	uiTabInsertAt(t, name, t->pages->len, child);
}

void uiTabInsertAt(uiTab *t, const char *name, uintmax_t n, uiControl *child)
{
	struct child *page;
	LRESULT hide, show;
	TCITEMW item;
	WCHAR *wname;

	// see below
	hide = curpage(t);

	// TODO make a child of the parent, not of the tab
	// this is the only real way to do proper tab stops
	// it's how tabs are /supposed to/ be done, anyway
	page = newChildWithTabPage(child, uiControl(t), t->hwnd);
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

void uiTabDelete(uiTab *t, uintmax_t n)
{
	struct child *page;

	// first delete the tab from the tab control
	// if this is the current tab, no tab will be selected, which is good
	if (SendMessageW(t->hwnd, TCM_DELETEITEM, (WPARAM) n, 0) == FALSE)
		logLastError("error deleting uiTab tab in tabDelete()");

	// now delete the page itself
	page = ptrArrayIndex(t->pages, struct child *, n);
	ptrArrayDelete(t->pages, n);
	childRemove(page);
}

uintmax_t uiTabNumPages(uiTab *t)
{
	return t->pages->len;
}

int uiTabMargined(uiTab *t, uintmax_t n)
{
	struct child *page;

	page = ptrArrayIndex(t->pages, struct child *, n);
	return childMargined(page);
}

void uiTabSetMargined(uiTab *t, uintmax_t n, int margined)
{
	struct child *page;

	page = ptrArrayIndex(t->pages, struct child *, n);
	childSetMargined(page, margined);
}

uiTab *uiNewTab(void)
{
	uiTab *t;

	t = (uiTab *) uiNewControl(uiTabType());

	t->hwnd = uiWindowsEnsureCreateControlHWND(0,			// don't set WS_EX_CONTROLPARENT yet; we do that dynamically in the message loop (see below)
		WC_TABCONTROLW, L"",
		// don't give WS_TABSTOP here; we only apply WS_TABSTOP if there are tabs
		TCS_TOOLTIPS,
		hInstance, NULL,
		TRUE);

	uiWindowsRegisterWM_NOTIFYHandler(t->hwnd, onWM_NOTIFY, uiControl(t));

	t->pages = newPtrArray();

	uiWindowsFinishNewControl(t, uiTab);
	uiControl(t)->ContainerUpdateState = tabContainerUpdateState;
	uiWindowsControl(t)->Relayout = tabRelayout;

	return t;
}
