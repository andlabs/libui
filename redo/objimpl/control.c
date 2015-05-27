// 26 may 2015
#include "out/ui.h"
#include "uipriv.h"

struct controlBase {
	uiControl *parent;
	int hidden;
	int disabled;
};

#define controlBase(c) ((struct controlBase *) (TODO))

void controlBaseDestroy(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	if (cb->parent != NULL)
		complain("attempt to destroy uiControl %p while it has a parent", c);
	uiControlCommitDestroy(c);
	// TODO free memory
}

uiControl *controlBaseParent(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	return cb->parent;
}

void controlBaseSetParent(uiControl *c, uiControl *parent)
{
	struct controlBase *cb = controlBase(c);

	if (parent != NULL && cb->parent != NULL)
		complain("attempt to reparent uiControl %p (has parent %p, attempt to give parent %p)", c, cb->parent, parent);
	if (parent == NULL && cb->parent == NULL)
		complain("attempt to double unparent uiControl %p", c);
	cb->parent = parent;
	uiControlCommitSetParent(c, parent);
}

int controlBaseContainerVisible(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	if (cb->hidden)
		return 0;
	if (cb->parent == NULL)
		return 1;
	return uiControlContainerVisible(cb->parent);
}

void controlBaseShow(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	cb->hidden = 0;
	uiControlUpdateState(c);
}

void controlBaseHide(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	cb->hidden = 1;
	uiControlUpdateState(c);
}

int controlBaseContainerEnabled(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	if (cb->disabled)
		return 0;
	if (cb->parent == NULL)
		return 1;
	return uiControlContainerEnabled(cb->parent);
}

void controlBaseEnable(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	cb->disabled = 0;
	uiControlUpdateState(c);
}

void controlBaseDisable(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	cb->disabled = 1;
	uiControlUpdateState(c);
}

void controlBaseUpdateState(uiControl *c)
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

void controlBaseContainerUpdateState(uiControl *c)
{
	// by default not a contianer; do nothing
}
