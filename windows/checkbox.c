// 7 april 2015
#include "uipriv_windows.h"

struct checkbox {
	uiCheckbox c;
	HWND hwnd;
	void (*onToggled)(uiCheckbox *, void *);
	void *onToggledData;
};

static BOOL onWM_COMMAND(uiControl *cc, WORD code, LRESULT *lResult)
{
	struct checkbox *c = (struct checkbox *) cc;
	WPARAM check;

	if (code != BN_CLICKED)
		return FALSE;

	// we didn't use BS_AUTOCHECKBOX (see controls_windows.go) so we have to manage the check state ourselves
	check = BST_CHECKED;
	if (SendMessage(c->hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED)
		check = BST_UNCHECKED;
	SendMessage(c->hwnd, BM_SETCHECK, check, 0);

	(*(c->onToggled))(uiCheckbox(c), c->onToggledData);
	*lResult = 0;
	return TRUE;
}

static BOOL onWM_NOTIFY(uiControl *c, NMHDR *nm, LRESULT *lResult)
{
	return FALSE;
}

static void onDestroy(void *data)
{
	struct checkbox *c = (struct checkbox *) data;

	uiFree(c);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define checkboxHeight 10
// from http://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
#define checkboxXFromLeftOfBoxToLeftOfLabel 12

static void checkboxPreferredSize(uiControl *cc, uiSizing *d, intmax_t *width, intmax_t *height)
{
	struct checkbox *c = (struct checkbox *) cc;

	*width = uiWindowsDlgUnitsToX(checkboxXFromLeftOfBoxToLeftOfLabel, d->sys->baseX) + uiWindowsWindowTextWidth(c->hwnd);
	*height = uiWindowsDlgUnitsToY(checkboxHeight, d->sys->baseY);
}

static void defaultOnToggled(uiCheckbox *c, void *data)
{
	// do nothing
}

static char *checkboxText(uiCheckbox *c)
{
	return uiWindowsControlText(uiControl(c));
}

static void checkboxSetText(uiCheckbox *c, const char *text)
{
	uiWindowsControlSetText(uiControl(c), text);
}

static void checkboxOnToggled(uiCheckbox *cc, void (*f)(uiCheckbox *, void *), void *data)
{
	struct checkbox *c = (struct checkbox *) cc;

	c->onToggled = f;
	c->onToggledData = data;
}

static int checkboxChecked(uiCheckbox *cc)
{
	struct checkbox *c = (struct checkbox *) cc;

	return SendMessage(c->hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

static void checkboxSetChecked(uiCheckbox *cc, int checked)
{
	struct checkbox *c = (struct checkbox *) cc;
	WPARAM check;

	check = BST_CHECKED;
	if (!checked)
		check = BST_UNCHECKED;
	SendMessage(c->hwnd, BM_SETCHECK, check, 0);
}

uiCheckbox *uiNewCheckbox(const char *text)
{
	struct checkbox *c;
	uiWindowsMakeControlParams p;
	WCHAR *wtext;

	c = uiNew(struct checkbox);

	p.dwExStyle = 0;
	p.lpClassName = L"button";
	wtext = toUTF16(text);
	p.lpWindowName = wtext;
	p.dwStyle = BS_CHECKBOX | WS_TABSTOP;
	p.hInstance = hInstance;
	p.useStandardControlFont = TRUE;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onDestroy = onDestroy;
	p.onDestroyData = c;
	uiWindowsMakeControl(uiControl(c), &p);
	uiFree(wtext);

	c->hwnd = (HWND) uiControlHandle(uiControl(c));

	c->onToggled = defaultOnToggled;

	uiControl(c)->PreferredSize = checkboxPreferredSize;

	uiCheckbox(c)->Text = checkboxText;
	uiCheckbox(c)->SetText = checkboxSetText;
	uiCheckbox(c)->OnToggled = checkboxOnToggled;
	uiCheckbox(c)->Checked = checkboxChecked;
	uiCheckbox(c)->SetChecked = checkboxSetChecked;

	return uiCheckbox(c);
}
