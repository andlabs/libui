// 7 april 2015
#include "uipriv_windows.h"

struct button {
	uiButton b;
	void (*onClicked)(uiButton *, void *);
	void *onClickedData;
};

static BOOL onWM_COMMAND(uiControl *c, WORD code, LRESULT *lResult)
{
	struct button *b = (struct button *) c;

	if (code != BN_CLICKED)
		return FALSE;
	(*(b->onClicked))(uiButton(b), b->onClickedData);
	*lResult = 0;
	return TRUE;
}

static BOOL onWM_NOTIFY(uiControl *c, NMHDR *nm, LRESULT *lResult)
{
	return FALSE;
}

static void onWM_DESTROY(uiControl *c)
{
	struct button *b = (struct button *) c;

	uiFree(b);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define buttonHeight 14

static void preferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	HWND hwnd;
	SIZE size;

	hwnd = uiControlHWND(c);

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

static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
}

static char *getText(uiButton *b)
{
	return uiWindowsControlText(uiControl(c));
}

static void setText(uiButton *b, const char *text)
{
	uiWindowsControlSetText(uiControl(b), text);
}

static void setOnClicked(uiButton *b, void (*f)(uiButton *, void *), void *data)
{
	struct button *b = (struct button *) b;

	b->onClicked = f;
	b->onClickedData = data;
}

uiButton *uiNewButton(const char *text)
{
	struct button *b;
	uiWindowsNewControlParams p;
	WCHAR *wtext;

	b = uiNew(struct button);

	p.dwExStyle = 0;
	p.lpClassName = L"button";
	wtext = toUTF16(text);
	p.lpWindowName = wtext;
	p.dwStyle = BS_PUSHBUTTON | WS_TABSTOP;
	p.hInstance = hInstance;
	p.useStandardControlFont = TRUE;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onWM_DESTROY = onWM_DESTROY;
	uiWindowsNewControl(uiControl(b), &p);
	uiFree(wtext);

	b->onClicked = defaultOnClicked;

	uiControl(b)->preferredSize = preferredSize;

	uiButton(b)->Text = getText;
	uiButton(b)->SetText = setText;
	uiButton(b)->OnClicked = setOnClicked;

	return uiButton(b);
}
