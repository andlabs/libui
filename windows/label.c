// 11 april 2015
#include "uipriv_windows.h"

struct label {
	uiLabel l;
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
	struct label *l = (struct label *) c;

	uiFree(l);
}

// via http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define labelHeight 8

static void preferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiWindowsWindowTextWidth(uiControlHWND(c));
	*height = uiDlgUnitsToY(labelHeight, d->sys->baseY);
}

static char *getText(uiLabel *l)
{
	return uiWindowsControlText(uiControl(l));
}

static void setText(uiLabel *l, const char *text)
{
	uiWindowsControlSetText(uiControl(l), text);
}

uiLabel *uiNewLabel(const char *text)
{
	struct label *l;
	uiWindowsNewControlParams p;
	WCHAR *wtext;

	l = uiNew(struct label);

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
	uiWindowsNewControl(uiControl(l), &p);
	uiFree(wtext);

	uiControl(l)->PreferredSize = preferredSize;

	uiLabel(l)->Text = getText;
	uiLabel(l)->SetText = setText;

	return uiLabel(l);
}
