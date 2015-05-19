// 6 april 2015
#include "uipriv_windows.h"

struct singleControl {
	void *internal;
	uiControl *parent;
	int userHidden;
	int containerHidden;
	int userDisabled;
	int containerDisabled;
};

static void singleDestroy(uiControl *c)
{
	struct singleControl *s = (struct singleControl *) (c->Internal);

	if (s->parent != NULL)
		complain("attempt to destroy a uiControl at %p while it still has a parent", c);
	osOnDestroy(s->internal);
	uiFree(s);
}

static uintptr_t singleHandle(uiControl *c)
{
	struct singleControl *s = (struct singleControl *) (c->Internal);

	return osSingleHandle(s->internal);
}

static uiControl *singleParent(uiControl *c)
{
	struct singleControl *s = (struct singleControl *) (c->Internal);

	return s->parent;
}

static void singleSetParent(uiControl *c, uiControl *parent)
{
	struct singleControl *s = (struct singleControl *) (c->Internal);
	uiControl *oldparent;

	oldparent = s->parent;
	s->parent = parent;
	osSingleSetParent(s->internal, oldparent, s->parent);
}

static void singleResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	struct singleControl *s = (struct singleControl *) (c->Internal);

	osSingleResize(s->internal, x, y, width, height, d);
}

static void singleQueueResize(uiControl *c)
{
	queueResize(c);
}

static uiSizing *singleSizing(uiControl *c)
{
	struct singleControl *s = (struct singleControl *) (c->Internal);

	return osSingleSizing(s->internal, c);
}

static int singleContainerVisible(uiControl *c)
{
	struct singleControl *s = (struct singleControl *) (c->Internal);

	return !s->userHidden && !s->containerHidden;
}

static void singleShow(uiControl *c)
{
	struct singleControl *s = (struct singleControl *) (c->Internal);

	s->userHidden = 0;
	if (!s->containerHidden)
		osSingleShow(s->internal);
	if (s->parent != NULL)
		uiControlQueueResize(s->parent);
}

static void singleHide(uiControl *c)
{
	struct singleControl *s = (struct singleControl *) (c->Internal);

	s->userHidden = 1;
	osSingleHide(s->internal);
	if (s->parent != NULL)
		uiControlQueueResize(s->parent);
}

static void singleContainerShow(uiControl *c)
{
	struct singleControl *s = (struct singleControl *) (c->Internal);

	s->containerHidden = 0;
	if (!s->userHidden)
		osSingleShow(s->internal);
	if (s->parent != NULL)
		uiControlQueueResize(s->parent);
}

static void singleContainerHide(uiControl *c)
{
	struct singleControl *s = (struct singleControl *) (c->Internal);

	s->containerHidden = 1;
	osSingleHide(s->internal);
	if (s->parent != NULL)
		uiControlQueueResize(s->parent);
}

static void singleEnable(uiControl *c)
{
	struct singleControl *s = (struct singleControl *) (c->Internal);

	s->userDisabled = 0;
	if (!s->containerDisabled)
		osSingleEnable(s->internal);
}

static void singleDisable(uiControl *c)
{
	struct singleControl *s = (struct singleControl *) (c->Internal);

	s->userDisabled = 1;
	osSingleDisable(s->internal);
}

static void singleContainerEnable(uiControl *c)
{
	struct singleControl *s = (struct singleControl *) (c->Internal);

	s->containerDisabled = 0;
	if (!s->userDisabled)
		osSingleEnable(s->internal);
}

static void singleContainerDisable(uiControl *c)
{
	struct singleControl *s = (struct singleControl *) (c->Internal);

	s->containerDisabled = 1;
	osSingleDisable(s->internal);
}

void makeControl(uiControl *c, void *internal)
{
	struct singleControl *s;

	s = uiNew(struct singleControl);

	s->internal = internal;

	uiControl(c)->Internal = s;
	uiControl(c)->Destroy = singleDestroy;
	uiControl(c)->Handle = singleHandle;
	uiControl(c)->Parent = singleParent;
	uiControl(c)->SetParent = singleSetParent;
	// PreferredSize() implemented by the individual controls
	uiControl(c)->Resize = singleResize;
	uiControl(c)->QueueResize = singleQueueResize;
	uiControl(c)->Sizing = singleSizing;
	uiControl(c)->ContainerVisible = singleContainerVisible;
	uiControl(c)->Show = singleShow;
	uiControl(c)->Hide = singleHide;
	uiControl(c)->ContainerShow = singleContainerShow;
	uiControl(c)->ContainerHide = singleContainerHide;
	uiControl(c)->Enable = singleEnable;
	uiControl(c)->Disable = singleDisable;
	uiControl(c)->ContainerEnable = singleContainerEnable;
	uiControl(c)->ContainerDisable = singleContainerDisable;
}
