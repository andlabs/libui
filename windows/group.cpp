// 16 may 2015
#include "uipriv_windows.hpp"

struct uiGroup {
	uiWindowsControl c;
	HWND hwnd;
	struct uiControl *child;
	int margined;
};

// from https://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define groupXMargin 6
#define groupYMarginTop 11 /* note this value /includes/ the groupbox label */
#define groupYMarginBottom 7

// unfortunately because the client area of a groupbox includes the frame and caption text, we have to apply some margins ourselves, even if we don't want "any"
// these were deduced by hand based on the standard DLU conversions; the X and Y top margins are the width and height, respectively, of one character cell
// they can be fine-tuned later
#define groupUnmarginedXMargin 4
#define groupUnmarginedYMarginTop 8
#define groupUnmarginedYMarginBottom 3

static void groupMargins(uiGroup *g, int *mx, int *mtop, int *mbottom)
{
	uiWindowsSizing sizing;
	int dummy;

	*mx = groupUnmarginedXMargin;
	*mtop = groupUnmarginedYMarginTop;
	*mbottom = groupUnmarginedYMarginBottom;
	dummy = 1;		// for the bottom conversion
	if (g->margined) {
		*mx = groupXMargin;
		*mtop = groupYMarginTop;
		*mbottom = groupYMarginBottom;
	}
	uiWindowsControlGetSizing(uiWindowsControl(g), &sizing);
	uiWindowsSizingDlgUnitsToPixels(&sizing, mx, mtop);
	uiWindowsSizingDlgUnitsToPixels(&sizing, &dummy, mbottom);
}

static void groupRelayout(uiGroup *g)
{
	uiGroup *g = uiGroup(c);
	uiWindowsSizing *d;

	uiWindowsEnsureMoveWindowDuringResize(g->hwnd, x, y, width, height);

	if (g->child == NULL)
		return;

	d = uiWindowsNewSizing(g->hwnd);
	x = 0;		// make relative to the top-left corner of the groupbox
	y = 0;
	if (g->margined) {
		x += uiWindowsDlgUnitsToX(groupXMargin, d->BaseX);
		y += uiWindowsDlgUnitsToY(groupYMarginTop, d->BaseY);
		width -= 2 * uiWindowsDlgUnitsToX(groupXMargin, d->BaseX);
		height -= uiWindowsDlgUnitsToY(groupYMarginTop, d->BaseY) + uiWindowsDlgUnitsToY(groupYMarginBottom, d->BaseY);
	} else {
		x += uiWindowsDlgUnitsToX(groupUnmarginedXMargin, d->BaseX);
		y += uiWindowsDlgUnitsToY(groupUnmarginedYMarginTop, d->BaseY);
		width -= 2 * uiWindowsDlgUnitsToX(groupUnmarginedXMargin, d->BaseX);
		height -= uiWindowsDlgUnitsToY(groupUnmarginedYMarginTop, d->BaseY) + uiWindowsDlgUnitsToY(groupUnmarginedYMarginBottom, d->BaseY);
	}
	uiWindowsFreeSizing(d);
	childRelayout(g->child, x, y, width, height);
}

static void uiGroupDestroy(uiControl *c)
{
	uiGroup *g = uiGroup(c);

	if (g->child != NULL) {
		uiControlSetParent(g->child, NULL);
		uiControlDestroy(g->child);
	}
	uiWindowsEnsureDestroyWindow(g->hwnd);
	uiFreeControl(uiControl(g));
}

uiWindowsControlDefaultHandle(uiGroup)
uiWindowsControlDefaultParent(uiGroup)
uiWindowsControlDefaultSetParent(uiGroup)
uiWindowsControlDefaultToplevel(uiGroup)
uiWindowsControlDefaultVisible(uiGroup)
uiWindowsControlDefaultShow(uiGroup)
uiWindowsControlDefaultHide(uiGroup)
uiWindowsControlDefaultEnabled(uiGroup)
uiWindowsControlDefaultEnable(uiGroup)
uiWindowsControlDefaultDisable(uiGroup)

