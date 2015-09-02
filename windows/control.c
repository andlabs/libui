// 16 august 2015
#include "uipriv_windows.h"

HWND uiWindowsEnsureCreateControlHWND(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, HINSTANCE hInstance, LPVOID lpParam, BOOL useStandardControlFont)
{
	HWND hwnd;

	// don't let using the arrow keys in a uiRadioButtons leave the radio buttons
	if ((dwStyle & WS_TABSTOP) != 0)
		dwStyle |= WS_GROUP;
	hwnd = CreateWindowExW(dwExStyle,
		lpClassName, lpWindowName,
		dwStyle | WS_CHILD | WS_VISIBLE,
		0, 0,
		// use a nonzero initial size just in case some control breaks with a zero initial size
		100, 100,
		utilWindow, NULL, hInstance, lpParam);
	if (hwnd == NULL)
		logLastError("error creating window in uiWindowsUtilCreateControlHWND()");
	if (useStandardControlFont)
		SendMessageW(hwnd, WM_SETFONT, (WPARAM) hMessageFont, (LPARAM) TRUE);
	return hwnd;
}

static uintmax_t type_uiWindowsControl = 0;

uintmax_t uiWindowsControlType(void)
{
	if (type_uiWindowsControl == 0)
		type_uiWindowsControl = uiRegisterType("uiWindowsControl", uiControlType(), sizeof (uiWindowsControl));
	return type_uiWindowsControl;
}

static void defaultCommitShow(uiControl *c)
{
	ShowWindow((HWND) uiControlHandle(c), SW_SHOW);
}

static void defaultCommitHide(uiControl *c)
{
	ShowWindow((HWND) uiControlHandle(c), SW_HIDE);
}

void osCommitEnable(uiControl *c)
{
	EnableWindow((HWND) uiControlHandle(c), TRUE);
}

void osCommitDisable(uiControl *c)
{
	EnableWindow((HWND) uiControlHandle(c), FALSE);
}

void uiWindowsFinishControl(uiControl *c)
{
	c->CommitShow = defaultCommitShow;
	c->CommitHide = defaultCommitHide;
}

char *uiWindowsUtilText(HWND hwnd)
{
	WCHAR *wtext;
	char *text;

	wtext = windowText(hwnd);
	text = toUTF8(wtext);
	uiFree(wtext);
	return text;
}

void uiWindowsUtilSetText(HWND hwnd, const char *text)
{
	WCHAR *wtext;

	wtext = toUTF16(text);
	if (SetWindowTextW(hwnd, wtext) == 0)
		logLastError("error setting control text in uiWindowsControlSetText()");
	uiFree(wtext);
}

void uiWindowsRearrangeControlIDsZOrder(uiControl *c)
{
	uiWindowsControl *wc;

	c = uiControlParent(c);
	if (c == NULL)
		return;
	wc = uiWindowsControl(c);
	(*(wc->ArrangeChildrenControlIDsZOrder))(wc);
}
