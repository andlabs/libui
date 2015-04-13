// 6 april 2015
#include "uipriv_windows.h"

typedef struct singleHWND singleHWND;

struct singleHWND {
	HWND hwnd;
	BOOL (*onWM_COMMAND)(uiControl *, WORD, LRESULT *);
	BOOL (*onWM_NOTIFY)(uiControl *, NMHDR *, LRESULT *);
	void (*onWM_DESTROY)(uiControl *);
	uiParent *parent;
	BOOL userHid;
	BOOL containerHid;
	BOOL userDisabled;
	BOOL containerDisabled;
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

static void singleSetParent(uiControl *c, uiParent *parent)
{
	singleHWND *s = (singleHWND *) (c->internal);

	s->parent = parent;
	if (SetParent(s->hwnd, (HWND) (s->parent)) == NULL)
		logLastError("error setting control parent in singleSetParent()");
	uiParentUpdate(s->parent);
}

static void singleRemoveParent(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->internal);
	uiParent *oldparent;

	oldparent = s->parent;
	s->parent = NULL;
	if (SetParent(s->hwnd, initialParent) == NULL)
		logLastError("error removing control parent in singleSetParent()");
	uiParentUpdate(oldparent);
}

static void singleResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	singleHWND *s = (singleHWND *) (c->internal);

	if (MoveWindow(s->hwnd, x, y, width, height, TRUE) == 0)
		logLastError("error moving control in singleResize()");
}

static int singleVisible(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->internal);

	if (s->userHid)
		return 0;
	return 1;
}

static void singleShow(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->internal);

	s->userHid = FALSE;
	if (!s->containerHid) {
		ShowWindow(s->hwnd, SW_SHOW);
		if (s->parent != NULL)
			uiParentUpdate(s->parent);
	}
}

static void singleHide(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->internal);

	s->userHid = TRUE;
	ShowWindow(s->hwnd, SW_HIDE);
	if (s->parent != NULL)
		uiParentUpdate(s->parent);
}

static void singleContainerShow(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->internal);

	s->containerHid = FALSE;
	if (!s->userHid) {
		ShowWindow(s->hwnd, SW_SHOW);
		if (s->parent != NULL)
			uiParentUpdate(s->parent);
	}
}

static void singleContainerHide(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->internal);

	s->containerHid = TRUE;
	ShowWindow(s->hwnd, SW_HIDE);
	if (s->parent != NULL)
		uiParentUpdate(s->parent);
}

static void singleEnable(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->internal);

	s->userDisabled = FALSE;
	if (!s->containerDisabled)
		EnableWindow(s->hwnd, TRUE);
}

static void singleDisable(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->internal);

	s->userDisabled = TRUE;
	EnableWindow(s->hwnd, FALSE);
}

static void singleContainerEnable(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->internal);

	s->containerDisabled = FALSE;
	if (!s->userDisabled)
		EnableWindow(s->hwnd, TRUE);
}

static void singleContainerDisable(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->internal);

	s->containerDisabled = TRUE;
	EnableWindow(s->hwnd, FALSE);
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
		if ((*(s->onWM_NOTIFY))(c, (NMHDR *) lParam, &lResult) != FALSE)
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
		// use a nonzero initial size just in case some control breaks with a zero initial size
		100, 100,
		initialParent, NULL, p->hInstance, NULL);
	if (s->hwnd == NULL)
		logLastError("error creating control in uiWindowsNewControl()");
	s->onWM_COMMAND = p->onWM_COMMAND;
	s->onWM_NOTIFY = p->onWM_NOTIFY;
	s->onWM_DESTROY = p->onWM_DESTROY;

	c = uiNew(uiControl);
	c->destroy = singleDestroy;
	c->handle = singleHandle;
	c->setParent = singleSetParent;
	c->removeParent = singleRemoveParent;
	c->resize = singleResize;
	c->visible = singleVisible;
	c->show = singleShow;
	c->hide = singleHide;
	c->containerShow = singleContainerShow;
	c->containerHide = singleContainerHide;
	c->enable = singleEnable;
	c->disable = singleDisable;
	c->containerEnable = singleContainerEnable;
	c->containerDisable = singleContainerDisable;

	if (p->useStandardControlFont)
		SendMessageW(s->hwnd, WM_SETFONT, (WPARAM) hMessageFont, (LPARAM) TRUE);

	if ((*fv_SetWindowSubclass)(s->hwnd, singleSubclassProc, 0, (DWORD_PTR) c) == FALSE)
		logLastError("error subclassing Windows control in uiWindowsNewControl()");

	c->internal = s;
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
