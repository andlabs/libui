// 7 april 2015
#include "uipriv_windows.h"

struct uiCheckbox {
	uiWindowsControl c;
	HWND hwnd;
	void (*onToggled)(uiCheckbox *, void *);
	void *onToggledData;
};

uiWindowsDefineControlWithOnDestroy(
	uiCheckbox,							// type name
	uiCheckboxType,						// type function
	uiWindowsUnregisterWM_COMMANDHandler(this->hwnd);	// on destroy
)

static BOOL onWM_COMMAND(uiControl *cc, HWND hwnd, WORD code, LRESULT *lResult)
{
	uiCheckbox *c = uiCheckbox(cc);
	WPARAM check;

	if (code != BN_CLICKED)
		return FALSE;

	// we didn't use BS_AUTOCHECKBOX (http://blogs.msdn.com/b/oldnewthing/archive/2014/05/22/10527522.aspx) so we have to manage the check state ourselves
	check = BST_CHECKED;
	if (SendMessage(c->hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED)
		check = BST_UNCHECKED;
	SendMessage(c->hwnd, BM_SETCHECK, check, 0);

	(*(c->onToggled))(c, c->onToggledData);
	*lResult = 0;
	return TRUE;
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define checkboxHeight 10
// from http://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
#define checkboxXFromLeftOfBoxToLeftOfLabel 12

static void minimumSize(uiWindowsControl *cc, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	uiCheckbox *c = uiCheckbox(cc);

	*width = uiWindowsDlgUnitsToX(checkboxXFromLeftOfBoxToLeftOfLabel, d->BaseX) + uiWindowsWindowTextWidth(c->hwnd);
	*height = uiWindowsDlgUnitsToY(checkboxHeight, d->BaseY);
}

static void defaultOnToggled(uiCheckbox *c, void *data)
{
	// do nothing
}

char *uiCheckboxText(uiCheckbox *c)
{
	return uiWindowsUtilText(c->hwnd);
}

void uiCheckboxSetText(uiCheckbox *c, const char *text)
{
	uiWindowsUtilSetText(c->hwnd, text);
	// changing the text might necessitate a change in the checkbox's size
	uiWindowsControlQueueRelayout(uiWindowsControl(c));
}

void uiCheckboxOnToggled(uiCheckbox *c, void (*f)(uiCheckbox *, void *), void *data)
{
	c->onToggled = f;
	c->onToggledData = data;
}

int uiCheckboxChecked(uiCheckbox *c)
{
	return SendMessage(c->hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void uiCheckboxSetChecked(uiCheckbox *c, int checked)
{
	WPARAM check;

	check = BST_CHECKED;
	if (!checked)
		check = BST_UNCHECKED;
	SendMessage(c->hwnd, BM_SETCHECK, check, 0);
}

uiCheckbox *uiNewCheckbox(const char *text)
{
	uiCheckbox *c;
	WCHAR *wtext;

	c = (uiCheckbox *) uiNewControl(uiCheckboxType());

	wtext = toUTF16(text);
	c->hwnd = uiWindowsEnsureCreateControlHWND(0,
		L"button", wtext,
		BS_CHECKBOX | WS_TABSTOP,
		hInstance, NULL,
		TRUE);
	uiFree(wtext);

	uiWindowsRegisterWM_COMMANDHandler(c->hwnd, onWM_COMMAND, uiControl(c));
	uiCheckboxOnToggled(c, defaultOnToggled, NULL);

	uiWindowsFinishNewControl(c, uiCheckbox);

	return c;
}
