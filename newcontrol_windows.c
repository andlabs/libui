// 6 april 2015
#include "uipriv_windows.h"

typedef struct uiSingleHWNDControl uiSingleHWNDControl;

struct uiSingleHWNDControl {
	uiControl control;
	HWND hwnd;
	BOOL (*onWM_COMMAND)(uiControl *, WPARAM, LPARAM, void *, LRESULT *);
	BOOL (*onWM_NOTIFY)(uiControl *, WPARAM, LPARAM, void *, LRESULT *);
	void *onCommandNotifyData;
	void (*preferredSize)(uiControl *, int, int, LONG, intmax_t *, intmax_t *);
};

#define S(c) ((uiSingleHWNDControl *) (c))

static uintptr_t singleHandle(uiControl *c)
{
	return (uintptr_t) (S(c)->hwnd);
}

void singleSetParent(uiControl *c, uintptr_t parentHWND)
{
	if (SetParent(S(c)->hwnd, (HWND) parentHWND) == NULL)
		logLastError("error changing control parent in singleSetParent()");
}

uiSize singlePreferredSize(uiControl *c, uiSizing *d)
{
	uiSize size;

	(*(S(c)->preferredSize))(c,
		d->baseX, d->baseY, d->internalLeading,
		&(size.width), &(size.height));
	return size;
}

static void singleResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	if (MoveWindow(S(c)->hwnd, x, y, width, height, TRUE) == 0)
		logLastError("error moving control in singleResize()");
}

static void singleContainerShow(uiControl *c)
{
	ShowWindow(S(c)->hwnd, SW_SHOW);
}

static void singleContainerHide(uiControl *c)
{
	ShowWindow(S(c)->hwnd, SW_HIDE);
}

uiControl *uiWindowsNewControl(uiWindowsNewControlParams *p)
{
	uiSingleHWNDControl *c;

	c = uiNew(uiSingleHWNDControl);
	c->hwnd = CreateWindowExW(p->dwExStyle,
		p->lpClassName, L"",
		p->dwStyle | WS_CHILD | WS_VISIBLE,
		0, 0,
		100, 100,
		// TODO specify control IDs properly
		initialParent, NULL, p->hInstance, NULL);
	if (c->hwnd == NULL)
		logLastError("error creating control in newSingleHWNDControl()");

	c->control.handle = singleHandle;
	c->control.setParent = singleSetParent;
	c->control.preferredSize = singlePreferredSize;
	c->control.resize = singleResize;
	c->control.containerShow = singleContainerShow;
	c->control.containerHide = singleContainerHide;

	c->onWM_COMMAND = p->onWM_COMMAND;
	c->onWM_NOTIFY = p->onWM_NOTIFY;
	c->onCommandNotifyData = p->onCommandNotifyData;
	c->preferredSize = p->preferredSize;

	// TODO subclass

	return (uiControl *) c;
}
