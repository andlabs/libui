// 14 april 2016
#include "uipriv_windows.h"

// TODO implement a custom font dialog that uses DirectWrite?

struct uiFontButton {
	uiWindowsControl c;
	HWND hwnd;
	LOGFONTW font;
	INT pointSize;
	BOOL already;
};

uiWindowsDefineControlWithOnDestroy(
	uiFontButton,							// type name
	uiFontButtonType,						// type function
	uiWindowsUnregisterWM_COMMANDHandler(this->hwnd);	// on destroy
)

static void updateFontButtonLabel(uiFontButton *b)
{
	// TODO
	SetWindowTextW(b->hwnd, b->font.lfFaceName);
	// changing the text might necessitate a change in the button's size
	uiWindowsControlQueueRelayout(uiWindowsControl(b));
}

static BOOL onWM_COMMAND(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	uiFontButton *b = uiFontButton(c);
	CHOOSEFONTW cf;

	if (code != BN_CLICKED)
		return FALSE;

	ZeroMemory(&cf, sizeof (CHOOSEFONTW));
	cf.lStructSize = sizeof (CHOOSEFONTW);
	cf.hwndOwner = GetAncestor(b->hwnd, GA_ROOT);		// TODO didn't we have a function for this
showFontDialog(cf.hwndOwner);
	cf.lpLogFont = &(b->font);
ZeroMemory(&(b->font), sizeof(LOGFONTW));
b->font.lfFaceName[0]='A';
b->font.lfFaceName[1]='r';
b->font.lfFaceName[2]='i';
b->font.lfFaceName[3]='a';
b->font.lfFaceName[4]='l';
b->font.lfFaceName[5]=0;
b->font.lfHeight=-15*96/72;
	// TODO CF_FORCEFONTEXIST? CF_INACTIVEFONTS? CF_NOSCRIPTSEL? CF_USESTYLE?
//	if (b->already)
		cf.Flags = CF_INITTOLOGFONTSTRUCT;
	if (ChooseFontW(&cf) != FALSE) {
		b->already = TRUE;
		updateFontButtonLabel(b);
		// TODO event
	} else {
		DWORD err;

		err = CommDlgExtendedError();
		if (err != 0)
			// TODO
			logLastError("TODO help");
	}

	*lResult = 0;
	return TRUE;
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define buttonHeight 14

static void minimumSize(uiWindowsControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	uiFontButton *b = uiFontButton(c);
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

#if 0
TODO
static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
}
#endif

#if 0
TODO
void uiButtonOnClicked(uiButton *b, void (*f)(uiButton *, void *), void *data)
{
	b->onClicked = f;
	b->onClickedData = data;
}
#endif

uiFontButton *uiNewFontButton(void)
{
	uiFontButton *b;

	b = (uiFontButton *) uiNewControl(uiFontButtonType());

	b->hwnd = uiWindowsEnsureCreateControlHWND(0,
		// TODO
		L"button", L"Arial 10",
		BS_PUSHBUTTON | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

	uiWindowsRegisterWM_COMMANDHandler(b->hwnd, onWM_COMMAND, uiControl(b));
//TODO	uiButtonOnClicked(b, defaultOnClicked, NULL);

	uiWindowsFinishNewControl(b, uiFontButton);

	return b;
}
