// 14 april 2016
#include "uipriv_windows.hpp"

struct uiFontButton {
	uiWindowsControl c;
	HWND hwnd;
	struct fontDialogParams params;
	BOOL already;
	void (*onChanged)(uiFontButton *, void *);
	void *onChangedData;
};

static void onDestroy(uiFontButton *);

uiWindowsDefineControlWithOnDestroy(
	uiFontButton,							// type name
	onDestroy(me);						// on destroy
)

static void onDestroy(uiFontButton *b)
{
	uiWindowsUnregisterWM_COMMANDHandler(b->hwnd);
	destroyFontDialogParams(&(b->params));
}

static void updateFontButtonLabel(uiFontButton *b)
{
	WCHAR *text;

	text = fontDialogParamsToString(&(b->params));
	// TODO error check
	SendMessageW(b->hwnd, WM_SETTEXT, 0, (LPARAM) text);
	uiFree(text);

	// changing the text might necessitate a change in the button's size
	uiWindowsControlMinimumSizeChanged(uiWindowsControl(b));
}

static BOOL onWM_COMMAND(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	uiFontButton *b = uiFontButton(c);
	HWND parent;

	if (code != BN_CLICKED)
		return FALSE;

	parent = GetAncestor(b->hwnd, GA_ROOT);		// TODO didn't we have a function for this
	if (showFontDialog(parent, &(b->params))) {
		updateFontButtonLabel(b);
		(*(b->onChanged))(b, b->onChangedData);
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

static void defaultOnChanged(uiFontButton *b, void *data)
{
	// do nothing
}

uiDrawTextFont *uiFontButtonFont(uiFontButton *b)
{
	// we don't own b->params.font; we have to add a reference
	// we don't own b->params.familyName either; we have to copy it
	return mkTextFont(b->params.font, TRUE, b->params.familyName, TRUE, b->params.size);
}

// TODO document that the Handle of a Font may not be unique

void uiFontButtonOnChanged(uiFontButton *b, void (*f)(uiFontButton *, void *), void *data)
{
	b->onChanged = f;
	b->onChangedData = data;
}

uiFontButton *uiNewFontButton(void)
{
	uiFontButton *b;

	b = (uiFontButton *) uiNewControl(uiFontButton);

	b->hwnd = uiWindowsEnsureCreateControlHWND(0,
		L"button", L"you should not be seeing this",
		BS_PUSHBUTTON | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

	loadInitialFontDialogParams(&(b->params));

	uiWindowsRegisterWM_COMMANDHandler(b->hwnd, onWM_COMMAND, uiControl(b));
	uiFontButtonOnChanged(b, defaultOnChanged, NULL);

	uiWindowsFinishNewControl(b, uiFontButton);

	// TODO move this back above the previous when merging with uiNewControl(); it's here because this calls Handle()
	updateFontButtonLabel(b);

	return b;
}
