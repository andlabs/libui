// 7 april 2015
#include "uipriv.h"

typedef struct stack stack;

struct stack {
	uiControl control;
	uiControl **controls;
	int *stretchy;
	intmax_t *width;		// both used by resize(); preallocated to save time and reduce risk of failure
	intmax_t *height;
	uintmax_t len;
	uintmax_t cap;
	int vertical;
	uintptr_t parent;
	int padded;
};

#define S(c) ((stack *) (c))

static void stackDestroy(uiControl *c)
{
	stack *s = (stack *) c;
	uintmax_t i;

	for (i = 0; i < S(c)->len; i++)
		uiControlDestroy(s->controls[i]);
	uiFree(s->controls);
	uiFree(s->stretchy);
	uiFree(s->width);
	uiFree(s->height);
	uiFree(s);
}

static uintptr_t stackHandle(uiControl *c)
{
	return 0;
}

static void stackSetParent(uiControl *c, uintptr_t parent)
{
	stack *s = S(c);
	uintmax_t i;

	s->parent = parent;
	for (i = 0; i < S(c)->len; i++)
		(*(s->controls[i]->setParent))(s->controls[i], s->parent);
	updateParent(s->parent);
}

static void stackRemoveParent(uiControl *c)
{
	stack *s = S(c);
	uintmax_t i;
	uintptr_t oldparent;

	oldparent = s->parent;
	s->parent = 0;
	for (i = 0; i < S(c)->len; i++)
		(*(s->controls[i]->removeParent))(s->controls[i]);
	updateParent(oldparent);
}

static uiSize stackPreferredSize(uiControl *c, uiSizing *d)
{
	stack *s = S(c);
	int xpadding, ypadding;
	uintmax_t nStretchy;
	intmax_t maxswid, maxsht;
	uintmax_t i;
	uiSize size, preferred;
	uiSizingComm *dd = (uiSizingComm *) d;

	size.width = 0;
	size.height = 0;
	if (s->len == 0)
		return size;

	// 0) get this Stack's padding
	xpadding = 0;
	ypadding = 0;
	if (s->padded) {
		xpadding = dd->xPadding;
		ypadding = dd->yPadding;
	}

	// 1) initialize the desired rect with the needed padding
	if (s->vertical)
		size.height = (s->len - 1) * ypadding;
	else
		size.width = (s->len - 1) * xpadding;

	// 2) add in the size of non-stretchy controls and get (but not add in) the largest widths and heights of stretchy controls
	// we still add in like direction of stretchy controls
	nStretchy = 0;
	maxswid = 0;
	maxsht = 0;
	for (i = 0; i < s->len; i++) {
		preferred = (*(s->controls[i]->preferredSize))(s->controls[i], d);
		if (s->stretchy[i]) {
			nStretchy++;
			if (maxswid < preferred.width)
				maxswid = preferred.width;
			if (maxsht < preferred.height)
				maxsht = preferred.height;
		}
		if (s->vertical) {
			if (size.width < preferred.width)
				size.width = preferred.width;
			if (!s->stretchy[i])
				size.height += preferred.height;
		} else {
			if (!s->stretchy[i])
				size.width += preferred.width;
			if (size.height < preferred.height)
				size.height = preferred.height;
		}
	}

	// 3) and now we can add in stretchy controls
	if (s->vertical)
		size.height += nStretchy * maxsht;
	else
		size.width += nStretchy * maxswid;

	return size;
}

static void stackResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	stack *s = S(c);
	int xpadding, ypadding;
	uintmax_t nStretchy;
	intmax_t stretchywid, stretchyht;
	uintmax_t i;
	uiSize preferred;
	uiSizingComm *dd = (uiSizingComm *) d;

	if (s->len == 0)
		return;

	// -1) get this Stack's padding
	xpadding = 0;
	ypadding = 0;
	if (s->padded) {
		xpadding = dd->xPadding;
		ypadding = dd->yPadding;
	}

	// 0) inset the available rect by the needed padding
	if (s->vertical)
		height -= (s->len - 1) * ypadding;
	else
		width -= (s->len - 1) * xpadding;

	// 1) get width and height of non-stretchy controls
	// this will tell us how much space will be left for stretchy controls
	stretchywid = width;
	stretchyht = height;
	nStretchy = 0;
	for (i = 0; i < s->len; i++) {
		if (s->stretchy[i]) {
			nStretchy++;
			continue;
		}
		c = s->controls[i];
		preferred = (*(s->controls[i]->preferredSize))(s->controls[i], d);
		if (s->vertical) {		// all controls have same width
			s->width[i] = width;
			s->height[i] = preferred.height;
			stretchyht -= preferred.height;
		} else {				// all controls have same height
			s->width[i] = preferred.width;
			s->height[i] = height;
			stretchywid -= preferred.width;
		}
	}

	// 2) now get the size of stretchy controls
	if (nStretchy != 0)
		if (s->vertical)
			stretchyht /= nStretchy;
		else
			stretchywid /= nStretchy;
	for (i = 0; i < s->len; i++)
		if (s->stretchy[i]) {
			s->width[i] = stretchywid;
			s->height[i] = stretchyht;
		}

	// 3) now we can position controls
	for (i = 0; i < s->len; i++) {
		(*(s->controls[i]->resize))(s->controls[i], x, y, s->width[i], s->height[i], d);
		if (s->vertical)
			y += s->height[i] + ypadding;
		else
			x += s->width[i] + xpadding;
	}
}

uiControl *uiNewHorizontalStack(void)
{
	stack *s;

	s = uiNew(stack);

	s->control.destroy = stackDestroy;
	s->control.handle = stackHandle;
	s->control.setParent = stackSetParent;
	s->control.removeParent = stackRemoveParent;
	s->control.preferredSize = stackPreferredSize;
	s->control.resize = stackResize;

	return (uiControl *) s;
}

uiControl *uiNewVerticalStack(void)
{
	uiControl *c;

	c = uiNewHorizontalStack();
	S(c)->vertical = 1;
	return c;
}

#define stackCapGrow 32

void uiStackAdd(uiControl *st, uiControl *c, int stretchy)
{
	stack *s = S(st);

	if (s->len >= s->cap) {
		s->cap += stackCapGrow;
		s->controls = (uiControl **) uiRealloc(s->controls, s->cap * sizeof (uiControl *), "uiControl *[]");
		s->stretchy = (int *) uiRealloc(s->stretchy, s->cap * sizeof (int), "int[]");
		s->width = (intmax_t *) uiRealloc(s->width, s->cap * sizeof (intmax_t), "intmax_t[]");
		s->height = (intmax_t *) uiRealloc(s->height, s->cap * sizeof (intmax_t), "intmax_t[]");
	}
	s->controls[s->len] = c;
	s->stretchy[s->len] = stretchy;
	if (s->parent != 0)
		(*(s->controls[s->len]->setParent))(s->controls[s->len], s->parent);
	s->len++;
	updateParent(s->parent);
}

// TODO get padded

void uiStackSetPadded(uiControl *st, int padded)
{
	stack *s = S(st);

	s->padded = padded;
	updateParent(s->parent);
}
