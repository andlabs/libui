// 7 april 2015
#include "uipriv_windows.hpp"

struct boxChild {
	uiControl *c;
	int stretchy;
	intmax_t width;
	intmax_t height;
};

struct uiBox {
	uiWindowsControl c;
	HWND hwnd;
	std::vector<struct boxChild> *controls;
	int vertical;
	int padded;
};

static void boxPadding(uiBox *b, int *xpadding, int *ypadding)
{
	uiWindowsSizing sizing;

	*xpadding = 0;
	*ypadding = 0;
	if (b->padded) {
		uiWindowsGetSizing(b->hwnd, &sizing);
		uiWindowsSizingStandardPadding(&sizing, xpadding, ypadding);
	}
}

static void boxRelayout(uiBox *b)
{
	RECT r;
	intmax_t x, y, width, height;
	int xpadding, ypadding;
	uintmax_t nStretchy;
	intmax_t stretchywid, stretchyht;
	uintmax_t i;
	intmax_t minimumWidth, minimumHeight;
	uiWindowsSizing *d;

	if (b->controls->size() == 0)
		return;

	uiWindowsEnsureGetClientRect(b->hwnd, &r);
	x = r.left;
	y = r.top;
	width = r.right - r.left;
	height = r.bottom - r.top;

	// -1) get this Box's padding
	boxPadding(b, &xpadding, &ypadding);

	// 0) inset the available rect by the needed padding
	// TODO this is incorrect if any controls are hidden
	if (b->vertical)
		height -= (b->controls->size() - 1) * ypadding;
	else
		width -= (b->controls->size() - 1) * xpadding;

	// 1) get width and height of non-stretchy controls
	// this will tell us how much space will be left for stretchy controls
	stretchywid = width;
	stretchyht = height;
	nStretchy = 0;
	for (struct boxChild &bc : *(b->controls)) {
		if (!uiControlVisible(bc.c))
			continue;
		if (bc.stretchy) {
			nStretchy++;
			continue;
		}
		uiWindowsControlMinimumSize(uiWindowsControl(bc.c), &minimumWidth, &minimumHeight);
		if (b->vertical) {		// all controls have same width
			bc.width = width;
			bc.height = minimumHeight;
			stretchyht -= minimumHeight;
		} else {				// all controls have same height
			bc.width = minimumWidth;
			bc.height = height;
			stretchywid -= minimumWidth;
		}
	}

	// 2) now get the size of stretchy controls
	if (nStretchy != 0)
		if (b->vertical)
			stretchyht /= nStretchy;
		else
			stretchywid /= nStretchy;
	// TODO put this in the above if
	for (struct boxChild &bc : *(b->controls)) {
		if (!uiControlVisible(bc.c))
			continue;
		if (bc.stretchy) {
			bc.width = stretchywid;
			bc.height = stretchyht;
		}
	}

	// 3) now we can position controls
	// first, make relative to the top-left corner of the container
	x = 0;
	y = 0;
	for (const struct boxChild &bc : *(b->controls)) {
		if (!uiControlVisible(bc.c))
			continue;
		uiWindowsEnsureMoveWindowDuringResize((HWND) uiControlHandle(bc.c), x, y, bc.width, bc.height);
		if (b->vertical)
			y += bc.height + ypadding;
		else
			x += bc.width + xpadding;
	}
}

static void uiBoxDestroy(uiControl *c)
{
	uiBox *b = uiBox(c);

	for (const struct boxChild &bc : *(b->controls)) {
		uiControlSetParent(bc.c, NULL);
		uiControlDestroy(bc.c);
	}
	delete b->controls;
	uiWindowsEnsureDestroyWindow(b->hwnd);
	uiFreeControl(uiControl(b));
}

uiWindowsControlDefaultHandle(uiBox)
uiWindowsControlDefaultParent(uiBox)
uiWindowsControlDefaultSetParent(uiBox)
uiWindowsControlDefaultToplevel(uiBox)
uiWindowsControlDefaultVisible(uiBox)
uiWindowsControlDefaultShow(uiBox)
uiWindowsControlDefaultHide(uiBox)
uiWindowsControlDefaultEnabled(uiBox)
uiWindowsControlDefaultEnable(uiBox)
uiWindowsControlDefaultDisable(uiBox)

static void uiBoxSyncEnableState(uiWindowsControl *c, int enabled)
{
	uiBox *b = uiBox(c);

	if (uiWindowsShouldStopSyncEnableState(uiWindowsControl(b), enabled))
		return;
	for (const struct boxChild &bc : *(b->controls))
		uiWindowsControlSyncEnableState(uiWindowsControl(bc.c), enabled);
}

uiWindowsControlDefaultSetParentHWND(uiBox)

