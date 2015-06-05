// 16 may 2015
#include "uipriv_windows.h"

struct group {
	uiGroup g;
	HWND hwnd;
	uiControl *child;
	void (*baseCommitDestroy)(uiControl *);
	int margined;
	void (*baseResize)(uiControl *, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
};

uiDefineControlType(uiGroup, uiTypeGroup, struct group)

static void groupCommitDestroy(uiControl *c)
{
	struct group *g = (struct group *) c;

	if (g->child != NULL) {
		uiControlSetParent(g->child, NULL);
		uiControlDestroy(g->child);
	}
	(*(g->baseCommitDestroy))(uiControl(g));
}

static uintptr_t groupHandle(uiControl *c)
{
	struct group *g = (struct group *) c;

	return (uintptr_t) (g->hwnd);
}

// from https://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
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

	*width = 0;
	*height = 0;
	if (g->child != NULL)
		uiControlPreferredSize(g->child, d, width, height);
	if (g->margined) {
		*width += 2 * uiWindowsDlgUnitsToX(groupXMargin, d->Sys->BaseX);
		*height += uiWindowsDlgUnitsToY(groupYMarginTop, d->Sys->BaseY) + uiWindowsDlgUnitsToY(groupYMarginBottom, d->Sys->BaseY);
	} else {
		*width += 2 * uiWindowsDlgUnitsToX(groupUnmarginedXMargin, d->Sys->BaseX);
		*height += uiWindowsDlgUnitsToY(groupUnmarginedYMarginTop, d->Sys->BaseY) + uiWindowsDlgUnitsToY(groupUnmarginedYMarginBottom, d->Sys->BaseY);
	}
}

static void groupResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	struct group *g = (struct group *) c;
	uiSizing *dchild;

	(*(g->baseResize))(uiControl(g), x, y, width, height, d);

	if (g->child == NULL)
		return;

	if (g->margined) {
		x += uiWindowsDlgUnitsToX(groupXMargin, d->Sys->BaseX);
		y += uiWindowsDlgUnitsToY(groupYMarginTop, d->Sys->BaseY);
		width -= 2 * uiWindowsDlgUnitsToX(groupXMargin, d->Sys->BaseX);
		height -= uiWindowsDlgUnitsToY(groupYMarginTop, d->Sys->BaseY) + uiWindowsDlgUnitsToY(groupYMarginBottom, d->Sys->BaseY);
	} else {
		x += uiWindowsDlgUnitsToX(groupUnmarginedXMargin, d->Sys->BaseX);
		y += uiWindowsDlgUnitsToY(groupUnmarginedYMarginTop, d->Sys->BaseY);
		width -= 2 * uiWindowsDlgUnitsToX(groupUnmarginedXMargin, d->Sys->BaseX);
		height -= uiWindowsDlgUnitsToY(groupUnmarginedYMarginTop, d->Sys->BaseY) + uiWindowsDlgUnitsToY(groupUnmarginedYMarginBottom, d->Sys->BaseY);
	}

	dchild = uiControlSizing(uiControl(g));
	uiControlResize(g->child, x, y, width, height, dchild);
	uiFreeSizing(dchild);
}

static void groupContainerUpdateState(uiControl *c)
{
	struct group *g = (struct group *) c;

	if (g->child != NULL)
		uiControlUpdateState(g->child);
}

static char *groupTitle(uiGroup *gg)
{
	struct group *g = (struct group *) gg;

	return uiWindowsUtilText(g->hwnd);
}

static void groupSetTitle(uiGroup *gg, const char *text)
{
	struct group *g = (struct group *) gg;

	uiWindowsUtilSetText(g->hwnd, text);
	// changing the text might necessitate a change in the groupbox's size
	uiControlQueueResize(uiControl(g));
}

static void groupSetChild(uiGroup *gg, uiControl *child)
{
	struct group *g = (struct group *) gg;

	if (g->child != NULL)
		uiControlSetParent(g->child, NULL);
	g->child = child;
	if (g->child != NULL) {
		uiControlSetParent(g->child, uiControl(g));
		uiControlQueueResize(g->child);
	}
}

static int groupMargined(uiGroup *gg)
{
	struct group *g = (struct group *) gg;

	return g->margined;
}

static void groupSetMargined(uiGroup *gg, int margined)
{
	struct group *g = (struct group *) gg;

	g->margined = margined;
	uiControlQueueResize(uiControl(g));
}

uiGroup *uiNewGroup(const char *text)
{
	struct group *g;
	WCHAR *wtext;

	g = (struct group *) uiWindowsNewSingleHWNDControl(uiTypeGroup());

	wtext = toUTF16(text);
	g->hwnd = uiWindowsUtilCreateControlHWND(WS_EX_CONTROLPARENT,
		L"button", wtext,
		BS_GROUPBOX,
		hInstance, NULL,
		TRUE);
	uiFree(wtext);

	uiControl(g)->Handle = groupHandle;
	uiControl(g)->PreferredSize = groupPreferredSize;
	g->baseResize = uiControl(g)->Resize;
	uiControl(g)->Resize = groupResize;
	g->baseCommitDestroy = uiControl(g)->CommitDestroy;
	uiControl(g)->CommitDestroy = groupCommitDestroy;
	uiControl(g)->ContainerUpdateState = groupContainerUpdateState;

	uiGroup(g)->Title = groupTitle;
	uiGroup(g)->SetTitle = groupSetTitle;
	uiGroup(g)->SetChild = groupSetChild;
	uiGroup(g)->Margined = groupMargined;
	uiGroup(g)->SetMargined = groupSetMargined;

	return uiGroup(g);
}
