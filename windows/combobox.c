// 20 may 2015
#include "uipriv_windows.h"

// TODO
// - is there extra space on the bottom?

// we as Common Controls 6 users don't need to worry about the height of comboboxes; see http://blogs.msdn.com/b/oldnewthing/archive/2006/03/10/548537.aspx

struct uiCombobox {
	uiWindowsControl c;
	HWND hwnd;
	void (*onSelected)(uiCombobox *, void *);
	void *onSelectedData;
};

uiWindowsDefineControlWithOnDestroy(
	uiCombobox,							// type name
	uiComboboxType,						// type function
	uiWindowsUnregisterWM_COMMANDHandler(this->hwnd);	// on destroy
)

// note: NOT triggered on entering text
static BOOL onWM_COMMAND(uiControl *cc, HWND hwnd, WORD code, LRESULT *lResult)
{
	uiCombobox *c = uiCombobox(cc);

	if (code != CBN_SELCHANGE)
		return FALSE;
	(*(c->onSelected))(c, c->onSelectedData);
	*lResult = 0;
	return TRUE;
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define comboboxWidth 107 /* this is actually the shorter progress bar width, but Microsoft only indicates as wide as necessary */
#define comboboxHeight 14

static void minimumSize(uiWindowsControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiWindowsDlgUnitsToX(comboboxWidth, d->BaseX);
	*height = uiWindowsDlgUnitsToY(comboboxHeight, d->BaseY);
}

static void defaultOnSelected(uiCombobox *c, void *data)
{
	// do nothing
}

void uiComboboxAppend(uiCombobox *c, const char *text)
{
	WCHAR *wtext;
	LRESULT res;

	wtext = toUTF16(text);
	res = SendMessageW(c->hwnd, CB_ADDSTRING, 0, (LPARAM) wtext);
	if (res == (LRESULT) CB_ERR)
		logLastError("error appending item to uiCombobox");
	else if (res == (LRESULT) CB_ERRSPACE)
		logLastError("memory exhausted appending item to uiCombobox");
	uiFree(wtext);
}

intmax_t uiComboboxSelected(uiCombobox *c)
{
	LRESULT n;

	n = SendMessage(c->hwnd, CB_GETCURSEL, 0, 0);
	if (n == (LRESULT) CB_ERR)
		return -1;
	return (intmax_t) n;
}

void uiComboboxSetSelected(uiCombobox *c, intmax_t n)
{
	// TODO error check
	SendMessageW(c->hwnd, CB_SETCURSEL, (WPARAM) n, 0);
}

void uiComboboxOnSelected(uiCombobox *c, void (*f)(uiCombobox *c, void *data), void *data)
{
	c->onSelected = f;
	c->onSelectedData = data;
}

static uiCombobox *finishNewCombobox(DWORD style)
{
	uiCombobox *c;

	c = (uiCombobox *) uiNewControl(uiComboboxType());

	c->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		L"combobox", L"",
		style | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

	uiWindowsRegisterWM_COMMANDHandler(c->hwnd, onWM_COMMAND, uiControl(c));
	uiComboboxOnSelected(c, defaultOnSelected, NULL);

	uiWindowsFinishNewControl(c, uiCombobox);

	return c;
}

uiCombobox *uiNewCombobox(void)
{
	return finishNewCombobox(CBS_DROPDOWNLIST);
}

uiCombobox *uiNewEditableCombobox(void)
{
	return finishNewCombobox(CBS_DROPDOWN);
}
