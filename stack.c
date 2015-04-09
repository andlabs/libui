// 7 april 2015
#include "uipriv.h"

// TODO
// - use stackControl
// - change prefwid/prefht to preferredWidth/preferredHeight

typedef struct stack stack;
typedef struct stackControl stackControl;

struct stack {
	uiControl **controls;
	int *stretchy;
	intmax_t *width;
	intmax_t *height;
	uintmax_t len;
	uintmax_t cap;
	int vertical;
	uintptr_t parent;
	int padded;
};

struct stackControl {
	uiControl *control;
	int stretchy;
	intmax_t width;		// both used by resize(); preallocated to save time and reduce risk of failure
	intmax_t height;
};

static void stackDestroy(uiControl *c)
{
	stack *s = (stack *) (c->data);
	uintmax_t i;

	for (i = 0; i < s->len; i++)
		uiControlDestroy(s->controls[i]);
	uiFree(s->controls);
	uiFree(s->stretchy);
	uiFree(s->width);
	uiFree(s->height);
	uiFree(s);
	uiFree(c);
}

static uintptr_t stackHandle(uiControl *c)
{
	return 0;
}

static void stackSetParent(uiControl *c, uintptr_t parent)
{
	stack *s = (stack *) (c->data);
	uintmax_t i;

	s->parent = parent;
	for (i = 0; i < s->len; i++)
		uiControlSetParent(s->controls[i], s->parent);
	updateParent(s->parent);
}

static void stackRemoveParent(uiControl *c)
{
	stack *s = (stack *) (c->data);
	uintmax_t i;
	uintptr_t oldparent;

	oldparent = s->parent;
	s->parent = 0;
	for (i = 0; i < s->len; i++)
		uiControlRemoveParent(s->controls[i]);
	updateParent(oldparent);
}

static void stackPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	stack *s = (stack *) (c->data);
	int xpadding, ypadding;
	uintmax_t nStretchy;
	intmax_t maxswid, maxsht;
	uintmax_t i;
	intmax_t prefwid, prefht;

	*width = 0;
	*height = 0;
	if (s->len == 0)
		return;

	// 0) get this Stack's padding
	xpadding = 0;
	ypadding = 0;
	if (s->padded) {
		xpadding = d->xPadding;
		ypadding = d->yPadding;
	}

	// 1) initialize the desired rect with the needed padding
	if (s->vertical)
		*height = (s->len - 1) * ypadding;
	else
		*width = (s->len - 1) * xpadding;

	// 2) add in the size of non-stretchy controls and get (but not add in) the largest widths and heights of stretchy controls
	// we still add in like direction of stretchy controls
	nStretchy = 0;
	maxswid = 0;
	maxsht = 0;
	for (i = 0; i < s->len; i++) {
		uiControlPreferredSize(s->controls[i], d, &prefwid, &prefht);
		if (s->stretchy[i]) {
			nStretchy++;
			if (maxswid < prefwid)
				maxswid = prefwid;
			if (maxsht < prefht)
				maxsht = prefht;
		}
		if (s->vertical) {
			if (*width < prefwid)
				*width = prefwid;
			if (!s->stretchy[i])
				*height += prefht;
		} else {
			if (!s->stretchy[i])
				*width += prefwid;
			if (*height < prefht)
				*height = prefht;
		}
	}

	// 3) and now we can add in stretchy controls
	if (s->vertical)
		*height += nStretchy * maxsht;
	else
		*width += nStretchy * maxswid;
}

static void stackResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	stack *s = (stack *) (c->data);
	int xpadding, ypadding;
	uintmax_t nStretchy;
	intmax_t stretchywid, stretchyht;
	uintmax_t i;
	intmax_t prefwid, prefht;

	if (s->len == 0)
		return;

	// -1) get this Stack's padding
	xpadding = 0;
	ypadding = 0;
	if (s->padded) {
		xpadding = d->xPadding;
		ypadding = d->yPadding;
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
		uiControlPreferredSize(s->controls[i], d, &prefwid, &prefht);
		if (s->vertical) {		// all controls have same width
			s->width[i] = width;
			s->height[i] = prefht;
			stretchyht -= prefht;
		} else {				// all controls have same height
			s->width[i] = prefwid;
			s->height[i] = height;
			stretchywid -= prefwid;
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
		uiControlResize(s->controls[i], x, y, s->width[i], s->height[i], d);
		if (s->vertical)
			y += s->height[i] + ypadding;
		else
			x += s->width[i] + xpadding;
	}
}

uiControl *uiNewHorizontalStack(void)
{
	uiControl *c;
	stack *s;

	c = uiNew(uiControl);
	s = uiNew(stack);

	c->data = s;
	c->destroy = stackDestroy;
	c->handle = stackHandle;
	c->setParent = stackSetParent;
	c->removeParent = stackRemoveParent;
	c->preferredSize = stackPreferredSize;
	c->resize = stackResize;

	return c;
}

uiControl *uiNewVerticalStack(void)
{
	uiControl *c;
	stack *s;

	c = uiNewHorizontalStack();
	s = (stack *) (c->data);
	s->vertical = 1;
	return c;
}

#define stackCapGrow 32

void uiStackAdd(uiControl *st, uiControl *c, int stretchy)
{
	stack *s = (stack *) (st->data);

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
		uiControlSetParent(s->controls[s->len], s->parent);
	s->len++;
	updateParent(s->parent);
}

// TODO get padded

void uiStackSetPadded(uiControl *c, int padded)
{
	stack *s = (stack *) (c->data);

	s->padded = padded;
	updateParent(s->parent);
}
