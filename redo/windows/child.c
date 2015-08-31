// 28 august 2015
#include "uipriv_windows.h"

// This file contains helpers for managing child controls.

struct child {
	uiControl *c;
	HWND hwnd;

	// This is a helper for uiTab pages.
	// For visual accuracy of tab page backgrounds, margins are also handled here, applied to the child only (rather than applied to the whole tab page).
	HWND tabpage;
	int margined;

	// This flag is for users of these functions.
	// For uiBox, this is "spaced".
	int flag;

	// These intmax_t variables are for users of these functions.
	// For uiBox, these are "width" and "height".
	intmax_t im[2];
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

	return c;
}

struct child *newChildWithTabPage(uiControl *child, uiControl *parent, HWND parentHWND)
{
	struct child *c;
	HWND tabpage;

	tabpage = newTabPage();
	c = newChild(child, parent, tabpage);
	uiWindowsEnsureSetParent(tabpage, parentHWND);
	return c;
}

void childRemove(struct child *c)
{
	uiWindowsEnsureSetParent(c->hwnd, utilwin);
	uiControlSetParent(c->c, NULL);
	if (c->tabpage != NULL)
		uiWindowsEnsureDestroyWindow(c->tabpage);
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
	intmax_t left, top, right, bottom

	wc = uiWindowsControl(c->c);
	(*(wc->MinimumSize))(wc, d, width, height);
	if (c->tabpage != NULL && c->margined) {
		tabPageMargins(c->tabpage, &left, &top, &right, &bottom);
		*width += left + right;
		*height += top + bottom;
	}
}

void childRelayout(struct child *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
	uiWindowsControl *wc;
	intmax_t left, top, right, bottom;

	if (c->tabpage != NULL) {
		uiWindowsEnsureMoveWindow(c->tabpage, x, y, width, height);
		x = 0;		// and make relative to the client rect of the tab page
		y = 0;
		if (c->margined) {
			tabPageMargins(c->tabpage, &left, &top, &right, &bottom);
			x += left;
			y += top;
			width -= left + right;
			height -= top + bottom;
		}
	}
	wc = uiWindowsControl(c->c);
	(*(wc->Relayout))(wc, x, y, width, height);
}

void childUpdateState(struct child *c)
{
	controlUpdateState(c->c);
}

HWND childTabPage(struct child *c)
{
	return c->tabpage;
}

int childMargined(struct child *c)
{
	return c->margined;
}

void childSetMargined(struct child *c)
{
	c->margined = margined;
	uiControlQueueResize(c->c);
}

int childFlag(struct child *c)
{
	return c->flag;
}

void childSetFlag(struct child *c, int flag)
{
	c->flag = flag;
}

intmax_t childIntmax(struct child *c, int n)
{
	return c->im[n];
}

void childSetIntmax(struct child *c, int n, intmax_t to)
{
	c->im[n] = to;
}