static void uiGroupSyncEnableState(uiWindowsControl *c, int enabled)
{
	uiGroup *g = uiGroup(c);

	if (uiWindowsShouldStopSyncEnableState(uiWindowsControl(g), enabled))
		return;
	EnableWindow(g->hwnd, enabled);
	if (g->child != NULL)
		uiWindowsControlSyncEnableState(uiWindowsControl(g->child), enabled);
}

uiWindowsControlDefaultSetParentHWND(uiGroup)

static void uiGroupMinimumSize(uiWindowsControl *c, intmax_t *width, intmax_t *height)
{
	uiGroup *g = uiGroup(c);
	int mx, mtop, mbottom;

	*width = 0;
	*height = 0;
	if (g->child != NULL)
		uiWindowsControlMinimumSize(uiWindowsControl(g->child), width, height);
	groupMargins(g, &mx, &mtop, &mbottom);
	*width += 2 * mx;
	*height += mtop + mbottom;
	// TODO label width? and when?
}

static void uiGroupMinimumSizeChanged(uiWindowsControl *c)
{
	uiGroup *g = uiGroup(c);

	if (uiWindowsControlTooSmall(uiWindowsControl(g))) {
		uiWindowsControlContinueMinimumSizeChanged(uiWindowsControl(g));
		return;
	}
	groupRelayout(g);
}

uiWindowsControlDefaultLayoutRect(uiGroup)
uiWindowsControlDefaultAssignControlIDZorder(uiGroup)

static void groupArrangeChildrenControlIDsZOrder(uiWindowsControl *c)
{
	uiGroup *g = uiGroup(c);

	if (g->child != NULL)
		childSetSoleControlID(g->child);
}

char *uiGroupTitle(uiGroup *g)
{
	return uiWindowsWindowText(g->hwnd);
}

void uiGroupSetTitle(uiGroup *g, const char *text)
{
	uiWindowsSetWindowText(g->hwnd, text);
	// changing the text might necessitate a change in the groupbox's size
	uiWindowsControlQueueRelayout(uiWindowsControl(g));
}

void uiGroupSetChild(uiGroup *g, uiControl *child)
{
	if (g->child != NULL) {
		uiControlSetParent(g->child, NULL);
		uiWindowsControlSetParentHWND(uiWindowsControl(g->child), NULL);
	}
	g->child = child;
	if (g->child != NULL) {
		uiControlSetParent(g->child, uiControl(g));
		uiWindowsControlSetParentHWND(uiWindowsControl(g->child), g->hwnd);
		uiWindowsControlAssignSoleControlIDZOrder(uiWindowsControl(g->child));
		uiWindowsControlChildMinimumSizeChanged(uiWindowsControl(g));
	}
}

int uiGroupMargined(uiGroup *g)
{
	return g->margined;
}

void uiGroupSetMargined(uiGroup *g, int margined)
{
	g->margined = margined;
	uiWindowsControlChildMinimumSizeChanged(uiWindowsControl(g));
}

static LRESULT CALLBACK groupSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	uiGroup *g = uiGroup(dwRefData);
	LRESULT lResult;

	if (handleParentMessages(hwnd, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;
	switch (uMsg) {
	case WM_WINDOWPOSCHANGED:
		// TODO check
		// TODO add check in container.c
		groupRelayout(g);
		// TODO is this right?
		break;
	// TODO WM_GETMINMAXINFO
	case WM_NCDESTROY:
		if (RemoveWindowSubclass(hwnd, groupSubProc, uIdSubclass) == FALSE)
			logLastError(L"error removing groupbox subclass");
		break;
	}
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

uiGroup *uiNewGroup(const char *text)
{
	uiGroup *g;
	WCHAR *wtext;

	uiWindowsNewControl(uiGroup, g);

	wtext = toUTF16(text);
	g->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CONTROLPARENT,
		L"button", wtext,
		BS_GROUPBOX,
		hInstance, NULL,
		TRUE);
	uiFree(wtext);

	if (SetWindowSubclass(g->hwnd, groupSubProc, 0, (DWORD_PTR) g) == FALSE)
		logLastError(L"error subclassing groupbox to handle parent messages");

	return g;
}
