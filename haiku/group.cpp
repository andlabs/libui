// 18 november 2015
#include "uipriv_haiku.hpp"

struct uiGroup {
	uiHaikuControl c;
	BStringView *dummy;
};

uiHaikuDefineControl(
	uiGroup,								// type name
	uiGroupType,							// type function
	dummy								// handle
)

char *uiGroupTitle(uiGroup *g)
{
	// TODO
	return NULL;
}

void uiGroupSetTitle(uiGroup *g, const char *title)
{
	// TODO
}

void uiGroupSetChild(uiGroup *g, uiControl *c)
{
	// TODO
}

int uiGroupMargined(uiGroup *g)
{
	// TODO
	return 0;
}

void uiGroupSetMargined(uiGroup *g, int margined)
{
	// TODO
}

uiGroup *uiNewGroup(const char *title)
{
	uiGroup *g;

	g = (uiGroup *) uiNewControl(uiGroupType());

	g->dummy = new BStringView(BRect(0, 0, 1, 1), NULL,
		"TODO uiGroup not implemented");

	uiHaikuFinishNewControl(g, uiGroup);

	return g;
}
