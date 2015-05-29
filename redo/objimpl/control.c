// 26 may 2015
#include "out/ui.h"
#include "uipriv.h"

struct controlBase {
	uiControl *parent;
	int hidden;
	int disabled;
};

#define controlBase(c) ((struct controlBase *) (TODO))

void uiControlBaseDestroy(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	if (cb->parent != NULL)
		complain("attempt to destroy uiControl %p while it has a parent", c);
	uiControlCommitDestroy(c);
	// TODO free memory
}

uiControl *uiControlBaseParent(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	return cb->parent;
}

void uiControlBaseSetParent(uiControl *c, uiControl *parent)
{
	struct controlBase *cb = controlBase(c);

	if (parent != NULL && cb->parent != NULL)
		complain("attempt to reparent uiControl %p (has parent %p, attempt to give parent %p)", c, cb->parent, parent);
	if (parent == NULL && cb->parent == NULL)
		complain("attempt to double unparent uiControl %p", c);
	cb->parent = parent;
	uiControlCommitSetParent(c, parent);
	// for situations such as where the old parent was disabled but the new one is not, etc.
	uiControlUpdateState(c);
}

int uiControlBaseContainerVisible(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	if (cb->hidden)
		return 0;
	if (cb->parent == NULL)
		return 1;
	return uiControlContainerVisible(cb->parent);
}

void uiControlBaseShow(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	cb->hidden = 0;
	uiControlUpdateState(c);
}

void uiControlBaseHide(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	cb->hidden = 1;
	uiControlUpdateState(c);
}

int uiControlBaseContainerEnabled(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	if (cb->disabled)
		return 0;
	if (cb->parent == NULL)
		return 1;
	return uiControlContainerEnabled(cb->parent);
}

void uiControlBaseEnable(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	cb->disabled = 0;
	uiControlUpdateState(c);
}

void uiControlBaseDisable(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	cb->disabled = 1;
	uiControlUpdateState(c);
}

void uiControlBaseUpdateState(uiControl *c)
{
	if (uiControlContainerVisible(c))
		uiControlCommitShow(c);
	else
		uiControlCommitHide(c);
	if (uiControlContainerEnabled(c))
		uiControlCommitEnable(c);
	else
		uiControlCommitDisable(c);
	uiControlContainerUpdateState(c);
}

void uiControlBaseContainerUpdateState(uiControl *c)
{
	// by default not a container; do nothing
}
