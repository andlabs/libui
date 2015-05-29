// 26 may 2015
#include "out/ui.h"
#include "uipriv.h"

struct controlBase {
	uiControl *parent;
	int hidden;
	int disabled;
};

#define controlBase(c) ((struct controlBase *) (c->Internal))

static void controlBaseDestroy(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	if (cb->parent != NULL)
		complain("attempt to destroy uiControl %p while it has a parent", c);
	uiControlCommitDestroy(c);
	uiFree(cb);
	// TODO free c?
}

static uiControl *controlBaseParent(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	return cb->parent;
}

static void controlBaseSetParent(uiControl *c, uiControl *parent)
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

static int controlBaseContainerVisible(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	if (cb->hidden)
		return 0;
	if (cb->parent == NULL)
		return 1;
	return uiControlContainerVisible(cb->parent);
}

static void controlBaseShow(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	cb->hidden = 0;
	uiControlUpdateState(c);
}

static void controlBaseHide(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	cb->hidden = 1;
	uiControlUpdateState(c);
}

static int controlBaseContainerEnabled(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	if (cb->disabled)
		return 0;
	if (cb->parent == NULL)
		return 1;
	return uiControlContainerEnabled(cb->parent);
}

static void controlBaseEnable(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	cb->disabled = 0;
	uiControlUpdateState(c);
}

static void controlBaseDisable(uiControl *c)
{
	struct controlBase *cb = controlBase(c);

	cb->disabled = 1;
	uiControlUpdateState(c);
}

static void controlBaseUpdateState(uiControl *c)
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

static void controlBaseContainerUpdateState(uiControl *c)
{
	// by default not a container; do nothing
}

void uiMakeControl(uiControl *c, uintmax_t type)
{
	uiTyped(c)->Type = type;
	uiControl(c)->Internal = uiNew(struct controlBase);
	uiControl(c)->Destroy = controlBaseDestroy;
	uiControl(c)->Parent = controlBaseParent;
	uiControl(c)->SetParent = controlBaseSetParent;
	uiControl(c)->ContainerVisible = controlBaseContainerVisible;
	uiControl(c)->Show = controlBaseShow;
	uiControl(c)->Hide = controlBaseHide;
	uiControl(c)->ContainerEnabled = controlBaseContainerEnabled;
	uiControl(c)->Enable = controlBaseEnable;
	uiControl(c)->Disable = controlBaseDisable;
	uiControl(c)->UpdateState = controlBaseUpdateState;
	uiControl(c)->ContainerUpdateState = controlBaseContainerUpdateState;
}
