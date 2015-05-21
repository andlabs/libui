// 7 april 2015
#include "uipriv_windows.h"

struct button {
	uiButton b;
	HWND hwnd;
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

static BOOL onWM_HSCROLL(uiControl *c, WORD code, LRESULT *lResult)
{
	return FALSE;
}

static void onDestroy(void *data)
{
	struct button *b = (struct button *) data;

	uiWindowsUnregisterWM_COMMANDHandler(b->hwnd);
	uiFree(b);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define buttonHeight 14

static void buttonPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct button *b = (struct button *) c;
	SIZE size;

	// try the comctl32 version 6 way
	size.cx = 0;		// explicitly ask for ideal size
	size.cy = 0;
	if (SendMessageW(b->hwnd, BCM_GETIDEALSIZE, 0, (LPARAM) (&size)) != FALSE) {
		*width = size.cx;
		*height = size.cy;
		return;
	}

	// that didn't work; fall back to using Microsoft's metrics
	// Microsoft says to use a fixed width for all buttons; this isn't good enough
	// use the text width instead, with some edge padding
	*width = uiWindowsWindowTextWidth(b->hwnd) + (2 * GetSystemMetrics(SM_CXEDGE));
	*height = uiWindowsDlgUnitsToY(buttonHeight, d->Sys->BaseY);
}

static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
}

static char *buttonText(uiButton *b)
{
	return uiWindowsControlText(uiControl(b));
}

static void buttonSetText(uiButton *b, const char *text)
{
	uiWindowsControlSetText(uiControl(b), text);
}

static void buttonOnClicked(uiButton *bb, void (*f)(uiButton *, void *), void *data)
{
	struct button *b = (struct button *) bb;

	b->onClicked = f;
	b->onClickedData = data;
}

uiButton *uiNewButton(const char *text)
{
	struct button *b;
	uiWindowsMakeControlParams p;
	WCHAR *wtext;

	b = uiNew(struct button);
	uiTyped(b)->Type = uiTypeButton();

	p.dwExStyle = 0;
	p.lpClassName = L"button";
	wtext = toUTF16(text);
	p.lpWindowName = wtext;
	p.dwStyle = BS_PUSHBUTTON | WS_TABSTOP;
	p.hInstance = hInstance;
	p.lpParam = NULL;
	p.useStandardControlFont = TRUE;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onWM_HSCROLL = onWM_HSCROLL;
	p.onDestroy = onDestroy;
	p.onDestroyData = b;
	uiWindowsMakeControl(uiControl(b), &p);
	uiFree(wtext);

	b->hwnd = (HWND) uiControlHandle(uiControl(b));
	uiWindowsRegisterWM_COMMANDHandler(b->hwnd, onWM_COMMAND, uiControl(b));

	b->onClicked = defaultOnClicked;

	uiControl(b)->PreferredSize = buttonPreferredSize;

	uiButton(b)->Text = buttonText;
	uiButton(b)->SetText = buttonSetText;
	uiButton(b)->OnClicked = buttonOnClicked;

	return uiButton(b);
}
