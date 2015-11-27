// 18 november 2015
#include "uipriv_winforms.hpp"

struct uiGroup {
	uiWindowsControl c;
	gcroot<GroupBox ^> *groupbox;
	uiControl *child;
	int margined;
};

static void onDestroy(uiGroup *);

uiWindowsDefineControlWithOnDestroy(
	uiGroup,								// type name
	uiGroupType,							// type function
	groupbox,							// handle
	onDestroy(hthis);						// on destroy
)

static void onDestroy(uiGroup *g)
{
	if (g->child != NULL) {
		(*(g->groupbox))->Content = nullptr;
		uiControlSetParent(g->child, NULL);
		uiControlDestroy(g->child);
	}
}

char *uiGroupTitle(uiGroup *g)
{
	String ^text;

	// TOOD bad cast?
	text = (String ^) ((*(g->groupbox))->Header);
	return uiWindowsCLRStringToText(text);
}

void uiGroupSetTitle(uiGroup *g, const char *title)
{
	(*(g->groupbox))->Header = fromUTF8(title);
	// TODO layout
}

void uiGroupSetChild(uiGroup *g, uiControl *c)
{
	if (g->child != NULL) {
		uiControlSetParent(g->child, NULL);
		(*(g->groupbox))->Content = nullptr;
	}
	g->child = c;
	if (g->child != NULL) {
		(*(g->groupbox))->Content = genericHandle(g->child);
		uiControlSetParent(g->child, uiControl(g));
	}
}

int uiGroupMargined(uiGroup *g)
{
	return g->margined;
}

void uiGroupSetMargined(uiGroup *g, int margined)
{
	g->margined = margined;
	// TODO Margin or Padding?
	// TODO really this? or should we just use another Border?
	if (g->margined)
		(*(g->groupbox))->Padding = Thickness(10, 10, 10, 10);
	else
		(*(g->groupbox))->Padding = Thickness(0, 0, 0, 0);
}

uiGroup *uiNewGroup(const char *title)
{
	uiGroup *g;

	g = (uiGroup *) uiNewControl(uiGroupType());

	g->groupbox = new gcroot<GroupBox ^>();
	*(g->groupbox) = gcnew GroupBox();
	(*(g->groupbox))->Header = fromUTF8(title);

	uiWindowsFinishNewControl(g, uiGroup, groupbox);

	return g;
}
