// 18 november 2015
#include <vector>
#include "uipriv_haiku.hpp"
using namespace std;

struct tabPage {
	BTab *tab;
	BView *view;
	struct singleChild *child;
};

struct uiTab {
	uiHaikuControl c;
	BTabView *tabview;
	vector<struct tabPage> *pages;
};

static void onDestroy(uiTab *);

uiHaikuDefineControlWithOnDestroy(
	uiTab,								// type name
	uiTabType,							// type function
	tabview,								// handle
	onDestroy(hthis);						// on destroy
)

static void onDestroy(uiTab *t)
{
	// TODO
}

void uiTabAppend(uiTab *t, const char *name, uiControl *c)
{
	uiTabInsertAt(t, name, t->pages->size(), c);
}

// see singlechild.cpp
static void attach(void *attachTo, BLayoutItem *what)
{
	BView *view = (BView *) attachTo;

	// TODO refine the interface around this
	view->SetLayout((BLayout *) what);
}

void uiTabInsertAt(uiTab *t, const char *name, uintmax_t before, uiControl *c)
{
	struct tabPage p;

	p.view = new BView(NULL, B_SUPPORTS_LAYOUT);
	// TODO needed?
	p.view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	p.tab = new BTab(p.view);
	p.child = newSingleChild(c, uiControl(t), attach, p.view);

	p.tab->SetLabel(name);

	// TODO insert in the correct place
	t->tabview->AddTab(p.view, p.tab);
	t->pages->push_back(p);
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

	t->tabview = new BTabView(NULL, B_WIDTH_FROM_LABEL);
	// TODO scrollable

	t->pages = new vector<struct tabPage>();

	uiHaikuFinishNewControl(t, uiTab);

	return t;
}
