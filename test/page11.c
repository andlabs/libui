// 14 may 2016
#include "test.h"

// TODO add a test for childless windows

static uiGroup *newg(const char *n, int s)
{
	uiGroup *g;

	g = uiNewGroup(n);
	if (s)
		uiGroupSetChild(g, NULL);
	return g;
}

static uiTab *newt(int tt)
{
	uiTab *t;

	t = uiNewTab();
	if (tt)
		uiTabAppend(t, "Test", NULL);
	return t;
}

uiBox *makePage11(void)
{
	uiBox *page11;
	uiBox *ns;
	uiBox *s;

	page11 = newHorizontalBox();

	ns = newVerticalBox();
	uiBoxAppend(ns, uiControl(newg("", 0)), 0);
	uiBoxAppend(ns, uiControl(newg("", 1)), 0);
	uiBoxAppend(ns, uiControl(newg("Group", 0)), 0);
	uiBoxAppend(ns, uiControl(newg("Group", 1)), 0);
	uiBoxAppend(ns, uiControl(newt(0)), 0);
	uiBoxAppend(ns, uiControl(newt(1)), 0);
	uiBoxAppend(page11, uiControl(ns), 1);

	s = newVerticalBox();
	uiBoxAppend(s, uiControl(newg("", 0)), 1);
	uiBoxAppend(s, uiControl(newg("", 1)), 1);
	uiBoxAppend(s, uiControl(newg("Group", 0)), 1);
	uiBoxAppend(s, uiControl(newg("Group", 1)), 1);
	uiBoxAppend(s, uiControl(newt(0)), 1);
	uiBoxAppend(s, uiControl(newt(1)), 1);
	uiBoxAppend(page11, uiControl(s), 1);

	return page11;
}
