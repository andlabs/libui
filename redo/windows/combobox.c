// 20 may 2015
#include "uipriv_windows.h"

// we as Common Controls 6 users don't need to worry about the height of comboboxes; see http://blogs.msdn.com/b/oldnewthing/archive/2006/03/10/548537.aspx

struct uiCombobox {
	uiWindowsControl c;
	HWND hwnd;
};

uiWindowsDefineControl(
	uiCombobox,							// type name
	uiComboboxType						// type function
)

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define comboboxWidth 107 /* this is actually the shorter progress bar width, but Microsoft only indicates as wide as necessary */
#define comboboxHeight 14

static void minimumSize(uiControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiWindowsDlgUnitsToX(comboboxWidth, d->BaseX);
	*height = uiWindowsDlgUnitsToY(comboboxHeight, d->BaseY);
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

static uiCombobox *finishNewCombobox(DWORD style)
{
	uiCombobox *c;

	c = (uiCombobox *) uiNewControl(uiComboboxType());

	c->hwnd = uiWindowsEnsureCreateControlHWND(WS_EX_CLIENTEDGE,
		L"combobox", L"",
		style | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

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
