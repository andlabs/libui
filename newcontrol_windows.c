// 6 april 2015
#include "uipriv_windows.h"

typedef struct singleHWND singleHWND;

struct singleHWND {
	HWND hwnd;
	BOOL (*onWM_COMMAND)(uiControl *, WORD, LRESULT *);
	BOOL (*onWM_NOTIFY)(uiControl *, WPARAM, LPARAM, LRESULT *);
	void (*onWM_DESTROY)(uiControl *);
	uintptr_t parent;
};

static void singleDestroy(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->internal);

	if (DestroyWindow(s->hwnd) == 0)
		logLastError("error destroying control in singleDestroy()");
	// the data structures are destroyed in the subclass procedure
}

static uintptr_t singleHandle(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->internal);

	return (uintptr_t) (s->hwnd);
}

static void singleSetParent(uiControl *c, uintptr_t parent)
{
	singleHWND *s = (singleHWND *) (c->internal);

	s->parent = parent;
	if (SetParent(s->hwnd, (HWND) (s->parent)) == NULL)
		logLastError("error setting control parent in singleSetParent()");
	updateParent(s->parent);
}

static void singleRemoveParent(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->internal);
	uintptr_t oldparent;

	oldparent = s->parent;
	s->parent = 0;
	if (SetParent(s->hwnd, initialParent) == NULL)
		logLastError("error removing control parent in singleSetParent()");
	updateParent(oldparent);
}

static void singleResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	singleHWND *s = (singleHWND *) (c->internal);

	if (MoveWindow(s->hwnd, x, y, width, height, TRUE) == 0)
		logLastError("error moving control in singleResize()");
}

static LRESULT CALLBACK singleSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	uiControl *c = (uiControl *) dwRefData;
	singleHWND *s = (singleHWND *) (c->internal);
	LRESULT lResult;

	switch (uMsg) {
	case msgCOMMAND:
		if ((*(s->onWM_COMMAND))(c, HIWORD(wParam), &lResult) != FALSE)
			return lResult;
		break;
	case msgNOTIFY:
		if ((*(s->onWM_NOTIFY))(c, wParam, lParam, &lResult) != FALSE)
			return lResult;
		break;
	case WM_DESTROY:
		(*(s->onWM_DESTROY))(c);
		uiFree(s);
		uiFree(c);
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
	uiControl *c;
	singleHWND *s;

	s = uiNew(singleHWND);
	s->hwnd = CreateWindowExW(p->dwExStyle,
		p->lpClassName, p->lpWindowName,
		p->dwStyle | WS_CHILD | WS_VISIBLE,
		0, 0,
		100, 100,
		// TODO specify control IDs properly
		initialParent, NULL, p->hInstance, NULL);
	if (s->hwnd == NULL)
		logLastError("error creating control in uiWindowsNewControl()");
	s->onWM_COMMAND = p->onWM_COMMAND;
	s->onWM_NOTIFY = p->onWM_NOTIFY;
	s->onWM_DESTROY = p->onWM_DESTROY;

	c = uiNew(uiControl);
	c->internal = s;
	c->destroy = singleDestroy;
	c->handle = singleHandle;
	c->setParent = singleSetParent;
	c->removeParent = singleRemoveParent;
	c->resize = singleResize;

	if ((*fv_SetWindowSubclass)(s->hwnd, singleSubclassProc, 0, (DWORD_PTR) c) == FALSE)
		logLastError("error subclassing Windows control in uiWindowsNewControl()");

	return c;
}

char *uiWindowsControlText(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->internal);
	WCHAR *wtext;
	char *text;

	wtext = windowText(s->hwnd);
	text = toUTF8(wtext);
	uiFree(wtext);
	return text;
}

void uiWindowsControlSetText(uiControl *c, const char *text)
{
	singleHWND *s = (singleHWND *) (c->internal);
	WCHAR *wtext;

	wtext = toUTF16(text);
	if (SetWindowTextW(s->hwnd, wtext) == 0)
		logLastError("error setting control text in uiWindowsControlSetText()");
	uiFree(wtext);
}
