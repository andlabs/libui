// 16 august 2015
#include "uipriv_windows.h"

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
