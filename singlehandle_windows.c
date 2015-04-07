// 6 april 2015
#include "tablepriv.h"

// Common code for controls with a single window handle.
// The only method NOT defined is preferredSize(); this differs between controls.

static uintptr_t singleHandle(uiControl *c)
{
	return (uintptr_t) (c->hwnd);
}

//TODO	void (*setParent)(uiControl *, uintptr_t);

static void singleResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	if (MoveWindow(c->hwnd, x, y, width, height, TRUE) == 0)
		logLastError("error moving control in singleResize()");
}

static void singleContainerShow(uiControl *c)
{
	ShowWindow(c->hwnd, SW_SHOW);
}

static void singleContainerHide(uiControl *c)
{
	ShowWindow(c->hwnd, SW_HIDE);
}

uiSingleHWNDControl *newSingleHWNDControl(DWORD exstyle, const WCHAR *class, DWORD style, HWND parent, HINSTANCE hInstance)
{
	uiSingleHWNDControl *c;

	c = uiNew(uiSingleHWNDControl);
	c->hwnd = CreateWindowExW(exstyle,
		class, L"",
		style | WS_CHILD | WS_VISIBLE,
		0, 0,
		100, 100,
		// TODO specify control IDs properly
		parent, NULL, hInstance, NULL);
	if (c->hwnd == NULL)
		logLastError("error creating control in newSingleHWNDControl()");

	c->control.handle = singleHandle;
//TODO	c->control.setParent = singleSetParent;
	c->control.resize = singleResize;
	c->control.containerShow = singleContainerShow;
	c->control.containerHide = singleContainerHide;

	return c;
}
