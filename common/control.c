// 26 may 2015
#include "../ui.h"
#include "uipriv.h"

void uiControlDestroy(uiControl *c)
{
	(*(c->Destroy))(c);
}

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

#define uiControlSignature 0x7569436F

uiControl *uiAllocControl(size_t size, uint32_t OSsig, uint32_t typesig, const char *typenamestr)
{
	uiControl *c;

	c = (uiControl *) uiAlloc(size, typenamestr);
	c->Signature = uiControlSignature;
	c->OSSignature = OSsig;
	c->TypeSignature = typesig;
	return c;
}

void uiFreeControl(uiControl *c)
{
	uiFree(c);
}

// TODO except where noted, replace complain() with userbug()

void uiControlVerifyDestroy(uiControl *c)
{
	if (uiControlParent(c) != NULL)
		complain("attempt to destroy uiControl %p while it has a parent", c);
}

void uiControlVerifySetParent(uiControl *c, uiControl *parent)
{
	uiControl *curParent;

	if (uiControlToplevel(c))
		complain("cannot set a parent on a toplevel (uiWindow)");
	curParent = uiControlParent(c);
	if (parent != NULL && curParent != NULL)
		complain("attempt to reparent uiControl %p (has parent %p, attempt to give parent %p)", c, curParent, parent);
	if (parent == NULL && curParent == NULL)
		// TODO implbug()
		complain("attempt to double unparent uiControl %p — likely an implementation bug ", c);
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
