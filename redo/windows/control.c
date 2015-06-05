// 27 may 2015
#include "uipriv_windows.h"

// TODO Edit ,s/Util/HWND/g ?

HWND uiWindowsUtilCreateControlHWND(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, HINSTANCE hInstance, LPVOID lpParam, BOOL useStandardControlFont)
{
	HWND hwnd;

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

#define HWND(c) ((HWND) uiControlHandle((c)))

void uiWindowsUtilDestroy(HWND hwnd)
{
	if (DestroyWindow(hwnd) == 0)
		logLastError("error destroying window in uiWindowsUtilDestroyWindow()");
}

static void singleHWNDCommitDestroy(uiControl *c)
{
	uiWindowsUtilDestroy(HWND(c));
}

void uiWindowsUtilSetParent(HWND hwnd, uiControl *parent)
{
	HWND newParent;

	newParent = utilWindow;
	if (parent != NULL)
		newParent = HWND(parent);
	if (SetParent(hwnd, newParent) == 0)
		logLastError("error changing window parent in uiWindowsUtilSetParent()");
}

static void singleHWNDCommitSetParent(uiControl *c, uiControl *parent)
{
	uiWindowsUtilSetParent(HWND(c), parent);
}

void uiWindowsUtilResize(HWND hwnd, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	moveWindow(hwnd, x, y, width, height, d);
}

static void singleHWNDResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	uiWindowsUtilResize(HWND(c), x, y, width, height, d);
}

static uiSizing *singleHWNDSizing(uiControl *c)
{
	// TODO change this to take a HWND and the parent
	return uiWindowsSizing(c);
}

void uiWindowsUtilShow(HWND hwnd)
{
	ShowWindow(hwnd, SW_SHOW);
}

static void singleHWNDCommitShow(uiControl *c)
{
	uiWindowsUtilShow(HWND(c));
}

void uiWindowsUtilHide(HWND hwnd)
{
	ShowWindow(hwnd, SW_HIDE);
}

static void singleHWNDCommitHide(uiControl *c)
{
	uiWindowsUtilHide(HWND(c));
}

void uiWindowsUtilEnable(HWND hwnd)
{
	EnableWindow(hwnd, TRUE);
}

static void singleHWNDCommitEnable(uiControl *c)
{
	uiWindowsUtilEnable(HWND(c));
}

void uiWindowsUtilDisable(HWND hwnd)
{
	EnableWindow(hwnd, FALSE);
}

static void singleHWNDCommitDisable(uiControl *c)
{
	uiWindowsUtilDisable(HWND(c));
}

uintptr_t uiWindowsUtilStartZOrder(HWND hwnd)
{
	HWND insertAfter;
	DWORD le;

	// see http://stackoverflow.com/questions/30491418/
	// also, the window at the beginning of the z-order has no previous window, so GetWindow() returns NULL
	// we have to differentiate these error states
	SetLastError(0);
	insertAfter = GetWindow(hwnd, GW_HWNDPREV);
	if (insertAfter == NULL) {
		le = GetLastError();
		SetLastError(le);		// just in case
		if (le != 0)
			logLastError("error getting insert after window in uiWindowsUtilStartZOrder()");
	}
	return (uintptr_t) insertAfter;
}

static uintptr_t singleHWNDStartZOrder(uiControl *c)
{
	return uiWindowsUtilStartZOrder(HWND(c));
}

uintptr_t uiWindowsUtilSetZOrder(HWND hwnd, uintptr_t insertAfter)
{
	setWindowInsertAfter(hwnd, (HWND) insertAfter);
	return (uintptr_t) hwnd;
}

static uintptr_t singleHWNDSetZOrder(uiControl *c, uintptr_t insertAfter)
{
	return uiWindowsUtilSetZOrder(HWND(c), insertAfter);
}

// TODO should disabled controls return 1? test tabbing across a tab with only disabled controls
int uiWindowsUtilHasTabStops(HWND hwnd)
{
	return (getStyle(hwnd) & WS_TABSTOP) != 0;
}

static int singleHWNDHasTabStops(uiControl *c)
{
	return uiWindowsUtilHasTabStops(HWND(c));
}

void setSingleHWNDFuncs(uiControl *c)
{
	uiControl(c)->CommitDestroy = singleHWNDCommitDestroy;
	uiControl(c)->CommitSetParent = singleHWNDCommitSetParent;
	uiControl(c)->Resize = singleHWNDResize;
	uiControl(c)->Sizing = singleHWNDSizing;
	uiControl(c)->CommitShow = singleHWNDCommitShow;
	uiControl(c)->CommitHide = singleHWNDCommitHide;
	uiControl(c)->CommitEnable = singleHWNDCommitEnable;
	uiControl(c)->CommitDisable = singleHWNDCommitDisable;
	uiControl(c)->StartZOrder = singleHWNDStartZOrder;
	uiControl(c)->SetZOrder = singleHWNDSetZOrder;
	uiControl(c)->HasTabStops = singleHWNDHasTabStops;
}

uiControl *uiWindowsNewSingleHWNDControl(uintmax_t type)
{
	uiControl *c;

	c = uiNewControl(type);
	setSingleHWNDFuncs(c);
	return c;
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
