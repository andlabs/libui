// 8 april 2015
#include "uipriv_windows.h"

struct entry {
};

static BOOL onWM_COMMAND(uiControl *c, WORD code, LRESULT *lResult)
{
	return FALSE;
}

static BOOL onWM_NOTIFY(uiControl *c, NMHDR *nm, LRESULT *lResult)
{
	return FALSE;
}

static void onWM_DESTROY(uiControl *c)
{
	struct entry *e = (struct entry *) (c->data);

	uiFree(e);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define entryWidth 107 /* this is actually the shorter progress bar width, but Microsoft only indicates as wide as necessary */
#define entryHeight 14

static void preferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiDlgUnitsToX(entryWidth, d->sys->baseX);
	*height = uiDlgUnitsToY(entryHeight, d->sys->baseY);
}

uiControl *uiNewEntry(void)
{
	uiControl *c;
	struct entry *e;
	uiWindowsNewControlParams p;

	p.dwExStyle = WS_EX_CLIENTEDGE;
	p.lpClassName = L"edit";
	p.lpWindowName = L"";
	p.dwStyle = ES_AUTOHSCROLL | ES_LEFT | ES_NOHIDESEL | WS_TABSTOP;
	p.hInstance = hInstance;
	p.useStandardControlFont = TRUE;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onWM_DESTROY = onWM_DESTROY;
	c = uiWindowsNewControl(&p);

	c->preferredSize = preferredSize;

	e = uiNew(struct entry);
	c->data = e;

	return c;
}

char *uiEntryText(uiControl *c)
{
	return uiWindowsControlText(c);
}

void uiEntrySetText(uiControl *c, const char *text)
{
	uiWindowsControlSetText(c, text);
}
