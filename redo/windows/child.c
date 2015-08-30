// 28 august 2015
#include "uipriv_windows.h"

// This file contains helpers for managing child controls.

struct child {
	uiControl *c;
	HWND hwnd;

	HWND parent;

	// This flag is for users of these functions.
	// For uiBox, this is "spaced".
	// For uiTab, this is "margined". (uiGroup and uiWindow have to maintain their margined state themselves, since the margined state is independent of whether there is a child for those two.)
	int flag;
};

struct child *newChild(uiControl *child, uiControl *parent, HWND parentHWND)
{
	struct child *c;

	if (child == NULL)
		return NULL;

	c = uiNew(struct child);
	c->c = child;
	c->hwnd = (HWND) uiControlHandle(c->c);

	uiControlSetParent(c->c, parent);
	uiWindowsEnsureSetParent(c->hwnd, parentHWND);
	c->parent = parentHWND;

	return c;
}

void childRemove(struct child *c)
{
	uiWindowsEnsureSetParent(c->hwnd, utilwin);
	uiControlSetParent(c->c, NULL);
	uiFree(c);
}

void childDestroy(struct child *c)
{
	uiControl *child;

	child = c->c;
	childRemove(c);
	uiControlDestroy(child);
}

HWND childHWND(struct child *c)
{
	return c->hwnd;
}

void childMinimumSize(struct child *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	uiWindowsControl *wc;

	wc = uiWindowsControl(c->c);
	(*(wc->MinimumSize))(wc, d, width, height);
}

void childRelayout(struct child *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
	uiWindowsControl *wc;

	wc = uiWindowsControl(c->c);
	(*(wc->Relayout))(wc, x, y, width, height);
}

void childUpdateState(struct child *c)
{
	controlUpdateState(c->c);
}

int childFlag(struct child *c)
{
	return c->flag;
}

void childSetFlag(struct child *c, int flag)
{
	c->flag = flag;
}
