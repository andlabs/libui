// 16 august 2015
#include "uipriv_haiku.hpp"

static uintmax_t type_uiHaikuControl = 0;

uintmax_t uiHaikuControlType(void)
{
	if (type_uiHaikuControl == 0)
		type_uiHaikuControl = uiRegisterType("uiHaikuControl", uiControlType(), sizeof (uiHaikuControl));
	return type_uiHaikuControl;
}

static void defaultCommitShow(uiControl *c)
{
	BView *view;

	view = (BView *) uiControlHandle(c);
	view->Show();
}

static void defaultCommitHide(uiControl *c)
{
	BView *view;

	view = (BView *) uiControlHandle(c);
	view->Hide();
}

void osCommitEnable(uiControl *c)
{
	// TODO this might need to be per-widget
}

void osCommitDisable(uiControl *c)
{
	// TODO this might need to be per-widget
}

void uiHaikuFinishControl(uiControl *c)
{
	c->CommitShow = defaultCommitShow;
	c->CommitHide = defaultCommitHide;
}
