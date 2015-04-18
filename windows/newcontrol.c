// 6 april 2015
#include "uipriv_windows.h"

typedef struct singleHWND singleHWND;

struct singleHWND {
	HWND hwnd;
	BOOL (*onWM_COMMAND)(uiControl *, WORD, LRESULT *);
	BOOL (*onWM_NOTIFY)(uiControl *, NMHDR *, LRESULT *);
	void (*onDestroy)(void *);
	void *onDestroyData;
	uiParent *parent;
	BOOL userHid;
	BOOL containerHid;
	BOOL userDisabled;
	BOOL containerDisabled;
};

static void singleDestroy(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->Internal);

	SendMessageW(s->hwnd, msgCanDestroyNow, 0, 0);
	(*(s->onDestroy))(s->onDestroyData);
	if (DestroyWindow(s->hwnd) == 0)
		logLastError("error destroying control in singleDestroy()");
	uiFree(s);
}

static uintptr_t singleHandle(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->Internal);

	return (uintptr_t) (s->hwnd);
}

static void singleSetParent(uiControl *c, uiParent *parent)
{
	singleHWND *s = (singleHWND *) (c->Internal);
	uiParent *oldparent;
	HWND newParentHWND;

	oldparent = s->parent;
	s->parent = parent;
	newParentHWND = initialParent;
	if (s->parent != NULL)
		newParentHWND = uiParentHWND(s->parent);
	if (SetParent(s->hwnd, newParentHWND) == NULL)
		logLastError("error setting control parent in singleSetParent()");
	if (oldparent != NULL)
		uiParentUpdate(oldparent);
	if (s->parent != NULL)
		uiParentUpdate(s->parent);
}

static void singleResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	singleHWND *s = (singleHWND *) (c->Internal);

	if (MoveWindow(s->hwnd, x, y, width, height, TRUE) == 0)
		logLastError("error moving control in singleResize()");
}

static int singleVisible(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->Internal);

	if (s->userHid)
		return 0;
	return 1;
}

static void singleShow(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->Internal);

	s->userHid = FALSE;
	if (!s->containerHid) {
		ShowWindow(s->hwnd, SW_SHOW);
		if (s->parent != NULL)
			uiParentUpdate(s->parent);
	}
}

static void singleHide(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->Internal);

	s->userHid = TRUE;
	ShowWindow(s->hwnd, SW_HIDE);
	if (s->parent != NULL)
		uiParentUpdate(s->parent);
}

static void singleContainerShow(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->Internal);

	s->containerHid = FALSE;
	if (!s->userHid) {
		ShowWindow(s->hwnd, SW_SHOW);
		if (s->parent != NULL)
			uiParentUpdate(s->parent);
	}
}

static void singleContainerHide(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->Internal);

	s->containerHid = TRUE;
	ShowWindow(s->hwnd, SW_HIDE);
	if (s->parent != NULL)
		uiParentUpdate(s->parent);
}

static void singleEnable(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->Internal);

	s->userDisabled = FALSE;
	if (!s->containerDisabled)
		EnableWindow(s->hwnd, TRUE);
}

static void singleDisable(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->Internal);

	s->userDisabled = TRUE;
	EnableWindow(s->hwnd, FALSE);
}

static void singleContainerEnable(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->Internal);

	s->containerDisabled = FALSE;
	if (!s->userDisabled)
		EnableWindow(s->hwnd, TRUE);
}

static void singleContainerDisable(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->Internal);

	s->containerDisabled = TRUE;
	EnableWindow(s->hwnd, FALSE);
}

static LRESULT CALLBACK singleSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	uiControl *c = (uiControl *) dwRefData;
	singleHWND *s = (singleHWND *) (c->Internal);
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
	case WM_NCDESTROY:		// just in case
		complain("trying to destroy control at %p before uiControlDestroy()", c);
		break;
	case msgCanDestroyNow:
		if ((*fv_RemoveWindowSubclass)(hwnd, singleSubclassProc, uIdSubclass) == FALSE)
			logLastError("error removing Windows control subclass in singleSubclassProc()");
		break;
	}
	return (*fv_DefSubclassProc)(hwnd, uMsg, wParam, lParam);
}

void uiWindowsNewControl(uiControl *c, uiWindowsNewControlParams *p)
{
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

	s->onDestroy = p->onDestroy;
	s->onDestroyData = p->onDestroyData;

	c->Destroy = singleDestroy;
	c->Handle = singleHandle;
	c->SetParent = singleSetParent;
	c->Resize = singleResize;
	c->Visible = singleVisible;
	c->Show = singleShow;
	c->Hide = singleHide;
	c->ContainerShow = singleContainerShow;
	c->ContainerHide = singleContainerHide;
	c->Enable = singleEnable;
	c->Disable = singleDisable;
	c->ContainerEnable = singleContainerEnable;
	c->ContainerDisable = singleContainerDisable;

	if (p->useStandardControlFont)
		SendMessageW(s->hwnd, WM_SETFONT, (WPARAM) hMessageFont, (LPARAM) TRUE);

	// this handles both blocking control destruction and redirected notification messages
	if ((*fv_SetWindowSubclass)(s->hwnd, singleSubclassProc, 0, (DWORD_PTR) c) == FALSE)
		logLastError("error subclassing Windows control in uiWindowsNewControl()");

	c->Internal = s;
}

char *uiWindowsControlText(uiControl *c)
{
	singleHWND *s = (singleHWND *) (c->Internal);
	WCHAR *wtext;
	char *text;

	wtext = windowText(s->hwnd);
	text = toUTF8(wtext);
	uiFree(wtext);
	return text;
}

void uiWindowsControlSetText(uiControl *c, const char *text)
{
	singleHWND *s = (singleHWND *) (c->Internal);
	WCHAR *wtext;

	wtext = toUTF16(text);
	if (SetWindowTextW(s->hwnd, wtext) == 0)
		logLastError("error setting control text in uiWindowsControlSetText()");
	uiFree(wtext);
}
