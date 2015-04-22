// 7 april 2015
#include "uipriv.h"

typedef struct box box;
typedef struct boxControl boxControl;

struct box {
	uiBox s;
	boxControl *controls;
	uintmax_t len;
	uintmax_t cap;
	int vertical;
	int hasParent;
	uiOSContainer *osContainer;
	int padded;
	int userHid;
	int containerHid;
	int userDisabled;
	int containerDisabled;
};

struct boxControl {
	uiControl *c;
	int stretchy;
	intmax_t width;		// both used by resize(); preallocated to save time and reduce risk of failure
	intmax_t height;
};

static void boxDestroy(uiControl *c)
{
	box *b = (box *) c;
	uintmax_t i;

	if (b->hasParent)
		complain("attempt to destroy a uiControl at %p while it still has a parent", c);
	for (i = 0; i < b->len; i++) {
		uiControlSetHasParent(b->controls[i].c, 0);
		uiControlSetOSContainer(b->controls[i].c, NULL);
		uiControlDestroy(b->controls[i].c);
	{
	uiFree(b->controls);
	uiFree(b);
}

static uintptr_t boxHandle(uiControl *c)
{
	return 0;
}

static void boxSetHasParent(uiControl *c, int hasParent)
{
	box *b = (box *) c;

	b->hasParent = hasParent;
}

static void boxSetOSContainer(uiControl *c, uiOSContainer *osContainer)
{
	box *b = (box *) c;
	uintmax_t i;
	uiOSContainer *oldcontainer;

	oldcontainer = b->osContainer;
	b->osContainer = osContainer;
	for (i = 0; i < b->len; i++)
		uiControlSetParent(b->controls[i].c, b->osContainer);
	if (oldcontainer != NULL)
		uiOSContainerUpdate(oldcontainer);
	if (b->osContainer != NULL)
		uiOSContainerUpdate(b->osContainer);
}

static void boxPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	box *b = (box *) c;
	int xpadding, ypadding;
	uintmax_t nStretchy;
	// these two contain the largest preferred width and height of all stretchy controls in the box
	// all stretchy controls will use this value to determine the final preferred size
	intmax_t maxStretchyWidth, maxStretchyHeight;
	uintmax_t i;
	intmax_t preferredWidth, preferredHeight;

	*width = 0;
	*height = 0;
	if (b->len == 0)
		return;

	// 0) get this Box's padding
	xpadding = 0;
	ypadding = 0;
	if (b->padded) {
		xpadding = d->xPadding;
		ypadding = d->yPadding;
	}

	// 1) initialize the desired rect with the needed padding
	if (b->vertical)
		*height = (b->len - 1) * ypadding;
	else
		*width = (b->len - 1) * xpadding;

	// 2) add in the size of non-stretchy controls and get (but not add in) the largest widths and heights of stretchy controls
	// we still add in like direction of stretchy controls
	nStretchy = 0;
	maxStretchyWidth = 0;
	maxStretchyHeight = 0;
	for (i = 0; i < b->len; i++) {
		if (!uiControlVisible(b->controls[i].c))
			continue;
		uiControlPreferredSize(b->controls[i].c, d, &preferredWidth, &preferredHeight);
		if (b->controls[i].stretchy) {
			nStretchy++;
			if (maxStretchyWidth < preferredWidth)
				maxStretchyWidth = preferredWidth;
			if (maxStretchyHeight < preferredHeight)
				maxStretchyHeight = preferredHeight;
		}
		if (b->vertical) {
			if (*width < preferredWidth)
				*width = preferredWidth;
			if (!b->controls[i].stretchy)
				*height += preferredHeight;
		} else {
			if (!b->controls[i].stretchy)
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
	box *b = (box *) c;
	int xpadding, ypadding;
	uintmax_t nStretchy;
	intmax_t stretchywid, stretchyht;
	uintmax_t i;
	intmax_t preferredWidth, preferredHeight;

	if (b->len == 0)
		return;

	// -1) get this Box's padding
	xpadding = 0;
	ypadding = 0;
	if (b->padded) {
		xpadding = d->xPadding;
		ypadding = d->yPadding;
	}

	// 0) inset the available rect by the needed padding
	if (b->vertical)
		height -= (b->len - 1) * ypadding;
	else
		width -= (b->len - 1) * xpadding;

	// 1) get width and height of non-stretchy controls
	// this will tell us how much space will be left for stretchy controls
	stretchywid = width;
	stretchyht = height;
	nStretchy = 0;
	for (i = 0; i < b->len; i++) {
		if (!uiControlVisible(b->controls[i].c))
			continue;
		if (b->controls[i].stretchy) {
			nStretchy++;
			continue;
		}
		uiControlPreferredSize(b->controls[i].c, d, &preferredWidth, &preferredHeight);
		if (b->vertical) {		// all controls have same width
			b->controls[i].width = width;
			b->controls[i].height = preferredHeight;
			stretchyht -= preferredHeight;
		} else {				// all controls have same height
			b->controls[i].width = preferredWidth;
			b->controls[i].height = height;
			stretchywid -= preferredWidth;
		}
	}

	// 2) now get the size of stretchy controls
	if (nStretchy != 0)
		if (b->vertical)
			stretchyht /= nStretchy;
		else
			stretchywid /= nStretchy;
	for (i = 0; i < b->len; i++) {
		if (!uiControlVisible(b->controls[i].c))
			continue;
		if (b->controls[i].stretchy) {
			b->controls[i].width = stretchywid;
			b->controls[i].height = stretchyht;
		}
	}

	// 3) now we can position controls
	for (i = 0; i < b->len; i++) {
		if (!uiControlVisible(b->controls[i].c))
			continue;
		uiControlResize(b->controls[i].c, x, y, b->controls[i].width, b->controls[i].height, d);
		if (b->vertical)
			y += b->controls[i].height + ypadding;
		else
			x += b->controls[i].width + xpadding;
	}
}

static int boxVisible(uiControl *c)
{
	box *b = (box *) c;

	return !(b->userHid);
}

static void boxShow(uiControl *c)
{
	box *b = (box *) c;
	uintmax_t i;

	b->userHid = 0;
	if (!b->containerHid) {
		for (i = 0; i < b->len; i++)
			uiControlContainerShow(b->controls[i].c);
		if (b->parent != NULL)
			uiParentUpdate(b->parent);
	}
}

static void boxHide(uiControl *c)
{
	box *b = (box *) c;
	uintmax_t i;

	b->userHid = 1;
	for (i = 0; i < b->len; i++)
		uiControlContainerHide(b->controls[i].c);
	if (b->parent != NULL)
		uiParentUpdate(b->parent);
}

static void boxContainerShow(uiControl *c)
{
	box *b = (box *) c;
	uintmax_t i;

	b->containerHid = 0;
	if (!b->userHid) {
		for (i = 0; i < b->len; i++)
			uiControlContainerShow(b->controls[i].c);
		if (b->parent != NULL)
			uiParentUpdate(b->parent);
	}
}

static void boxContainerHide(uiControl *c)
{
	box *b = (box *) c;
	uintmax_t i;

	b->containerHid = 1;
	for (i = 0; i < b->len; i++)
		uiControlContainerHide(b->controls[i].c);
	if (b->parent != NULL)
		uiParentUpdate(b->parent);
}

static void boxEnable(uiControl *c)
{
	box *b = (box *) c;
	uintmax_t i;

	b->userDisabled = 0;
	if (!b->containerDisabled)
		for (i = 0; i < b->len; i++)
			uiControlContainerEnable(b->controls[i].c);
}

static void boxDisable(uiControl *c)
{
	box *b = (box *) c;
	uintmax_t i;

	b->userDisabled = 1;
	for (i = 0; i < b->len; i++)
		uiControlContainerDisable(b->controls[i].c);
}

static void boxContainerEnable(uiControl *c)
{
	box *b = (box *) c;
	uintmax_t i;

	b->containerDisabled = 0;
	if (!b->userDisabled)
		for (i = 0; i < b->len; i++)
			uiControlContainerEnable(b->controls[i].c);
}

static void boxContainerDisable(uiControl *c)
{
	box *b = (box *) c;
	uintmax_t i;

	b->containerDisabled = 1;
	for (i = 0; i < b->len; i++)
		uiControlContainerDisable(b->controls[i].c);
}

#define boxCapGrow 32

static void boxAppend(uiBox *ss, uiControl *c, int stretchy)
{
	box *b = (box *) ss;

	if (b->len >= b->cap) {
		b->cap += boxCapGrow;
		b->controls = (boxControl *) uiRealloc(b->controls, b->cap * sizeof (boxControl), "boxControl[]");
	}
	uiControlSethasParent(c, 1);
	b->controls[b->len].c = c;
	b->controls[b->len].stretchy = stretchy;
	b->len++;		// must be here for parent updates to work
	if (b->osContainer != NULL) {
		uiControlSetOSContainer(b->controls[b->len - 1].c, b->osContainer);
		uiParentUpdate(b->parent);
	}
}

static void boxDelete(uiBox *ss, uintmax_t index)
{
	box *b = (box *) ss;
	uiControl *removed;
	uintmax_t i;

	removed = b->controls[index].c;
	// TODO switch to memmove?
	for (i = index; i < b->len - 1; i++)
		b->controls[i] = b->controls[i + 1];
	// TODO memset the last one to NULL
	b->len--;
	uiControlSetHasParent(removed, 0);
	if (b->osContainer != NULL) {
		uiControlSetOSContainer(removed, NULL);
		uiParentUpdate(b->parent);
	}
}

static int boxPadded(uiBox *ss)
{
	box *b = (box *) ss;

	return b->padded;
}

static void boxSetPadded(uiBox *ss, int padded)
{
	box *b = (box *) ss;

	b->padded = padded;
	if (b->parent != NULL)
		uiParentUpdate(b->parent);
}

uiBox *uiNewHorizontalBox(void)
{
	box *b;

	b = uiNew(box);

	uiControl(b)->Destroy = boxDestroy;
	uiControl(b)->Handle = boxHandle;
	uiControl(b)->SetHasParent = boxSetHasParent;
	uiControl(b)->SetOSContainer = boxSetOSContainer;
	uiControl(b)->PreferredSize = boxPreferredSize;
	uiControl(b)->Resize = boxResize;
	uiControl(b)->Visible = boxVisible;
	uiControl(b)->Show = boxShow;
	uiControl(b)->Hide = boxHide;
	uiControl(b)->ContainerShow = boxContainerShow;
	uiControl(b)->ContainerHide = boxContainerHide;
	uiControl(b)->Enable = boxEnable;
	uiControl(b)->Disable = boxDisable;
	uiControl(b)->ContainerEnable = boxContainerEnable;
	uiControl(b)->ContainerDisable = boxContainerDisable;

	uiBox(b)->Append = boxAppend;
	uiBox(b)->Delete = boxDelete;
	uiBox(b)->Padded = boxPadded;
	uiBox(b)->SetPadded = boxSetPadded;

	return uiBox(b);
}

uiBox *uiNewVerticalBox(void)
{
	uiBox *ss;
	box *b;

	ss = uiNewHorizontalBox();
	b = (box *) ss;
	b->vertical = 1;
	return ss;
}
