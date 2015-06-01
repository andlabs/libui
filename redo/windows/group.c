// 16 may 2015
#include "uipriv_windows.h"

struct group {
	uiGroup g;
	HWND hwnd;
	uiControl *child;
	void (*baseCommitDestroy)(uiControl *);
};

uiDefineControlType(uiGroup, uiTypeGroup, struct group)

static void groupCommitDestroy(uiControl *c)
{
	struct group *g = (struct group *) c;

	if (g->child != NULL) {
		uiControlSetParent(g->child, NULL);
		uiControlDestroy(g->child);
	}
	(*(g->baseCommitDestroy))(uiControl(g));
}

static uintptr_t groupHandle(uiControl *c)
{
	struct group *g = (struct group *) c;

	return (uintptr_t) (g->hwnd);
}

static void groupPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	// TODO
	*width = 0;
	*height = 0;
}

static void groupContainerUpdateState(uiControl *c)
{
	struct group *g = (struct group *) c;

	if (g->child != NULL)
		uiControlUpdateState(g->child);
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

uiGroup *uiNewGroup(const char *text)
{
	struct group *g;
	WCHAR *wtext;

	g = (struct group *) uiWindowsNewSingleHWNDControl(uiTypeGroup());

	wtext = toUTF16(text);
	g->hwnd = uiWindowsUtilCreateControlHWND(WS_EX_CONTROLPARENT,
		L"button", wtext,
		BS_GROUPBOX,
		hInstance, NULL,
		TRUE);
	uiFree(wtext);

	uiControl(g)->Handle = groupHandle;
	uiControl(g)->PreferredSize = groupPreferredSize;
	g->baseCommitDestroy = uiControl(g)->CommitDestroy;
	uiControl(g)->CommitDestroy = groupCommitDestroy;
	uiControl(g)->ContainerUpdateState = groupContainerUpdateState;

	uiGroup(g)->SetChild = groupSetChild;

	return uiGroup(g);
}
