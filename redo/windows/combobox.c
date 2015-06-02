// 20 may 2015
#include "uipriv_windows.h"

// TODO point to where it says size is autocomputed on comctl6

struct combobox {
	uiCombobox c;
	HWND hwnd;
};

uiDefineControlType(uiCombobox, uiTypeCombobox, struct combobox)

static uintptr_t comboboxHandle(uiControl *cc)
{
	struct combobox *c = (struct combobox *) cc;

	return (uintptr_t) (c->hwnd);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define comboboxWidth 107 /* this is actually the shorter progress bar width, but Microsoft only indicates as wide as necessary */
#define comboboxHeight 14

static void comboboxPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiWindowsDlgUnitsToX(comboboxWidth, d->Sys->BaseX);
	*height = uiWindowsDlgUnitsToY(comboboxHeight, d->Sys->BaseY);
}

static void comboboxAppend(uiCombobox *cc, const char *text)
{
	struct combobox *c = (struct combobox *) cc;
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
	struct combobox *c;

	c = (struct combobox *) uiWindowsNewSingleHWNDControl(uiTypeCombobox());

	c->hwnd = uiWindowsUtilCreateControlHWND(WS_EX_CLIENTEDGE,
		L"combobox", L"",
		style | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

	uiControl(c)->Handle = comboboxHandle;
	uiControl(c)->PreferredSize = comboboxPreferredSize;

	uiCombobox(c)->Append = comboboxAppend;

	return uiCombobox(c);
}

uiCombobox *uiNewCombobox(void)
{
	return finishNewCombobox(CBS_DROPDOWNLIST);
}

uiCombobox *uiNewEditableCombobox(void)
{
	return finishNewCombobox(CBS_DROPDOWN);
}
