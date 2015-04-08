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
	void *data;
};

#define S(c) ((uiSingleHWNDControl *) (c))

static uintptr_t singleHandle(uiControl *c)
{
	return (uintptr_t) (S(c)->hwnd);
}

static void singleSetParent(uiControl *c, uintptr_t parentHWND)
{
	if (SetParent(S(c)->hwnd, (HWND) parentHWND) == NULL)
		logLastError("error changing control parent in singleSetParent()");
}

static uiSize singlePreferredSize(uiControl *c, uiSizing *d)
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

static LRESULT CALLBACK singleSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	uiSingleHWNDControl *c = (uiSingleHWNDControl *) dwRefData;
	LRESULT lResult;

	switch (uMsg) {
	case msgCOMMAND:
		if ((*(c->onWM_COMMAND))((uiControl *) c, wParam, lParam, c->onCommandNotifyData, &lResult) != FALSE)
			return lResult;
		break;
	case msgNOTIFY:
		if ((*(c->onWM_NOTIFY))((uiControl *) c, wParam, lParam, c->onCommandNotifyData, &lResult) != FALSE)
			return lResult;
		break;
	case WM_NCDESTROY:
		if ((*fv_RemoveWindowSubclass)(hwnd, singleSubclassProc, uIdSubclass) == FALSE)
			logLastError("error removing Windows control subclass in singleSubclassProc()");
		break;
	}
	return (*fv_DefSubclassProc)(hwnd, uMsg, wParam, lParam);
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
		logLastError("error creating control in uiWindowsNewControl()");

	c->control.handle = singleHandle;
	c->control.setParent = singleSetParent;
	c->control.preferredSize = singlePreferredSize;
	c->control.resize = singleResize;

	c->onWM_COMMAND = p->onWM_COMMAND;
	c->onWM_NOTIFY = p->onWM_NOTIFY;
	c->onCommandNotifyData = p->onCommandNotifyData;
	c->preferredSize = p->preferredSize;

	c->data = p->data;

	if ((*fv_SetWindowSubclass)(c->hwnd, singleSubclassProc, 0, (DWORD_PTR) c) == FALSE)
		logLastError("error subclassing Windows control in uiWindowsNewControl()");

	return (uiControl *) c;
}

void *uiWindowsControlData(uiControl *c)
{
	return S(c)->data;
}
