// 7 april 2015
#include "uipriv_windows.h"

struct button {
	uiControl *c;
	void (*onClicked)(uiControl *, void *);
	void *onClickedData;
};

#define B(x) ((struct button *) (x))

static BOOL onWM_COMMAND(uiControl *c, WPARAM wParam, LPARAM lParam, void *data, LRESULT *lResult)
{
	if (HIWORD(wParam) != BN_CLICKED)
		return FALSE;
	(*(B(data)->onClicked))(c, B(data)->onClickedData);
	*lResult = 0;
	return TRUE;
}

static BOOL onWM_NOTIFY(uiControl *c, WPARAM wParam, LPARAM lParam, void *data, LRESULT *lResult)
{
	return FALSE;
}

static void onWM_DESTROY(uiControl *c, void *data)
{
	struct button *b = (struct button *) data;

	uiFree(b);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define buttonHeight 14

static void preferredSize(uiControl *c, int baseX, int baseY, LONG internalLeading, intmax_t *width, intmax_t *height)
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
	*height = uiDlgUnitToY(buttonHeight, baseY);
}

static void defaultOnClicked(uiControl *c, void *data)
{
	// do nothing
}

// TODO destruction
uiControl *uiNewButton(const char *text)
{
	struct button *b;
	uiWindowsNewControlParams p;
	WCHAR *wtext;
	HWND hwnd;

	b = uiNew(struct button);

	p.dwExStyle = 0;
	p.lpClassName = L"button";
	p.dwStyle = BS_PUSHBUTTON | WS_TABSTOP;
	p.hInstance = hInstance;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onWM_DESTROY = onWM_DESTROY;
	p.onCommandNotifyDestroyData = b;
	p.preferredSize = preferredSize;
	p.data = b;
	b->c = uiWindowsNewControl(&p);

	hwnd = (HWND) uiControlHandle(b->c);
	wtext = toUTF16(text);
	if (SetWindowTextW(hwnd, wtext) == 0)
		logLastError("error setting button text in uiNewButton()");
	uiFree(wtext);
	SendMessageW(hwnd, WM_SETFONT, (WPARAM) hMessageFont, (LPARAM) TRUE);

	b->onClicked = defaultOnClicked;

	return b->c;
}

// TODO text

void uiButtonOnClicked(uiControl *c, void (*f)(uiControl *, void *), void *data)
{
	struct button *b;

	b = (struct button *) uiWindowsControlData(c);
	b->onClicked = f;
	b->onClickedData = data;
}
