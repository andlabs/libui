// 18 november 2015
#include "uipriv_wpf.hpp"

struct uiGroup {
	uiWindowsControl c;
	gcroot<GroupBox ^> *groupbox;
	int margined;
};

uiWindowsDefineControl(
	uiGroup,								// type name
	uiGroupType,							// type function
	groupbox								// handle
)

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
	// TODO
}

int uiGroupMargined(uiGroup *g)
{
	return g->margined;
}

void uiGroupSetMargined(uiGroup *g, int margined)
{
	// TODO
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
