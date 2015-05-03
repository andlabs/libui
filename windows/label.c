// 11 april 2015
#include "uipriv_windows.h"

struct label {
	uiLabel l;
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
	struct label *l = (struct label *) data;

	uiFree(l);
}

// via http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define labelHeight 8

static void preferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct label *l = (struct label *) c;

	*width = uiWindowsWindowTextWidth(l->hwnd);
	*height = uiDlgUnitsToY(labelHeight, d->sys->baseY);
}

static char *labelText(uiLabel *l)
{
	return uiWindowsControlText(uiControl(l));
}

static void labelSetText(uiLabel *l, const char *text)
{
	uiWindowsControlSetText(uiControl(l), text);
}

uiLabel *uiNewLabel(const char *text)
{
	struct label *l;
	uiWindowsMakeControlParams p;
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
	p.onDestroy = onDestroy;
	p.onDestroyData = l;
	uiWindowsMakeControl(uiControl(l), &p);
	uiFree(wtext);

	l->hwnd = (HWND) uiControlHandle(uiControl(l));

	uiControl(l)->PreferredSize = preferredSize;

	uiLabel(l)->Text = labelText;
	uiLabel(l)->SetText = labelSetText;

	return uiLabel(l);
}
