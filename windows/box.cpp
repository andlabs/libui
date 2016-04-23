// 7 april 2015
#include "uipriv_windows.hpp"

// TODO C++-ize

struct uiBox {
	uiWindowsControl c;
	HWND hwnd;
	struct ptrArray *controls;
	int vertical;
	int padded;
};

#define ctrlStretchy(child) childFlag(child)
#define ctrlSetStretchy(child, stretchy) childSetFlag(child, stretchy)
// both used by resize(); preallocated to save time and reduce risk of failure
#define ctrlWidth(child) childIntmax(child, 0)
#define ctrlSetWidth(child, w) childSetIntmax(child, 0, w)
#define ctrlHeight(child) childIntmax(child, 1)
#define ctrlSetHeight(child, h) childSetIntmax(child, 1, h)

static void onDestroy(uiBox *);

uiWindowsDefineControlWithOnDestroy(
	uiBox,								// type name
	uiBoxType,							// type function
	onDestroy(this);						// on destroy
)

static void onDestroy(uiBox *b)
{
	struct child *bc;

	while (b->controls->len != 0) {
		bc = ptrArrayIndex(b->controls, struct child *, 0);
		ptrArrayDelete(b->controls, 0);
		childDestroy(bc);
	}
	ptrArrayDestroy(b->controls);
}

