// 11 june 2015
#include "uipriv_unix.h"

struct group {
	uiGroup g;
	GtkWidget *widget;
	uiControl *child;
	int margined;
};

uiDefineControlType(uiGroup, uiTypeGroup, struct group)

static uintptr_t groupHandle(uiControl *c)
{
	struct group *g = (struct group *) c;

	return (uintptr_t) (g->widget);
}

static void groupContainerUpdateState(uiControl *c)
{
	struct group *g = (struct group *) c;

	if (g->child != NULL)
		uiControlUpdateState(g->child);
}

static char *groupTitle(uiGroup *gg)
{
	struct group *g = (struct group *) gg;

	return PUT_CODE_HERE;
}

static void groupSetTitle(uiGroup *gg, const char *text)
{
	struct group *g = (struct group *) gg;

	PUT_CODE_HERE;
	// changing the text might necessitate a change in the groupbox's size
	uiControlQueueResize(uiControl(g));
}

static void groupSetChild(uiGroup *gg, uiControl *child)
{
	struct group *g = (struct group *) gg;

	if (g->child != NULL)
		uiControlSetParent(g->child, NULL);
	g->child = child;
	if (g->child != NULL) {
		uiControlSetParent(g->child, uiControl(g));
		uiControlQueueResize(g->child);
	}
}

static int groupMargined(uiGroup *gg)
{
	struct group *g = (struct group *) gg;

	return g->margined;
}

static void groupSetMargined(uiGroup *gg, int margined)
{
	struct group *g = (struct group *) gg;

	g->margined = margined;
	uiControlQueueResize(uiControl(g));
}

uiGroup *uiNewGroup(const char *text)
{
	struct group *g;

	g = (struct group *) uiNewControl(uiTypeGroup());

	PUT_CODE_HERE;

	uiControl(g)->Handle = groupHandle;
	uiControl(g)->ContainerUpdateState = groupContainerUpdateState;

	uiGroup(g)->Title = groupTitle;
	uiGroup(g)->SetTitle = groupSetTitle;
	uiGroup(g)->SetChild = groupSetChild;
	uiGroup(g)->Margined = groupMargined;
	uiGroup(g)->SetMargined = groupSetMargined;

	return uiGroup(g);
}