static void uiBoxMinimumSize(uiWindowsControl *c, intmax_t *width, intmax_t *height)
{
	uiBox *b = uiBox(c);
	int xpadding, ypadding;
	uintmax_t nStretchy;
	// these two contain the largest minimum width and height of all stretchy controls in the box
	// all stretchy controls will use this value to determine the final minimum size
	intmax_t maxStretchyWidth, maxStretchyHeight;
	uintmax_t i;
	intmax_t minimumWidth, minimumHeight;
	uiWindowsSizing sizing;

	*width = 0;
	*height = 0;
	if (b->controls->size() == 0)
		return;

	// 0) get this Box's padding
	boxPadding(b, &xpadding, &ypadding);

	// 1) initialize the desired rect with the needed padding
	// TODO this is wrong if any controls are hidden
	if (b->vertical)
		*height = (b->controls->size() - 1) * ypadding;
	else
		*width = (b->controls->size() - 1) * xpadding;

	// 2) add in the size of non-stretchy controls and get (but not add in) the largest widths and heights of stretchy controls
	// we still add in like direction of stretchy controls
	nStretchy = 0;
	maxStretchyWidth = 0;
	maxStretchyHeight = 0;
	for (const struct boxChild &bc : *(b->controls)) {
		if (!uiControlVisible(bc.c))
			continue;
		uiWindowsControlMinimumSize(uiWindowsControl(bc.c), &minimumWidth, &minimumHeight);
		if (bc.stretchy) {
			nStretchy++;
			if (maxStretchyWidth < minimumWidth)
				maxStretchyWidth = minimumWidth;
			if (maxStretchyHeight < minimumHeight)
				maxStretchyHeight = minimumHeight;
		}
		if (b->vertical) {
			if (*width < minimumWidth)
				*width = minimumWidth;
			if (!bc.stretchy)
				*height += minimumHeight;
		} else {
			if (!bc.stretchy)
				*width += minimumWidth;
			if (*height < minimumHeight)
				*height = minimumHeight;
		}
	}

	// 3) and now we can add in stretchy controls
	if (b->vertical)
		*height += nStretchy * maxStretchyHeight;
	else
		*width += nStretchy * maxStretchyWidth;
}

static void uiBoxMinimumSizeChanged(uiWindowsControl *c)
{
	uiBox *b = uiBox(c);

	if (uiWindowsControlTooSmall(uiWindowsControl(b))) {
		uiWindowsControlContinueMinimumSizeChanged(uiWindowsControl(b));
		return;
	}
	boxRelayout(b);
}

uiWindowsControlDefaultLayoutRect(uiBox)
uiWindowsControlDefaultAssignControlIDZOrder(uiBox)

static void boxArrangeChildren(uiBox *b)
{
	LONG_PTR controlID;
	HWND insertAfter;
	uintmax_t i;

	controlID = 100;
	insertAfter = NULL;
	for (const struct boxChild &bc : *(b->controls))
		uiWindowsControlAssignControlIDZOrder(uiWindowsControl(bc.c), &controlID, &insertAfter);
}

void uiBoxAppend(uiBox *b, uiControl *c, int stretchy)
{
	struct boxChild bc;

	bc.c = c;
	bc.stretchy = stretchy;
	uiControlSetParent(bc.c, uiControl(b));
	uiWindowsControlSetParentHWND(uiWindowsControl(bc.c), b->hwnd);
	b->controls->push_back(bc);
	boxArrangeChildren(b);
	uiWindowsControlMinimumSizeChanged(uiWindowsControl(b));
}

void uiBoxDelete(uiBox *b, uintmax_t index)
{
	uiControl *c;

	c = (*(b->controls))[index].c;
	uiControlSetParent(c, NULL);
	uiWindowsControlSetParentHWND(uiWindowsControl(c), NULL);
	b->controls->erase(b->controls->begin() + index);
	boxArrangeChildren(b);
	uiWindowsControlMinimumSizeChanged(uiWindowsControl(b));
}

int uiBoxPadded(uiBox *b)
{
	return b->padded;
}

void uiBoxSetPadded(uiBox *b, int padded)
{
	b->padded = padded;
	uiWindowsControlMinimumSizeChanged(uiWindowsControl(b));
}

static void onResize(uiWindowsControl *c)
{
	boxRelayout(uiBox(c));
}

static uiBox *finishNewBox(int vertical)
{
	uiBox *b;

	uiWindowsNewControl(uiBox, b);

	b->hwnd = uiWindowsMakeContainer(uiWindowsControl(b), onResize);

	b->vertical = vertical;
	b->controls = new std::vector<struct boxChild>;

	return b;
}

uiBox *uiNewHorizontalBox(void)
{
	return finishNewBox(0);
}

uiBox *uiNewVerticalBox(void)
{
	return finishNewBox(1);
}
