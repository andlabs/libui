// 7 april 2015
#include "uipriv_windows.h"

struct button {
	void (*onClicked)(uiControl *, void *);
	void *onClickedData;
};

static BOOL onWM_COMMAND(uiControl *c, WORD code, LRESULT *lResult)
{
	struct button *b = (struct button *) (c->data);

	if (code != BN_CLICKED)
		return FALSE;
	(*(b->onClicked))(c, b->onClickedData);
	*lResult = 0;
	return TRUE;
}

static BOOL onWM_NOTIFY(uiControl *c, WPARAM wParam, LPARAM lParam, LRESULT *lResult)
{
	return FALSE;
}

static void onWM_DESTROY(uiControl *c)
{
	struct button *b = (struct button *) (c->data);

	uiFree(b);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define buttonHeight 14

static void preferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	HWND hwnd;
	SIZE size;

	hwnd = (HWND) uiControlHandle(c);

	// try the comctl32 version 6 way
	size.cx = 0;		// explicitly ask for ideal size
	size.cy = 0;
	if (SendMessageW(hwnd, BCM_GETIDEALSIZE, 0, (LPARAM) (&size)) != FALSE) {
		*width = size.cx;
		*height = size.cy;
		return;
	}

	// that didn't work; fall back to using Microsoft's metrics
	// Microsoft says to use a fixed width for all buttons; this isn't good enough
	// use the text width instead, with some edge padding
	*width = uiWindowsWindowTextWidth(hwnd) + (2 * GetSystemMetrics(SM_CXEDGE));
	*height = uiDlgUnitsToY(buttonHeight, d->sys->baseY);
}

static void defaultOnClicked(uiControl *c, void *data)
{
	// do nothing
}

uiControl *uiNewButton(const char *text)
{
	uiControl *c;
	struct button *b;
	uiWindowsNewControlParams p;
	WCHAR *wtext;
	HWND hwnd;

	p.dwExStyle = 0;
	p.lpClassName = L"button";
	wtext = toUTF16(text);
	p.lpWindowName = wtext;
	p.dwStyle = BS_PUSHBUTTON | WS_TABSTOP;
	p.hInstance = hInstance;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onWM_DESTROY = onWM_DESTROY;
	c = uiWindowsNewControl(&p);
	uiFree(wtext);

	c->preferredSize = preferredSize;

	hwnd = (HWND) uiControlHandle(c);
	SendMessageW(hwnd, WM_SETFONT, (WPARAM) hMessageFont, (LPARAM) TRUE);

	b = uiNew(struct button);
	b->onClicked = defaultOnClicked;
	c->data = b;

	return c;
}

char *uiButtonText(uiControl *c)
{
	return uiWindowsControlText(c);
}

void uiButtonSetText(uiControl *c, const char *text)
{
	uiWindowsControlSetText(c, text);
}

void uiButtonOnClicked(uiControl *c, void (*f)(uiControl *, void *), void *data)
{
	struct button *b = (struct button *) (c->data);

	b->onClicked = f;
	b->onClickedData = data;
}
