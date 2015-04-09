// 7 april 2015
#include "uipriv_windows.h"

struct checkbox {
	void (*onToggled)(uiControl *, void *);
	void *onToggledData;
};

static BOOL onWM_COMMAND(uiControl *c, WORD code, LRESULT *lResult)
{
	struct checkbox *cc = (struct checkbox *) (c->data);
	HWND hwnd;
	WPARAM check;

	if (code != BN_CLICKED)
		return FALSE;

	// we didn't use BS_AUTOCHECKBOX (see controls_windows.go) so we have to manage the check state ourselves
	hwnd = (HWND) uiControlHandle(c);
	check = BST_CHECKED;
	if (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED)
		check = BST_UNCHECKED;
	SendMessage(hwnd, BM_SETCHECK, check, 0);

	(*(cc->onToggled))(c, cc->onToggledData);
	*lResult = 0;
	return TRUE;
}

static BOOL onWM_NOTIFY(uiControl *c, WPARAM wParam, LPARAM lParam, LRESULT *lResult)
{
	return FALSE;
}

static void onWM_DESTROY(uiControl *c)
{
	struct checkbox *cc = (struct checkbox *) (c->data);

	uiFree(cc);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define checkboxHeight 10
// from http://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
#define checkboxXFromLeftOfBoxToLeftOfLabel 12

static void preferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	*width = uiDlgUnitsToX(checkboxXFromLeftOfBoxToLeftOfLabel, d->sys->baseX) + uiWindowsWindowTextWidth((HWND) uiControlHandle(c));
	*height = uiDlgUnitsToY(checkboxHeight, d->sys->baseY);
}

static void defaultOnToggled(uiControl *c, void *data)
{
	// do nothing
}

uiControl *uiNewCheckbox(const char *text)
{
	uiControl *c;
	struct checkbox *cc;
	uiWindowsNewControlParams p;
	WCHAR *wtext;
	HWND hwnd;

	p.dwExStyle = 0;
	p.lpClassName = L"button";
	wtext = toUTF16(text);
	p.lpWindowName = wtext;
	p.dwStyle = BS_CHECKBOX | WS_TABSTOP;
	p.hInstance = hInstance;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onWM_DESTROY = onWM_DESTROY;
	c = uiWindowsNewControl(&p);
	uiFree(wtext);

	c->preferredSize = preferredSize;

	hwnd = (HWND) uiControlHandle(c);
	SendMessageW(hwnd, WM_SETFONT, (WPARAM) hMessageFont, (LPARAM) TRUE);

	cc = uiNew(struct checkbox);
	cc->onToggled = defaultOnToggled;
	c->data = cc;

	return c;
}

char *uiCheckboxText(uiControl *c)
{
	return uiWindowsControlText(c);
}

void uiCheckboxSetText(uiControl *c, const char *text)
{
	uiWindowsControlSetText(c, text);
}

void uiCheckboxOnToggled(uiControl *c, void (*f)(uiControl *, void *), void *data)
{
	struct checkbox *cc = (struct checkbox *) (c->data);

	cc->onToggled = f;
	cc->onToggledData = data;
}

int uiCheckboxChecked(uiControl *c)
{
	HWND hwnd;

	hwnd = (HWND) uiControlHandle(c);
	return SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void uiCheckboxSetChecked(uiControl *c, int checked)
{
	HWND hwnd;
	WPARAM check;

	hwnd = (HWND) uiControlHandle(c);
	check = BST_CHECKED;
	if (!checked)
		check = BST_UNCHECKED;
	SendMessage(hwnd, BM_SETCHECK, check, 0);
}
