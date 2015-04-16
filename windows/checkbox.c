// 7 april 2015
#include "uipriv_windows.h"

struct checkbox {
	uiCheckbox c;
	void (*onToggled)(uiControl *, void *);
	void *onToggledData;
};

static BOOL onWM_COMMAND(uiControl *cc, WORD code, LRESULT *lResult)
{
	struct checkbox *c = (struct checkbox *) cc;
	HWND hwnd;
	WPARAM check;

	if (code != BN_CLICKED)
		return FALSE;

	// we didn't use BS_AUTOCHECKBOX (see controls_windows.go) so we have to manage the check state ourselves
	hwnd = uiControlHWND(c);
	check = BST_CHECKED;
	if (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED)
		check = BST_UNCHECKED;
	SendMessage(hwnd, BM_SETCHECK, check, 0);

	(*(c->onToggled))(uiCheckbox(c), c->onToggledData);
	*lResult = 0;
	return TRUE;
}

static BOOL onWM_NOTIFY(uiControl *c, NMHDR *nm, LRESULT *lResult)
{
	return FALSE;
}

static void onWM_DESTROY(uiControl *cc)
{
	struct checkbox *c = (struct checkbox *) cc;

	uiFree(c);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define checkboxHeight 10
// from http://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
#define checkboxXFromLeftOfBoxToLeftOfLabel 12

static void preferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiDlgUnitsToX(checkboxXFromLeftOfBoxToLeftOfLabel, d->sys->baseX) + uiWindowsWindowTextWidth(uiControlHWND(c));
	*height = uiDlgUnitsToY(checkboxHeight, d->sys->baseY);
}

static void defaultOnToggled(uiCheckbox *c, void *data)
{
	// do nothing
}

static char *getText(uiCheckbox *c)
{
	return uiWindowsControlText(uiControl(c));
}

static void setText(uiCheckbox *c, const char *text)
{
	uiWindowsControlSetText(uiControl(c), text);
}

static void setOnToggled(uiCheckbox *cc, void (*f)(uiCheckbox *, void *), void *data)
{
	struct checkbox *c = (struct checkbox *) cc;

	c->onToggled = f;
	c->onToggledData = data;
}

static int getChecked(uiChecckbox *c)
{
	HWND hwnd;

	hwnd = uiControlHWND(uiControl(c));
	return SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

static void setChecked(uiCheckbox *c, int checked)
{
	HWND hwnd;
	WPARAM check;

	hwnd = uiControlHWND(uiControl(c));
	check = BST_CHECKED;
	if (!checked)
		check = BST_UNCHECKED;
	SendMessage(hwnd, BM_SETCHECK, check, 0);
}

uiControl *uiNewCheckbox(const char *text)
{
	struct checkbox *c;
	uiWindowsNewControlParams p;
	WCHAR *wtext;

	c = uiNew(struct checkbox)

	p.dwExStyle = 0;
	p.lpClassName = L"button";
	wtext = toUTF16(text);
	p.lpWindowName = wtext;
	p.dwStyle = BS_CHECKBOX | WS_TABSTOP;
	p.hInstance = hInstance;
	p.useStandardControlFont = TRUE;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onWM_DESTROY = onWM_DESTROY;
	uiWindowsNewControl(uiControl(c), &p);
	uiFree(wtext);

	c->onToggled = defaultOnToggled;

	uiControl(c)->PreferredSize = preferredSize;

	uiCheckbox(c)->Text = getText;
	uiCheckbox(c)->SetText = setText;
	uiCheckbox(c)->OnToggled = setOnToggled;
	uiCheckbox(c)->Checked = getChecked;
	uiCheckbox(c)->SetChecked = setChecked;

	return uiCheckbox(c);
}
