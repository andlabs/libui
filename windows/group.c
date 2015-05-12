// 11 may 2015
#include "uipriv_windows.h"

struct group {
	uiGroup g;
	HWND hwnd;
	uiBin *bin;
	void (*baseResize)(uiControl *, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
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
	struct group *g = (struct group *) data;

	uiBinRemoveOSParent(g->bin);
	uiControlDestroy(uiControl(g->bin));
	uiFree(g);
}

static void groupPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct group *g = (struct group *) c;

	uiControlPreferredSize(uiControl(g->bin), d, width, height);
	// TODO
	*width += 20;
	*height += 20;
}

static void groupResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	struct group *g = (struct group *) c;
	RECT r;

	(*(g->baseResize))(uiControl(g), x, y, width, height, d);
	if (GetClientRect(g->hwnd, &r) == 0)
		logLastError("error getting uiGroup client rect for bin resize in groupResize()");
	// TODO
	r.left += 10;
	r.right -= 10;
	r.top += 10;
	r.bottom -= 10;
	uiBinResizeRootAndUpdate(g->bin, r.left, r.top, r.right - r.left, r.bottom - r.top);
}

static void groupSetChild(uiGroup *gg, uiControl *c)
{
	struct group *g = (struct group *) gg;

	uiBinSetMainControl(g->bin, c);
	uiContainerUpdate(uiContainer(g->bin));
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
	p.useStandardControlFont = TRUE;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onDestroy = onDestroy;
	p.onDestroyData = g;
	uiWindowsMakeControl(uiControl(g), &p);
	uiFree(wtext);

	g->hwnd = (HWND) uiControlHandle(uiControl(g));

	g->bin = newBin();
	uiBinSetOSParent(g->bin, (uintptr_t) (g->hwnd));

	uiControl(g)->PreferredSize = groupPreferredSize;
	g->baseResize = uiControl(g)->Resize;
	uiControl(g)->Resize = groupResize;
	// TODO enable, disable, sysfunc

	uiGroup(g)->SetChild = groupSetChild;

	return uiGroup(g);
}
