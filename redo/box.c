// 7 april 2015
#include "out/ui.h"
#include "uipriv.h"

struct box {
	uiBox b;
	void (*baseCommitDestroy)(uiControl *);
	uintptr_t handle;
	struct ptrArray *controls;
	int vertical;
	int padded;
	void (*baseResize)(uiControl *, intmax_t, intmax_t, intmax_t, intmax_t, uiSizing *);
};

struct boxControl {
	uiControl *c;
	int stretchy;
	intmax_t width;		// both used by resize(); preallocated to save time and reduce risk of failure
	intmax_t height;
};

static void boxCommitDestroy(uiControl *c)
{
	struct box *b = (struct box *) c;
	struct boxControl *bc;

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
	(*(b->baseCommitDestroy))(uiControl(b));
}

static uintptr_t boxHandle(uiControl *c)
{
	struct box *b = (struct box *) c;

	return b->handle;
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
		if (!uiControlContainerVisible(bc->c))
			continue;
		// TODO dchild
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

static void boxResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	struct box *b = (struct box *) c;
	struct boxControl *bc;
	int xpadding, ypadding;
	uintmax_t nStretchy;
	intmax_t stretchywid, stretchyht;
	uintmax_t i;
	intmax_t preferredWidth, preferredHeight;
	uiSizing *dchild;

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
		bc = ptrArrayIndex(b->controls, struct boxControl *, i);
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
		bc = ptrArrayIndex(b->controls, struct boxControl *, i);
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
		bc = ptrArrayIndex(b->controls, struct boxControl *, i);
		if (!uiControlContainerVisible(bc->c))
			continue;
		uiControlResize(bc->c, x, y, bc->width, bc->height, dchild);
		if (b->vertical)
			y += bc->height + ypadding;
		else
			x += bc->width + xpadding;
	}
	uiFreeSizing(dchild);
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

static void boxAppend(uiBox *ss, uiControl *c, int stretchy)
{
	struct box *b = (struct box *) ss;
	struct boxControl *bc;

	bc = uiNew(struct boxControl);
	bc->c = c;
	bc->stretchy = stretchy;
	uiControlSetParent(bc->c, uiControl(b));
	ptrArrayAppend(b->controls, bc);
	uiControlQueueResize(uiControl(b));
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
	uiControlQueueResize(uiControl(b));
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
	uiControlQueueResize(uiControl(b));
}

uiBox *uiNewHorizontalBox(void)
{
	struct box *b;

	b = uiNew(struct box);
	uiTyped(b)->Type = uiTypeBox();

	b->handle = uiMakeContainer(uiControl(b));

	b->controls = newPtrArray();

	uiControl(b)->Handle = boxHandle;
	uiControl(b)->PreferredSize = boxPreferredSize;
	b->baseResize = uiControl(b)->Resize;
	uiControl(b)->Resize = boxResize;
	uiControl(b)->SysFunc = boxSysFunc;
	b->baseCommitDestroy = uiControl(b)->CommitDestroy;
	uiControl(b)->CommitDestroy = boxCommitDestroy;

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
