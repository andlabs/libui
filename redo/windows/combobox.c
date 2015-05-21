// 20 may 2015
#include "uipriv_windows.h"

struct combobox {
	uiCombobox c;
	HWND hwnd;
};

static void onDestroy(void *data)
{
	struct combobox *c = (struct combobox *) data;

	uiFree(c);
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

uiCombobox *uiNewCombobox(void)
{
	struct combobox *c;
	uiWindowsMakeControlParams p;

	c = uiNew(struct combobox);
	uiTyped(c)->Type = uiTypeCombobox();

	p.dwExStyle = WS_EX_CLIENTEDGE;
	p.lpClassName = L"combobox";
	p.lpWindowName = L"";
	p.dwStyle = CBS_DROPDOWNLIST | WS_TABSTOP;
	p.hInstance = hInstance;
	p.lpParam = NULL;
	p.useStandardControlFont = TRUE;
	p.onDestroy = onDestroy;
	p.onDestroyData = c;
	uiWindowsMakeControl(uiControl(c), &p);

	c->hwnd = (HWND) uiControlHandle(uiControl(c));

	uiControl(c)->PreferredSize = comboboxPreferredSize;

	uiCombobox(c)->Append = comboboxAppend;

	return uiCombobox(c);
}
