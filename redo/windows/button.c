// 7 april 2015
#include "uipriv_windows.h"

struct uiButton {
	uiWindowsControl c;
	HWND hwnd;
	void (*onClicked)(uiButton *, void *);
	void *onClickedData;
};

uiWindowsDefineControlWithOnDestroy(
	uiButton,								// type name
	uiButtonType,							// type function
	uiWindowsUnregisterWM_COMMANDHandler(this->hwnd);	// on destroy
)

static BOOL onWM_COMMAND(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	uiButton *b = uiButton(c);

	if (code != BN_CLICKED)
		return FALSE;
	(*(b->onClicked))(b, b->onClickedData);
	*lResult = 0;
	return TRUE;
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define buttonHeight 14

static void minimumSize(uiWindowsControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	uiButton *b = uiButton(c);
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
	*height = uiWindowsDlgUnitsToY(buttonHeight, d->BaseY);
}

static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
}

char *uiButtonText(uiButton *b)
{
	return uiWindowsUtilText(b->hwnd);
}

void uiButtonSetText(uiButton *b, const char *text)
{
	uiWindowsUtilSetText(b->hwnd, text);
	// changing the text might necessitate a change in the button's size
	uiControlQueueResize(uiControl(b));
}

void uiButtonOnClicked(uiButton *b, void (*f)(uiButton *, void *), void *data)
{
	b->onClicked = f;
	b->onClickedData = data;
}

uiButton *uiNewButton(const char *text)
{
	uiButton *b;
	WCHAR *wtext;

	b = (uiButton *) uiNewControl(uiButtonType());

	wtext = toUTF16(text);
	b->hwnd = uiWindowsEnsureCreateControlHWND(0,
		L"button", wtext,
		BS_PUSHBUTTON | WS_TABSTOP,
		hInstance, NULL,
		TRUE);
	uiFree(wtext);

	uiWindowsRegisterWM_COMMANDHandler(b->hwnd, onWM_COMMAND, uiControl(b));
	uiButtonOnClicked(b, defaultOnClicked, NULL);

	uiWindowsFinishNewControl(b, uiButton);

	return b;
}
