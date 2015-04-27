// 27 april 2015
#include "uipriv_windows.h"

struct window {
	uiWindow w;
	HWND hwnd;
	uiContainer *bin;
	int hidden;
	int margined;
};

// TODO window class and init functions

static void windowDestroy(uiControl *c)
{
	struct window *w = (struct window *) c;

	// first hide ourselves
	ShowWindow(w->hwnd, SW_HIDE);
	// now destroy the bin
	// the bin has no parent, so we can just call uiControlDestroy()
	uiControlDestroy(uiControl(w->bin));
	// TODO menus
	// now destroy ourselves
	if (DestroyWindow(w->hwnd) == 0)
		logLastError("error destroying uiWindow in windowDestroy()");
	uiFree(w);
}

static uintptr_t windowHandle(uiControl *c)
{
	struct window *w = (struct window *) c;

	return (uintptr_t) (w->hwnd);
}

static void windowSetParent(uiControl *c)
{
	complain("attempt to give the uiWindow at %p a parent", c);
}

static void windowPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	complain("attempt to get the preferred size of the uiWindow at %p", c);
}

static void windowResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	complain("attempt to resize the uiWindow at %p", c);
}

static int windowVisible(uiControl *c)
{
	struct window *w = (struct window *) c;

	return !w->hidden;
}

static void windowShow(uiControl *w)
{
	struct window *w = (struct window *) c;

	// TODO first show logic
	ShowWindow(w->hwnd, SW_SHOW);
	w->hidden = 0;
}

static void windowHide(uiControl *w)
{
	struct window *w = (struct window *) c;

	ShowWindow(w->hwnd, SW_HIDE);
	w->hidden = 1;
}

static void windowEnable(uiControl *c)
{
	struct window *w = (struct window *) c;

	EnableWindow(w->hwnd, TRUE);
}

static void windowDisable(uiControl *c)
{
	struct window *w = (struct window *) c;

	EnableWindow(w->hwnd, FALSE);
}
