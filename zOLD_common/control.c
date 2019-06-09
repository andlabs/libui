// 26 may 2015
#include "../ui.h"
#include "uipriv.h"

uintptr_t uiControlHandle(uiControl *c)
{
	return (*(c->Handle))(c);
}

uiControl *uiControlParent(uiControl *c)
{
	return (*(c->Parent))(c);
}

void uiControlSetParent(uiControl *c, uiControl *parent)
{
	(*(c->SetParent))(c, parent);
}

int uiControlToplevel(uiControl *c)
{
	return (*(c->Toplevel))(c);
}

int uiControlVisible(uiControl *c)
{
	return (*(c->Visible))(c);
}

void uiControlShow(uiControl *c)
{
	(*(c->Show))(c);
}

void uiControlHide(uiControl *c)
{
	(*(c->Hide))(c);
}

int uiControlEnabled(uiControl *c)
{
	return (*(c->Enabled))(c);
}

void uiControlEnable(uiControl *c)
{
	(*(c->Enable))(c);
}

void uiControlDisable(uiControl *c)
{
	(*(c->Disable))(c);
}

void uiControlVerifySetParent(uiControl *c, uiControl *parent)
{
	uiControl *curParent;

	if (uiControlToplevel(c))
		uiprivUserBug("You cannot give a toplevel uiControl a parent. (control: %p)", c);
	curParent = uiControlParent(c);
	if (parent != NULL && curParent != NULL)
		uiprivUserBug("You cannot give a uiControl a parent while it already has one. (control: %p; current parent: %p; new parent: %p)", c, curParent, parent);
	if (parent == NULL && curParent == NULL)
		uiprivImplBug("attempt to double unparent uiControl %p", c);
}

int uiControlEnabledToUser(uiControl *c)
{
	while (c != NULL) {
		if (!uiControlEnabled(c))
			return 0;
		c = uiControlParent(c);
	}
	return 1;
}
