// 16 may 2015
#include "uipriv_windows.hpp"
#include	<stdio.h>

struct fixedChild {
	uiControl *c;
	int x;
	int y;
};

struct uiFixed {
	uiWindowsControl c;
	HWND hwnd;
	std::vector<struct fixedChild> *controls;
};

static void fixedRelayout(uiFixed *g)
{
	if (g->controls->size() == 0)
		return;
	for (struct fixedChild &fc : *(g->controls)) {
		if (!uiControlVisible(fc.c))
			continue;
		uiWindowsMoveWindow((HWND) uiControlHandle(fc.c), fc.x, fc.y);
	}
}

static void uiFixedDestroy(uiControl *c)
{
	uiFixed *g = uiFixed(c);

	for (const struct fixedChild &fc : *(g->controls)) {
		uiControlSetParent(fc.c, NULL);
		uiControlDestroy(fc.c);
	}
	delete g->controls;
	uiWindowsEnsureDestroyWindow(g->hwnd);
	uiFreeControl(uiControl(g));
}

uiWindowsControlDefaultHandle(uiFixed)
uiWindowsControlDefaultParent(uiFixed)
uiWindowsControlDefaultSetParent(uiFixed)
uiWindowsControlDefaultToplevel(uiFixed)
uiWindowsControlDefaultVisible(uiFixed)
uiWindowsControlDefaultShow(uiFixed)
uiWindowsControlDefaultHide(uiFixed)
uiWindowsControlDefaultEnabled(uiFixed)
uiWindowsControlDefaultEnable(uiFixed)
uiWindowsControlDefaultDisable(uiFixed)

static void uiFixedSyncEnableState(uiWindowsControl *c, int enabled)
{
	uiFixed *g = uiFixed(c);

	if (uiWindowsShouldStopSyncEnableState(uiWindowsControl(g), enabled))
		return;
	for (const struct fixedChild &fc : *(g->controls))
		uiWindowsControlSyncEnableState(uiWindowsControl(fc.c), enabled);
}

uiWindowsControlDefaultSetParentHWND(uiFixed)

static void uiFixedMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	uiFixed *g = uiFixed(c);

	*width = 0;
	*height = 0;

	for (const struct fixedChild &fc : *(g->controls)) {
		if (!uiControlVisible(fc.c))
			continue;
		if (fc.x > *width)
			*width = fc.x;
		if (fc.y > *height)
			*height = fc.y;
	}
}

static void uiFixedMinimumSizeChanged(uiWindowsControl *c)
{
	uiFixed *g = uiFixed(c);

	if (uiWindowsControlTooSmall(uiWindowsControl(g))) {
		uiWindowsControlContinueMinimumSizeChanged(uiWindowsControl(g));
		return;
	}
	
	fixedRelayout(g);
}

uiWindowsControlDefaultLayoutRect(uiFixed)
uiWindowsControlDefaultAssignControlIDZOrder(uiFixed)

static void uiFixedChildVisibilityChanged(uiWindowsControl *c)
{
	// TODO eliminate the redundancy
	uiWindowsControlMinimumSizeChanged(c);
}

static void fixedArrangeChildren(uiFixed *g)
{
	LONG_PTR controlID;
	HWND insertAfter;

	controlID = 100;
	insertAfter = NULL;
	for (const struct fixedChild &fc : *(g->controls))
		uiWindowsControlAssignControlIDZOrder(uiWindowsControl(fc.c), &controlID, &insertAfter);
}

void uiFixedSize(uiFixed *g, uiControl *control, int *width, int *height) {
	RECT r;
	uiWindowsEnsureGetWindowRect((HWND) uiControlHandle(control), &r);
	*width = r.right-r.left;
	*height = r.bottom-r.top;
}

void uiFixedSetSize(uiFixed *g, uiControl *control, int width, int height) {
	uiWindowsResizeWindow((HWND) uiControlHandle(control), width, height);
}

void uiFixedAppend(uiFixed *g, uiControl *child, int x, int y)
{
	struct fixedChild fc;

	fc.c = child;
	uiControlSetParent(fc.c, uiControl(g));
	uiWindowsControlSetParentHWND(uiWindowsControl(fc.c), g->hwnd);
	fc.x = x;
	fc.y = y;
	g->controls->push_back(fc);
	fixedArrangeChildren(g);
	uiWindowsControlMinimumSizeChanged(uiWindowsControl(g));
}

void uiFixedMove(uiFixed *g, uiControl *child, int x, int y)
{
	for (struct fixedChild &fc : *(g->controls)) {
		if (fc.c == child) {
			fc.x = x;
			fc.y = y;
			uiWindowsControlMinimumSizeChanged(uiWindowsControl(g));
			return;
		}
	}
}

static void onResize(uiWindowsControl *c)
{
	
}

uiFixed *uiNewFixed(void)
{
	uiFixed *g;

	uiWindowsNewControl(uiFixed, g);

	g->hwnd = uiWindowsMakeContainer(uiWindowsControl(g), onResize);
	g->controls = new std::vector<struct fixedChild>;

	return g;
}
