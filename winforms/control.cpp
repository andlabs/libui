// 16 august 2015
#include "uipriv_winforms.hpp"

static uintmax_t type_uiWindowsControl = 0;

uintmax_t uiWindowsControlType(void)
{
	if (type_uiWindowsControl == 0)
		type_uiWindowsControl = uiRegisterType("uiWindowsControl", uiControlType(), sizeof (uiWindowsControl));
	return type_uiWindowsControl;
}

Control ^genericHandle(uiControl *c)
{
	gcroot<Control ^> *h;

	h = (gcroot<Control ^> *) uiControlHandle(c);
	return *h;
}

static void defaultCommitShow(uiControl *c)
{
	genericHandle(c)->Visibility = Visibility::Visible;
}

static void defaultCommitHide(uiControl *c)
{
	// TODO formally document this behavior (it's how GTK+ works)
	genericHandle(c)->Visibility = Visibility::Collapsed;
}

void osCommitEnable(uiControl *c)
{
	genericHandle(c)->IsEnabled = true;
}

void osCommitDisable(uiControl *c)
{
	genericHandle(c)->IsEnabled = false;
}

void uiWindowsFinishControl(uiControl *c)
{
	c->CommitShow = defaultCommitShow;
	c->CommitHide = defaultCommitHide;
}
