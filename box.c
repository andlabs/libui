// 7 april 2015
#include "ui.h"
#include "uipriv.h"

struct box {
	uiBox b;
	void (*baseDestroy)(uiControl *);
	struct ptrArray *controls;
	int vertical;
	void (*baseSetParent)(uiControl *, uiContainer *);
	uiContainer *parent;
	int padded;
};

struct boxControl {
	uiControl *c;
	int stretchy;
	intmax_t width;		// both used by resize(); preallocated to save time and reduce risk of failure
	intmax_t height;
};

static void boxDestroy(uiControl *c)
{
	struct box *b = (struct box *) c;
	struct boxControl *bc;

	if (b->parent != NULL)
		complain("attempt to destroy uiBox %p while it has a parent", b);
	// don't chain up to base here; we need to destroy children ourselves first
	while (b->controls->len != 0) {
		bc = ptrArrayIndex(b->controls, struct boxControl *, 0);
		uiControlSetParent(bc->c, NULL);
		uiControlDestroy(bc->c);
		ptrArrayDelete(b->controls, 0);
		uiFree(bc);
	}
	ptrArrayDestroy(b->controls);
	// NOW we can chain up to base
	(*(b->baseDestroy))(uiControl(b));
	uiFree(b);
}

static void boxSetParent(uiControl *c, uiContainer *parent)
{
	struct box *b = (struct box *) c;

	// this does all the actual work
	(*(b->baseSetParent))(uiControl(b), parent);
	// we just need to have a copy of the parent ourselves for boxSetPadded()
	b->parent = parent;
}

static void boxPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct box *b = (struct box *) c;
	struct boxControl *bc;
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
		bc = ptrArrayIndex(b->controls, struct boxControl *, i);
		if (!uiControlVisible(bc->c))
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
}

static void boxSysFunc(uiControl *c, uiControlSysFuncParams *p)
{
	struct box *b = (struct box *) c;
	struct boxControl *bc;
	uintmax_t i;

	for (i = 0; i < b->controls->len; i++) {
		bc = ptrArrayIndex(b->controls, struct boxControl *, i);
		uiControlSysFunc(bc->c, p);
	}
}

static void boxResizeChildren(uiContainer *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	struct box *b = (struct box *) c;
	struct boxControl *bc;
	int xpadding, ypadding;
	uintmax_t nStretchy;
	intmax_t stretchywid, stretchyht;
	uintmax_t i;
	intmax_t preferredWidth, preferredHeight;

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
		bc = ptrArrayIndex(b->controls, struct boxControl *, i);
		if (!uiControlVisible(bc->c))
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
		bc = ptrArrayIndex(b->controls, struct boxControl *, i);
		if (!uiControlVisible(bc->c))
			continue;
		if (bc->stretchy) {
			bc->width = stretchywid;
			bc->height = stretchyht;
		}
	}

	// 3) now we can position controls
	for (i = 0; i < b->controls->len; i++) {
		bc = ptrArrayIndex(b->controls, struct boxControl *, i);
		if (!uiControlVisible(bc->c))
			continue;
		uiControlResize(bc->c, x, y, bc->width, bc->height, d);
		if (b->vertical)
			y += bc->height + ypadding;
		else
			x += bc->width + xpadding;
	}
}

static void boxAppend(uiBox *ss, uiControl *c, int stretchy)
{
	struct box *b = (struct box *) ss;
	struct boxControl *bc;

	bc = uiNew(struct boxControl);
	bc->c = c;
	bc->stretchy = stretchy;
	uiControlSetParent(bc->c, uiContainer(b));
	ptrArrayAppend(b->controls, bc);
	uiContainerUpdate(uiContainer(b));
}

static void boxDelete(uiBox *ss, uintmax_t index)
{
	struct box *b = (struct box *) ss;
	struct boxControl *bc;
	uiControl *removed;

	// TODO rearrange this
	// TODO sync call order with that of Destroy()
	bc = ptrArrayIndex(b->controls, struct boxControl *, index);
	removed = bc->c;
	ptrArrayDelete(b->controls, index);
	uiControlSetParent(removed, NULL);
	uiFree(bc);
	uiContainerUpdate(uiContainer(b));
}

static int boxPadded(uiBox *ss)
{
	struct box *b = (struct box *) ss;

	return b->padded;
}

static void boxSetPadded(uiBox *ss, int padded)
{
	struct box *b = (struct box *) ss;

	b->padded = padded;
	uiContainerUpdate(uiContainer(b));
}

uiBox *uiNewHorizontalBox(void)
{
	struct box *b;

	b = uiNew(struct box);

	uiMakeContainer(uiContainer(b));

	b->controls = newPtrArray();

	b->baseDestroy = uiControl(b)->Destroy;
	uiControl(b)->Destroy = boxDestroy;
	b->baseSetParent = uiControl(b)->SetParent;
	uiControl(b)->SetParent = boxSetParent;
	uiControl(b)->PreferredSize = boxPreferredSize;
	uiControl(b)->SysFunc = boxSysFunc;

	uiContainer(b)->ResizeChildren = boxResizeChildren;

	uiBox(b)->Append = boxAppend;
	uiBox(b)->Delete = boxDelete;
	uiBox(b)->Padded = boxPadded;
	uiBox(b)->SetPadded = boxSetPadded;

	return uiBox(b);
}

uiBox *uiNewVerticalBox(void)
{
	uiBox *bb;
	struct box *b;

	bb = uiNewHorizontalBox();
	b = (struct box *) bb;
	b->vertical = 1;
	return bb;
}
