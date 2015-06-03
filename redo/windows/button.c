// 7 april 2015
#include "uipriv_windows.h"

struct button {
	uiButton b;
	HWND hwnd;
	void (*onClicked)(uiButton *, void *);
	void *onClickedData;
	void (*baseCommitDestroy)(uiControl *);
};

uiDefineControlType(uiButton, uiTypeButton, struct button)

static BOOL onWM_COMMAND(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	struct button *b = (struct button *) c;

	if (code != BN_CLICKED)
		return FALSE;
	(*(b->onClicked))(uiButton(b), b->onClickedData);
	*lResult = 0;
	return TRUE;
}

static void buttonCommitDestroy(uiControl *c)
{
	struct button *b = (struct button *) c;

	uiWindowsUnregisterWM_COMMANDHandler(b->hwnd);
	(*(b->baseCommitDestroy))(uiControl(b));
}

static uintptr_t buttonHandle(uiControl *c)
{
	struct button *b = (struct button *) c;

	return (uintptr_t) (b->hwnd);
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
	return uiWindowsSingleHWNDControlText(uiControl(b));
}

static void buttonSetText(uiButton *b, const char *text)
{
	uiWindowsSingleHWNDControlSetText(uiControl(b), text);
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
	WCHAR *wtext;

	b = (struct button *) uiWindowsNewSingleHWNDControl(uiTypeButton());

	wtext = toUTF16(text);
	b->hwnd = uiWindowsUtilCreateControlHWND(0,
		L"button", wtext,
		BS_PUSHBUTTON | WS_TABSTOP,
		hInstance, NULL,
		TRUE);
	uiFree(wtext);

	uiWindowsRegisterWM_COMMANDHandler(b->hwnd, onWM_COMMAND, uiControl(b));

	b->onClicked = defaultOnClicked;

	uiControl(b)->Handle = buttonHandle;
	uiControl(b)->PreferredSize = buttonPreferredSize;
	b->baseCommitDestroy = uiControl(b)->CommitDestroy;
	uiControl(b)->CommitDestroy = buttonCommitDestroy;

	uiButton(b)->Text = buttonText;
	uiButton(b)->SetText = buttonSetText;
	uiButton(b)->OnClicked = buttonOnClicked;

	return uiButton(b);
}
