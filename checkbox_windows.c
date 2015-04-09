// 7 april 2015
#include "uipriv_windows.h"

struct checkbox {
	uiControl *c;
	void (*onToggled)(uiControl *, void *);
	void *onToggledData;
};

#define C(x) ((struct checkbox *) (x))

static BOOL onWM_COMMAND(uiControl *c, WPARAM wParam, LPARAM lParam, void *data, LRESULT *lResult)
{
	HWND hwnd;
	WPARAM check;

	if (HIWORD(wParam) != BN_CLICKED)
		return FALSE;

	// we didn't use BS_AUTOCHECKBOX (see controls_windows.go) so we have to manage the check state ourselves
	hwnd = (HWND) uiControlHandle(c);
	check = BST_CHECKED;
	if (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED)
		check = BST_UNCHECKED;
	SendMessage(hwnd, BM_SETCHECK, check, 0);

	(*(C(data)->onToggled))(c, C(data)->onToggledData);
	*lResult = 0;
	return TRUE;
}

static BOOL onWM_NOTIFY(uiControl *c, WPARAM wParam, LPARAM lParam, void *data, LRESULT *lResult)
{
	return FALSE;
}

static void onWM_DESTROY(uiControl *c, void *data)
{
	struct checkbox *cc = (struct checkbox *) data;

	uiFree(cc);
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define checkboxHeight 10
// from http://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx
#define checkboxXFromLeftOfBoxToLeftOfLabel 12

static void preferredSize(uiControl *c, int baseX, int baseY, LONG internalLeading, intmax_t *width, intmax_t *height)
{
	*width = uiDlgUnitToX(checkboxXFromLeftOfBoxToLeftOfLabel, baseX) + uiWindowsWindowTextWidth((HWND) uiControlHandle(c));
	*height = uiDlgUnitToY(checkboxHeight, baseY);
}

static void defaultOnToggled(uiControl *c, void *data)
{
	// do nothing
}

uiControl *uiNewCheckbox(const char *text)
{
	struct checkbox *c;
	uiWindowsNewControlParams p;
	WCHAR *wtext;
	HWND hwnd;

	c = uiNew(struct checkbox);

	p.dwExStyle = 0;
	p.lpClassName = L"button";
	wtext = toUTF16(text);
	p.lpWindowName = wtext;
	p.dwStyle = BS_CHECKBOX | WS_TABSTOP;
	p.hInstance = hInstance;
	p.onWM_COMMAND = onWM_COMMAND;
	p.onWM_NOTIFY = onWM_NOTIFY;
	p.onWM_DESTROY = onWM_DESTROY;
	p.onCommandNotifyDestroyData = c;
	p.preferredSize = preferredSize;
	p.data = c;
	c->c = uiWindowsNewControl(&p);
	uiFree(wtext);

	hwnd = (HWND) uiControlHandle(c->c);
	SendMessageW(hwnd, WM_SETFONT, (WPARAM) hMessageFont, (LPARAM) TRUE);

	c->onToggled = defaultOnToggled;

	return c->c;
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
	struct checkbox *cc;

	cc = (struct checkbox *) uiWindowsControlData(c);
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
