// 18 november 2015
#include "uipriv_haiku.hpp"

struct uiTab {
	uiHaikuControl c;
	BStringView *dummy;
};

uiHaikuDefineControl(
	uiTab,								// type name
	uiTabType,							// type function
	dummy								// handle
)

void uiTabAppend(uiTab *t, const char *name, uiControl *c)
{
	// TODO
}

void uiTabInsertAt(uiTab *t, const char *name, uintmax_t before, uiControl *c)
{
	// TODO
}

void uiTabDelete(uiTab *t, uintmax_t index)
{
	// TODO
}

uintmax_t uiTabNumPages(uiTab *t)
{
	// TODO
	return 0;
}

int uiTabMargined(uiTab *t, uintmax_t page)
{
	// TODO
	return 0;
}

void uiTabSetMargined(uiTab *t, uintmax_t page, int margined)
{
	// TODO
}

uiTab *uiNewTab(void)
{
	uiTab *t;

	t = (uiTab *) uiNewControl(uiTabType());

	t->dummy = new BStringView(BRect(0, 0, 1, 1), NULL,
		"TODO uiTab not implemented");

	uiHaikuFinishNewControl(t, uiTab);

	return t;
}
