// 7 april 2015
#include "uipriv.h"

// TODO
// - rename to uiBox

typedef struct stack stack;
typedef struct stackControl stackControl;

struct stack {
	uiStack s;
	stackControl *controls;
	uintmax_t len;
	uintmax_t cap;
	int vertical;
	uiParent *parent;
	int padded;
	int userHid;
	int containerHid;
	int userDisabled;
	int containerDisabled;
};

struct stackControl {
	uiControl *c;
	int stretchy;
	intmax_t width;		// both used by resize(); preallocated to save time and reduce risk of failure
	intmax_t height;
};

static void stackDestroy(uiControl *c)
{
	stack *b = (stack *) c;
	uintmax_t i;

	if (b->parent != NULL)
		complain("attempt to destroy a uiControl at %p while it still has a parent %p", c, b->parent);
	for (i = 0; i < b->len; i++)
		uiControlDestroy(b->controls[i].c);
	uiFree(b->controls);
	uiFree(b);
}

static uintptr_t stackHandle(uiControl *c)
{
	return 0;
}

static void stackSetParent(uiControl *c, uiParent *parent)
{
	stack *b = (stack *) c;
	uintmax_t i;
	uiParent *oldparent;

	oldparent = b->parent;
	b->parent = parent;
	for (i = 0; i < b->len; i++)
		uiControlSetParent(b->controls[i].c, b->parent);
	if (oldparent != NULL)
		uiParentUpdate(oldparent);
	if (b->parent != NULL)
		uiParentUpdate(b->parent);
}

static void stackPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	stack *b = (stack *) c;
	int xpadding, ypadding;
	uintmax_t nStretchy;
	// these two contain the largest preferred width and height of all stretchy controls in the stack
	// all stretchy controls will use this value to determine the final preferred size
	intmax_t maxStretchyWidth, maxStretchyHeight;
	uintmax_t i;
	intmax_t preferredWidth, preferredHeight;

	*width = 0;
	*height = 0;
	if (b->len == 0)
		return;

	// 0) get this Stack's padding
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

static void stackResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	stack *b = (stack *) c;
	int xpadding, ypadding;
	uintmax_t nStretchy;
	intmax_t stretchywid, stretchyht;
	uintmax_t i;
	intmax_t preferredWidth, preferredHeight;

	if (b->len == 0)
		return;

	// -1) get this Stack's padding
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

static int stackVisible(uiControl *c)
{
	stack *b = (stack *) c;

	return !(b->userHid);
}

static void stackShow(uiControl *c)
{
	stack *b = (stack *) c;
	uintmax_t i;

	b->userHid = 0;
	if (!b->containerHid) {
		for (i = 0; i < b->len; i++)
			uiControlContainerShow(b->controls[i].c);
		if (b->parent != NULL)
			uiParentUpdate(b->parent);
	}
}

static void stackHide(uiControl *c)
{
	stack *b = (stack *) c;
	uintmax_t i;

	b->userHid = 1;
	for (i = 0; i < b->len; i++)
		uiControlContainerHide(b->controls[i].c);
	if (b->parent != NULL)
		uiParentUpdate(b->parent);
}

static void stackContainerShow(uiControl *c)
{
	stack *b = (stack *) c;
	uintmax_t i;

	b->containerHid = 0;
	if (!b->userHid) {
		for (i = 0; i < b->len; i++)
			uiControlContainerShow(b->controls[i].c);
		if (b->parent != NULL)
			uiParentUpdate(b->parent);
	}
}

static void stackContainerHide(uiControl *c)
{
	stack *b = (stack *) c;
	uintmax_t i;

	b->containerHid = 1;
	for (i = 0; i < b->len; i++)
		uiControlContainerHide(b->controls[i].c);
	if (b->parent != NULL)
		uiParentUpdate(b->parent);
}

static void stackEnable(uiControl *c)
{
	stack *b = (stack *) c;
	uintmax_t i;

	b->userDisabled = 0;
	if (!b->containerDisabled)
		for (i = 0; i < b->len; i++)
			uiControlContainerEnable(b->controls[i].c);
}

static void stackDisable(uiControl *c)
{
	stack *b = (stack *) c;
	uintmax_t i;

	b->userDisabled = 1;
	for (i = 0; i < b->len; i++)
		uiControlContainerDisable(b->controls[i].c);
}

static void stackContainerEnable(uiControl *c)
{
	stack *b = (stack *) c;
	uintmax_t i;

	b->containerDisabled = 0;
	if (!b->userDisabled)
		for (i = 0; i < b->len; i++)
			uiControlContainerEnable(b->controls[i].c);
}

static void stackContainerDisable(uiControl *c)
{
	stack *b = (stack *) c;
	uintmax_t i;

	b->containerDisabled = 1;
	for (i = 0; i < b->len; i++)
		uiControlContainerDisable(b->controls[i].c);
}

#define stackCapGrow 32

static void stackAppend(uiStack *ss, uiControl *c, int stretchy)
{
	stack *b = (stack *) ss;

	if (b->len >= b->cap) {
		b->cap += stackCapGrow;
		b->controls = (stackControl *) uiRealloc(b->controls, b->cap * sizeof (stackControl), "stackControl[]");
	}
	b->controls[b->len].c = c;
	b->controls[b->len].stretchy = stretchy;
	b->len++;		// must be here for parent updates to work
	if (b->parent != NULL) {
		uiControlSetParent(b->controls[b->len - 1].c, b->parent);
		uiParentUpdate(b->parent);
	}
}

static void stackDelete(uiStack *ss, uintmax_t index)
{
	stack *b = (stack *) ss;
	uiControl *removed;
	uintmax_t i;

	removed = b->controls[index].c;
	// TODO switch to memmove?
	for (i = index; i < b->len - 1; i++)
		b->controls[i] = b->controls[i + 1];
	// TODO memset the last one to NULL
	b->len--;
	if (b->parent != NULL) {
		uiControlSetParent(removed, NULL);
		uiParentUpdate(b->parent);
	}
}

static int stackPadded(uiStack *ss)
{
	stack *b = (stack *) ss;

	return b->padded;
}

static void stackSetPadded(uiStack *ss, int padded)
{
	stack *b = (stack *) ss;

	b->padded = padded;
	if (b->parent != NULL)
		uiParentUpdate(b->parent);
}

uiStack *uiNewHorizontalStack(void)
{
	stack *b;

	b = uiNew(stack);

	uiControl(b)->Destroy = stackDestroy;
	uiControl(b)->Handle = stackHandle;
	uiControl(b)->SetParent = stackSetParent;
	uiControl(b)->PreferredSize = stackPreferredSize;
	uiControl(b)->Resize = stackResize;
	uiControl(b)->Visible = stackVisible;
	uiControl(b)->Show = stackShow;
	uiControl(b)->Hide = stackHide;
	uiControl(b)->ContainerShow = stackContainerShow;
	uiControl(b)->ContainerHide = stackContainerHide;
	uiControl(b)->Enable = stackEnable;
	uiControl(b)->Disable = stackDisable;
	uiControl(b)->ContainerEnable = stackContainerEnable;
	uiControl(b)->ContainerDisable = stackContainerDisable;

	uiStack(b)->Append = stackAppend;
	uiStack(b)->Delete = stackDelete;
	uiStack(b)->Padded = stackPadded;
	uiStack(b)->SetPadded = stackSetPadded;

	return uiStack(b);
}

uiStack *uiNewVerticalStack(void)
{
	uiStack *ss;
	stack *b;

	ss = uiNewHorizontalStack();
	b = (stack *) ss;
	b->vertical = 1;
	return ss;
}
