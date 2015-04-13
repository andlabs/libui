// 11 april 2015
#include "uipriv_windows.h"

struct label {
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
	struct label *l = (struct label *) (c->data);

	uiFree(l);
}

// via http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define labelHeight 8

static void preferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiWindowsWindowTextWidth(uiControlHWND(c));
	*height = uiDlgUnitsToY(labelHeight, d->sys->baseY);
}

uiControl *uiNewLabel(const char *text)
{
	uiControl *c;
	struct label *l;
	uiWindowsNewControlParams p;
	WCHAR *wtext;

	p.dwExStyle = 0;
	p.lpClassName = L"static";
	wtext = toUTF16(text);
	p.lpWindowName = wtext;
	// SS_LEFTNOWORDWRAP clips text past the end; SS_NOPREFIX avoids accelerator translation
	// controls are vertically aligned to the top by default (thanks Xeek in irc.freenode.net/#winapi)
	p.dwStyle = SS_LEFTNOWORDWRAP | SS_NOPREFIX;
	p.hInstance = hInstance;
	p.useStandardControlFont = TRUE;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onWM_DESTROY = onWM_DESTROY;
	c = uiWindowsNewControl(&p);
	uiFree(wtext);

	c->preferredSize = preferredSize;

	l = uiNew(struct label);
	c->data = l;

	return c;
}

char *uiLabelText(uiControl *c)
{
	return uiWindowsControlText(c);
}

void uiLabelSetText(uiControl *c, const char *text)
{
	uiWindowsControlSetText(c, text);
}
