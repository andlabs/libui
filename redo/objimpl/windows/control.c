// 27 may 2015
#include "uipriv_windows.h"

// TODO Edit ,s/Util/HWND/g

#define HWND(c) ((HWND) uiControlHandle((c)))

void uiWindowsUtilDestroy(HWND hwnd)
{
	if (DestroyWindow(hwnd) == 0)
		logLastError("error destroying window in uiWindowsUtilDestroyWindow()");
}

void uiWindowsSingleHWNDControlCommitDestroy(uiControl *c)
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

void uiWindowsSingleHWNDControlCommitSetParent(uiControl *c, uiControl *parent)
{
	uiWindowsUtilSetParent(HWND(c), parent);
}

// TODO resizing

void uiWIndowsUtilShow(HWND hwnd)
{
	ShowWindow(hwnd, SW_SHOW);
}

void uiWindowsSingleHWNDControlCommitShow(uiControl *c)
{
	uiWindowsUtilShow(HWND(c));
}

void uiWindowsUtilHide(HWND hwnd)
{
	ShowWindow(hwnd, SW_HIDE);
}

void uiWindowsSingleHWNDControlCommitHide(uiControl *c)
{
	uiWindowsUtilHide(HWND(c));
}

void uiWIndowsUtilEnable(HWND hwnd)
{
	EnableWindow(hwnd, TRUE);
}

void uiWindowsSingleHWNDControlCommitEnable(uiControl *c)
{
	uiWindowsUtilEnable(HWND(c));
}

void uiWindowsUtilDisable(HWND hwnd)
{
	EnableWindow(hwnd, FALSE);
}

void uiWindowsSingleHWNDControlCommitDisable(uiControl *c)
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

void uiWindowsSingleHWNDControlSysFunc(uiControl *c, uiControlSysFuncParams *p)
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

void uiWindowsSingleHWNDControlStartZOrder(uiControl *c, uiControlSysFuncParams *p)
{
	uiWindowsUtilStartZOrder(HWND(c), p);
}
