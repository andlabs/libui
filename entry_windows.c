// 8 april 2015
#include "uipriv_windows.h"

struct entry {
	uiControl *c;
};

#define E(x) ((struct entry *) (x))

static BOOL onWM_COMMAND(uiControl *c, WPARAM wParam, LPARAM lParam, void *data, LRESULT *lResult)
{
	return FALSE;
}

static BOOL onWM_NOTIFY(uiControl *c, WPARAM wParam, LPARAM lParam, void *data, LRESULT *lResult)
{
	return FALSE;
}

static void onWM_DESTROY(uiControl *c, void *data)
{
	struct entry *e = (struct entry *) data;

	uiFree(e);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define entryWidth 107 /* this is actually the shorter progress bar width, but Microsoft only indicates as wide as necessary */
#define entryHeight 14

static void preferredSize(uiControl *c, int baseX, int baseY, LONG internalLeading, intmax_t *width, intmax_t *height)
{
	*width = uiDlgUnitToX(entryWidth, baseX);
	*height = uiDlgUnitToY(entryHeight, baseY);
}

uiControl *uiNewEntry(void)
{
	struct entry *e;
	uiWindowsNewControlParams p;
	HWND hwnd;

	e = uiNew(struct entry);

	p.dwExStyle = WS_EX_CLIENTEDGE;
	p.lpClassName = L"edit";
	p.lpWindowName = L"";
	// TODO ES_NOHIDESEL?
	p.dwStyle = ES_AUTOHSCROLL | ES_LEFT | WS_TABSTOP;
	p.hInstance = hInstance;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onWM_DESTROY = onWM_DESTROY;
	p.onCommandNotifyDestroyData = e;
	p.preferredSize = preferredSize;
	p.data = e;
	e->c = uiWindowsNewControl(&p);

	hwnd = (HWND) uiControlHandle(e->c);
	SendMessageW(hwnd, WM_SETFONT, (WPARAM) hMessageFont, (LPARAM) TRUE);

	return e->c;
}

char *uiEntryText(uiControl *c)
{
	return uiWindowsControlText(c);
}

void uiEntrySetText(uiControl *c, const char *text)
{
	uiWindowsControlSetText(c, text);
}
