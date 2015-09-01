// 7 april 2015
#include "uipriv_windows.h"

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
/* TODO
	uiBox *b = uiBox(c);
	struct child *bc;
	int xpadding, ypadding;
	uintmax_t nStretchy;
	// these two contain the largest preferred width and height of all stretchy controls in the box
	// all stretchy controls will use this value to determine the final preferred size
	intmax_t maxStretchyWidth, maxStretchyHeight;
	uintmax_t i;
	intmax_t preferredWidth, preferredHeight;

	*width = 0;
	*height = 0;
	if (b->controls->len == 0)
		return;

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
		if (!uiControlContainerVisible(bc->c))
			continue;
		uiControlPreferredSize(bc->c, d, &preferredWidth, &preferredHeight);
		if (bc->stretchy) {
			nStretchy++;
			if (maxStretchyWidth < preferredWidth)
				maxStretchyWidth = preferredWidth;
			if (maxStretchyHeight < preferredHeight)
				maxStretchyHeight = preferredHeight;
		}
		if (b->vertical) {
			if (*width < preferredWidth)
				*width = preferredWidth;
			if (!bc->stretchy)
				*height += preferredHeight;
		} else {
			if (!bc->stretchy)
				*width += preferredWidth;
			if (*height < preferredHeight)
				*height = preferredHeight;
		}
	}

	// 3) and now we can add in stretchy controls
	if (b->vertical)
		*height += nStretchy * maxStretchyHeight;
	else
		*width += nStretchy * maxStretchyWidth;
*/
}

static void boxRelayout(uiWindowsControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
/* TODO
	uibox *b = uiBox(c);
	struct child *bc;
	int xpadding, ypadding;
	uintmax_t nStretchy;
	intmax_t stretchywid, stretchyht;
	uintmax_t i;
	intmax_t preferredWidth, preferredHeight;
	uiWindowsSizing *dchild;

	(*(b->baseResize))(uiControl(b), x, y, width, height, d);

	if (b->controls->len == 0)
		return;

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
		if (!uiControlContainerVisible(bc->c))
			continue;
		if (bc->stretchy) {
			nStretchy++;
			continue;
		}
		uiControlPreferredSize(bc->c, d, &preferredWidth, &preferredHeight);
		if (b->vertical) {		// all controls have same width
			bc->width = width;
			bc->height = preferredHeight;
			stretchyht -= preferredHeight;
		} else {				// all controls have same height
			bc->width = preferredWidth;
			bc->height = height;
			stretchywid -= preferredWidth;
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
		if (!uiControlContainerVisible(bc->c))
			continue;
		if (bc->stretchy) {
			bc->width = stretchywid;
			bc->height = stretchyht;
		}
	}

	// 3) now we can position controls
	dchild = uiControlSizing(uiControl(b));
	for (i = 0; i < b->controls->len; i++) {
		bc = ptrArrayIndex(b->controls, struct child *, i);
		if (!uiControlContainerVisible(bc->c))
			continue;
		uiControlResize(bc->c, x, y, bc->width, bc->height, dchild);
		if (b->vertical)
			y += bc->height + ypadding;
		else
			x += bc->width + xpadding;
	}
	uiFreeSizing(dchild);
*/
}

/* TODO
static int boxHasTabStops(uiControl *c)
{
	uiBox *b = uiBox(c);
	struct child *bc;
	uintmax_t i;

	for (i = 0; i < b->controls->len; i++) {
		bc = ptrArrayIndex(b->controls, struct child *, i);
		if (uiControlHasTabStops(bc->c))
			return 1;
	}
	return 0;
}
*/

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

void uiBoxAppend(uiBox *b, uiControl *c, int stretchy)
{
	struct child *bc;
/* TODO
	uintptr_t zorder;
	int dozorder;
	uintmax_t i;

	// start the zorder with the *CURRENT* first child
	// this is in case we're adding a new first child
	dozorder = 0;
	if (b->controls->len != 0) {
		dozorder = 1;
		bc = ptrArrayIndex(b->controls, struct child *, 0);
		zorder = uiControlStartZOrder(bc->c);
	}
*/

	bc = newChild(c, uiControl(b), b->hwnd);
	ctrlSetStretchy(bc, stretchy);
	ptrArrayAppend(b->controls, bc);
	uiWindowsControlQueueRelayout(uiWindowsControl(b));

/* TODO
	// and now update the zorder for all controls
	if (dozorder)
		for (i = 0; i < b->controls->len; i++) {
			bc = ptrArrayIndex(b->controls, struct child *, i);
			zorder = uiControlSetZOrder(bc->c, zorder);
		}
*/
}

void uiBoxDelete(uiBox *b, uintmax_t index)
{
	struct child *bc;

	bc = ptrArrayIndex(b->controls, struct child *, index);
	ptrArrayDelete(b->controls, index);
	childRemove(bc);
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
