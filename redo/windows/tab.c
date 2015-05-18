// 16 may 2015
#include "uipriv_windows.h"

struct tab {
	uiTab t;
	HWND hwnd;
};

static BOOL onWM_COMMAND(uiControl *c, WORD code, LRESULT *lResult)
{
	return FALSE;
}

static BOOL onWM_NOTIFY(uiControl *c, NMHDR *nm, LRESULT *lResult)
{
	return FALSE;
}

static void onDestroy(void *data)
{
	// TODO
}

static void tabPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	// TODO
}

static void tabAppend(uiTab *tt, const char *name, uiControl *child)
{
}

static void tabInsertAt(uiTab *tt, const char *name, uintmax_t n, uiControl *child)
{
}

static void tabDelete(uiTab *tt, uintmax_t n)
{
}

static uintmax_t tabNumPages(uiTab *tt)
{
	return 0;
}

static int tabMargined(uiTab *tt, uintmax_t n)
{
	return 0;
}

static void tabSetMargined(uiTab *tt, uintmax_t n, int margined)
{
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

	uiControl(t)->PreferredSize = tabPreferredSize;

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
