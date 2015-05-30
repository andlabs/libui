// 11 may 2015
#include "uipriv_windows.h"

struct group {
	uiGroup g;
	HWND hwnd;
	uiControl *child;
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

	if (g->child != NULL) {
		uiControlSetParent(g->child, NULL);
		uiControlDestroy(g->child);
	}
	uiFree(g);
}

// TODO get source
#define groupXMargin 6
#define groupYMarginTop 11 /* note this value /includes the groupbox label */
#define groupYMarginBottom 7

// unfortunately because the client area of a groupbox includes the frame and caption text, we have to apply some margins ourselves, even if we don't want "any"
// these were deduced by hand based on the standard DLU conversions; the X and Y top margins are the width and height, respectively, of one character cell
// they can be fine-tuned later
#define groupUnmarginedXMargin 4
#define groupUnmarginedYMarginTop 8
#define groupUnmarginedYMarginBottom 3

static void groupPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct group *g = (struct group *) c;

	uiControlPreferredSize(g->child, d, width, height);
	*width += uiWindowsDlgUnitsToX(groupUnmarginedXMargin, d->Sys->BaseX) * 2;
	*height += uiWindowsDlgUnitsToY(groupUnmarginedYMarginTop, d->Sys->BaseY) + uiWindowsDlgUnitsToY(groupUnmarginedYMarginBottom, d->Sys->BaseY);
}

static void groupResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	struct group *g = (struct group *) c;

	(*(g->baseResize))(uiControl(g), x, y, width, height, d);
	if (g->child != NULL)
		uiControlQueueResize(g->child);
}

static void groupComputeChildSize(uiControl *c, intmax_t *x, intmax_t *y, intmax_t *width, intmax_t *height, uiSizing *d)
{
	struct group *g = (struct group *) c;
	RECT r;

	if (GetClientRect(g->hwnd, &r) == 0)
		logLastError("error getting uiGroup client rect for computing child size in groupResize()");
	r.left += uiWindowsDlgUnitsToX(groupUnmarginedXMargin, d->Sys->BaseX);
	r.top += uiWindowsDlgUnitsToY(groupUnmarginedYMarginTop, d->Sys->BaseY);
	r.right -= uiWindowsDlgUnitsToX(groupUnmarginedXMargin, d->Sys->BaseX);
	r.bottom -= uiWindowsDlgUnitsToY(groupUnmarginedYMarginBottom, d->Sys->BaseY);
	*x = r.left;
	*y = r.top;
	*width = r.right - r.left;
	*height = r.bottom - r.top;
}

static void groupSetChild(uiGroup *gg, uiControl *c)
{
	struct group *g = (struct group *) gg;

	g->child = c;
	if (g->child != NULL)
		uiControlQueueResize(g->child);
}

uiGroup *uiNewGroup(const char *text)
{
	struct group *g;
	uiWindowsMakeControlParams p;
	WCHAR *wtext;

	g = uiNew(struct group);
	uiTyped(g)->Type = uiTypeGroup();

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
	g->baseResize = uiControl(g)->Resize;
	uiControl(g)->Resize = groupResize;
	uiControl(g)->ComputeChildSize = groupComputeChildSize;
	// TODO enable, disable, sysfunc

	uiGroup(g)->SetChild = groupSetChild;

	return uiGroup(g);
}