static void minimumSize(uiWindowsControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	uiBox *b = uiBox(c);
	struct child *bc;
	int xpadding, ypadding;
	uintmax_t nStretchy;
	// these two contain the largest minimum width and height of all stretchy controls in the box
	// all stretchy controls will use this value to determine the final minimum size
	intmax_t maxStretchyWidth, maxStretchyHeight;
	uintmax_t i;
	intmax_t minimumWidth, minimumHeight;
	uiWindowsSizing *dself;

	*width = 0;
	*height = 0;
	if (b->controls->len == 0)
		return;

	dself = uiWindowsNewSizing(b->hwnd);

	// 0) get this Box's padding
	xpadding = 0;
	ypadding = 0;
	if (b->padded) {
		xpadding = d->XPadding;
		ypadding = d->YPadding;
	}

	// 1) initialize the desired rect with the needed padding
	// TODO this is wrong if any controls are hidden
	if (b->vertical)
		*height = (b->controls->len - 1) * ypadding;
	else
		*width = (b->controls->len - 1) * xpadding;

	// 2) add in the size of non-stretchy controls and get (but not add in) the largest widths and heights of stretchy controls
	// we still add in like direction of stretchy controls
	nStretchy = 0;
	maxStretchyWidth = 0;
	maxStretchyHeight = 0;
	for (i = 0; i < b->controls->len; i++) {
		bc = ptrArrayIndex(b->controls, struct child *, i);
		if (!childVisible(bc))
			continue;
		childMinimumSize(bc, dself, &minimumWidth, &minimumHeight);
		if (ctrlStretchy(bc)) {
			nStretchy++;
			if (maxStretchyWidth < minimumWidth)
				maxStretchyWidth = minimumWidth;
			if (maxStretchyHeight < minimumHeight)
				maxStretchyHeight = minimumHeight;
		}
		if (b->vertical) {
			if (*width < minimumWidth)
				*width = minimumWidth;
			if (!ctrlStretchy(bc))
				*height += minimumHeight;
		} else {
			if (!ctrlStretchy(bc))
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

	uiWindowsFreeSizing(dself);
}

static void boxRelayout(uiWindowsControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
	uiBox *b = uiBox(c);
	struct child *bc;
	int xpadding, ypadding;
	uintmax_t nStretchy;
	intmax_t stretchywid, stretchyht;
	uintmax_t i;
	intmax_t minimumWidth, minimumHeight;
	uiWindowsSizing *d;

	uiWindowsEnsureMoveWindow(b->hwnd, x, y, width, height);

	if (b->controls->len == 0)
		return;

	d = uiWindowsNewSizing(b->hwnd);

	// -1) get this Box's padding
	xpadding = 0;
	ypadding = 0;
	if (b->padded) {
		xpadding = d->XPadding;
		ypadding = d->YPadding;
	}

	// 0) inset the available rect by the needed padding
	// TODO this is incorrect if any controls are hidden
	if (b->vertical)
		height -= (b->controls->len - 1) * ypadding;
	else
		width -= (b->controls->len - 1) * xpadding;

	// 1) get width and height of non-stretchy controls
	// this will tell us how much space will be left for stretchy controls
	stretchywid = width;
	stretchyht = height;
	nStretchy = 0;
	for (i = 0; i < b->controls->len; i++) {
		bc = ptrArrayIndex(b->controls, struct child *, i);
		if (!childVisible(bc))
			continue;
		if (ctrlStretchy(bc)) {
			nStretchy++;
			continue;
		}
		childMinimumSize(bc, d, &minimumWidth, &minimumHeight);
		if (b->vertical) {		// all controls have same width
			ctrlSetWidth(bc, width);
			ctrlSetHeight(bc, minimumHeight);
			stretchyht -= minimumHeight;
		} else {				// all controls have same height
			ctrlSetWidth(bc, minimumWidth);
			ctrlSetHeight(bc, height);
			stretchywid -= minimumWidth;
		}
	}

	// 2) now get the size of stretchy controls
	if (nStretchy != 0)
		if (b->vertical)
			stretchyht /= nStretchy;
		else
			stretchywid /= nStretchy;
	for (i = 0; i < b->controls->len; i++) {
		bc = ptrArrayIndex(b->controls, struct child *, i);
		if (!childVisible(bc))
			continue;
		if (ctrlStretchy(bc)) {
			ctrlSetWidth(bc, stretchywid);
			ctrlSetHeight(bc, stretchyht);
		}
	}

	// 3) now we can position controls
	// first, make relative to the top-left corner of the container
	x = 0;
	y = 0;
	for (i = 0; i < b->controls->len; i++) {
		bc = ptrArrayIndex(b->controls, struct child *, i);
		if (!childVisible(bc))
			continue;
		childRelayout(bc, x, y, ctrlWidth(bc), ctrlHeight(bc));
		if (b->vertical)
			y += ctrlHeight(bc) + ypadding;
		else
			x += ctrlWidth(bc) + xpadding;
	}

	uiWindowsFreeSizing(d);
}

static void boxContainerUpdateState(uiControl *c)
{
	uiBox *b = uiBox(c);
	struct child *bc;
	uintmax_t i;

	for (i = 0; i < b->controls->len; i++) {
		bc = ptrArrayIndex(b->controls, struct child *, i);
		childUpdateState(bc);
	}
}

static void redoControlIDsZOrder(uiBox *b)
{
	struct child *bc;
	LONG_PTR controlID;
	HWND insertAfter;
	uintmax_t i;

	controlID = 100;
	insertAfter = NULL;
	for (i = 0; i < b->controls->len; i++) {
		bc = ptrArrayIndex(b->controls, struct child *, i);
		childAssignControlIDZOrder(bc, &controlID, &insertAfter);
	}
}

static void boxArrangeChildrenControlIDsZOrder(uiWindowsControl *c)
{
	uiBox *b = uiBox(c);

	redoControlIDsZOrder(b);
}

void uiBoxAppend(uiBox *b, uiControl *c, int stretchy)
{
	struct child *bc;

	bc = newChild(c, uiControl(b), b->hwnd);
	ctrlSetStretchy(bc, stretchy);
	ptrArrayAppend(b->controls, bc);
	redoControlIDsZOrder(b);
	uiWindowsControlQueueRelayout(uiWindowsControl(b));
}

void uiBoxDelete(uiBox *b, uintmax_t index)
{
	struct child *bc;

	bc = ptrArrayIndex(b->controls, struct child *, index);
	ptrArrayDelete(b->controls, index);
	childRemove(bc);
	redoControlIDsZOrder(b);
	uiWindowsControlQueueRelayout(uiWindowsControl(b));
}

int uiBoxPadded(uiBox *b)
{
	return b->padded;
}

void uiBoxSetPadded(uiBox *b, int padded)
{
	b->padded = padded;
	uiWindowsControlQueueRelayout(uiWindowsControl(b));
}

static uiBox *finishNewBox(int vertical)
{
	uiBox *b;

	b = (uiBox *) uiNewControl(uiBoxType());

	b->hwnd = newContainer();

	b->vertical = vertical;
	b->controls = newPtrArray();

	uiWindowsFinishNewControl(b, uiBox);
	uiControl(b)->ContainerUpdateState = boxContainerUpdateState;
	uiWindowsControl(b)->Relayout = boxRelayout;
	uiWindowsControl(b)->ArrangeChildrenControlIDsZOrder = boxArrangeChildrenControlIDsZOrder;

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
