// 26 may 2015
#include "../ui.h"
#include "uipriv.h"

struct controlBase {
	uiControl *parent;
	int hidden;
	int disabled;
};

static uintmax_t type_uiControl = 0;

uintmax_t uiControlType(void)
{
	if (type_uiControl == 0)
		type_uiControl = uiRegisterType("uiControl", 0, sizeof (uiControl));
	return type_uiControl;
}

#define controlBase(c) ((struct controlBase *) (c->Internal))

void uiControlDestroy(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	if (cb->parent != NULL)
		complain("attempt to destroy uiControl %p while it has a parent", c);
	(*(c->CommitDestroy))(c);
	uiFree(cb);
	uiFree(c);
}

uintptr_t uiControlHandle(uiControl *c)
{
	return (*(c->Handle))(c);
}

uiControl *uiControlParent(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	return cb->parent;
}

int isToplevel(uiControl *c)
{
	return uiIsA(c, uiWindowType(), 0) != NULL;
}

// returns self if self is a window
uiControl *toplevelOwning(uiControl *c)
{
	struct controlBase *cb;

	for (;;) {
		if (isToplevel(c))
			return c;
		cb = controlBase(c);
		if (cb->parent == NULL)
			return NULL;
		c = cb->parent;
	}
}

void uiControlSetParent(uiControl *c, uiControl *parent)
{
	struct controlBase *cb = controlBase(c);

	if (isToplevel(c))
		complain("cannot set a parent on a toplevel (uiWindow)");
	if (parent != NULL && cb->parent != NULL)
		complain("attempt to reparent uiControl %p (has parent %p, attempt to give parent %p)", c, cb->parent, parent);
	if (parent == NULL && cb->parent == NULL)
		complain("attempt to double unparent uiControl %p", c);
	cb->parent = parent;
	// for situations such as where the old parent was disabled but the new one is not, etc.
	controlUpdateState(c);
}

// only to be called by the immediate parent of a control
int controlSelfVisible(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	return !cb->hidden;
}

static int controlContainerVisible(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	if (cb->hidden)
		return 0;
	if (cb->parent == NULL)
		return 1;
	return controlContainerVisible(cb->parent);
}

void uiControlShow(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	cb->hidden = 0;
	controlUpdateState(c);
}

void uiControlHide(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	cb->hidden = 1;
	controlUpdateState(c);
}

static int controlContainerEnabled(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	if (cb->disabled)
		return 0;
	if (cb->parent == NULL)
		return 1;
	return controlContainerEnabled(cb->parent);
}

void uiControlEnable(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	cb->disabled = 0;
	controlUpdateState(c);
}

void uiControlDisable(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	cb->disabled = 1;
	controlUpdateState(c);
}

void controlUpdateState(uiControl *c)
{
	if (controlContainerVisible(c))
		(*(c->CommitShow))(c);
	else
		(*(c->CommitHide))(c);
	if (controlContainerEnabled(c))
		osCommitEnable(c);
	else
		osCommitDisable(c);
	(*(c->ContainerUpdateState))(c);
	// and queue a resize, just in case we showed/hid something
	// for instance, on Windows uiBox, if we don't do this, hiding a control will show empty space until the window is resized
//TODO	uiControlQueueResize(c);
}

uiControl *uiNewControl(uintmax_t type)
{
	uiControl *c;

	c = uiControl(newTyped(type));
	c->Internal = uiNew(struct controlBase);
	return c;
}
