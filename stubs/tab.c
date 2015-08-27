// 11 june 2015
#include "uipriv_OSHERE.h"

struct tab {
	uiTab t;
	OSTYPE OSHANDLE;
};

uiDefineControlType(uiTab, uiTypeTab, struct tab)

static uintptr_t tabHandle(uiControl *c)
{
	struct tab *t = (struct tab *) c;

	return (uintptr_t) (t->OSHANDLE);
}

static void tabAppend(uiTab *tt, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) tt;

	uiTabInsertAt(tt, name, PUT_CODE_HERE, child);
}

static void tabInsertAt(uiTab *tt, const char *name, uintmax_t n, uiControl *child)
{
	struct tab *t = (struct tab *) tt;

	PUT_CODE_HERE;
}

static void tabDelete(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;

	PUT_CODE_HERE;
}

static uintmax_t tabNumPages(uiTab *tt)
{
	struct tab *t = (struct tab *) tt;

	return PUT_CODE_HERE;
}

static int tabMargined(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;

	return PUT_CODE_HERE;
}

static void tabSetMargined(uiTab *tt, uintmax_t n, int margined)
{
	struct tab *t = (struct tab *) tt;

	PUT_CODE_HERE;
}

uiTab *uiNewTab(void)
{
	struct tab *t;

	t = (struct tab *) MAKE_CONTROL_INSTANCE(uiTypeTab());

	PUT_CODE_HERE;

	uiControl(t)->Handle = tabHandle;

	uiTab(t)->Append = tabAppend;
	uiTab(t)->InsertAt = tabInsertAt;
	uiTab(t)->Delete = tabDelete;
	uiTab(t)->NumPages = tabNumPages;
	uiTab(t)->Margined = tabMargined;
	uiTab(t)->SetMargined = tabSetMargined;

	return uiTab(t);
}
