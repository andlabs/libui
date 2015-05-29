// 27 may 2015
#include "uipriv_windows.h"

// TODO Edit ,s/Util/HWND/g ?

HWND uiWindowsUtilCreateControlHWND(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, HINSTANCE hInstance, LPVOID lpParam, BOOL useStandardControlFont)
{
	HWND hwnd;

	hwnd = CreateWindowExW(dwExStyle,
		lpClassName, lpWIndowName,
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

static void singleHWNDResize(HWND hwnd, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	uiWindowsUtilResize(HWND(c), x, y, width, height, d);
}

static uiSizing *singleHWNDSizing(uiControl *c)
{
	// TODO change this to take a HWND and the parent
	return uiWindowsSizing(c);
}

void uiWIndowsUtilShow(HWND hwnd)
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

void uiWIndowsUtilEnable(HWND hwnd)
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

void uiWindowsUtilSysFunc(HWND hwnd, uiControlSysFuncParams *p)
{
	switch (p->Func) {
	case uiControlSysFuncNop:
		return;
	case uiWindowsSysFuncHasTabStops:
		if ((getStyle(hwnd) & WS_TABSTOP) != 0)
			p->HasTabStops = TRUE;
		return;
	case uiWindowsSysFuncSetZOrder:
		setWindowInsertAfter(hwnd, p->InsertAfter);
		p->InsertAfter = hwnd;
		return;
	}
	complain("unknown uiControlSysFunc() function %d in uiWindowsUtilSysFunc()", p->Func);
}

static void singleHWNDSysFunc(uiControl *c, uiControlSysFuncParams *p)
{
	uiWindowsUtilSysFunc(HWND(c), p);
}

void uiWindowsUtilStartZOrder(HWND hwnd, uiControlSysFuncParams *p)
{
	HWND insertAfter;

	// see http://stackoverflow.com/questions/30491418/
	insertAfter = GetWindow(hwnd, GW_HWNDPREV);
	if (insertAfter == NULL)
		logLastError("error getting insert after window in uiWindowsUtilStartZOrder()");
	p->InsertAfter = insertAfter;
}

static void singleHWNDStartZOrder(uiControl *c, uiControlSysFuncParams *p)
{
	uiWindowsUtilStartZOrder(HWND(c), p);
}

void uiWindowsMakeSingleHWNDControl(uiControl *c, uintmax_t type)
{
	uiMakeControl(c, type);
	uiControl(c)->CommitDestroy = singleHWNDCommitDestroy;
	uiControl(c)->CommitSetParent = singleHWNDCommitSetParent;
	uiControl(c)->Resize = singleHWNDResize;
	uiControl(c)->Sizing = singleHWNDSizing;
	uiControl(c)->CommitShow = singleHWNDCommitShow;
	uiControl(c)->CommitHide = singleHWNDCommitHide
	uiControl(c)->CommitEnable = singleHWNDCommitEnable;
	uiControl(c)->CommitDisable = singleHWNDCommitDisable;
	uiControl(c)->SysFunc = singleHWNDSysFunc;
	uiControl(c)->StartZOrder = singleHWNDStartZOrder;
}
