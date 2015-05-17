// 16 may 2015
#include "uipriv_windows.h"

struct group {
	uiGroup g;
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

static void groupPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	// TODO
}

static void groupSetChild(uiGroup *gg, uiControl *c)
{
	// TODO
}

uiGroup *uiNewGroup(const char *text)
{
	struct group *g;
	uiWindowsMakeControlParams p;
	WCHAR *wtext;

	g = uiNew(struct group);

	p.dwExStyle = WS_EX_CONTROLPARENT;
	p.lpClassName = L"button";
	wtext = toUTF16(text);
	p.lpWindowName = wtext;
	p.dwStyle = BS_GROUPBOX;
	p.hInstance = hInstance;
	p.lpParam = NULL;
	p.useStandardControlFont = TRUE;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onDestroy = onDestroy;
	p.onDestroyData = g;
	uiWindowsMakeControl(uiControl(g), &p);
	uiFree(wtext);

	g->hwnd = (HWND) uiControlHandle(uiControl(g));

	uiControl(g)->PreferredSize = groupPreferredSize;

	uiGroup(g)->SetChild = groupSetChild;

	return uiGroup(g);
}
