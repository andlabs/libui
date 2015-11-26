// 26 november 2015
#include "uipriv_wpf.hpp"

// TODO save child alignments?
// TODO scroll tabs instead of multiline tabs

using namespace System::Collections::Generic;

ref class tabPage {
public:
	TabItem ^item;
	uiControl *c;
	Border ^border;		// see uiTabSetMargined()
	int margined;
};

struct uiTab {
	uiWindowsControl c;
	gcroot<TabControl ^> *tab;
	gcroot<List<tabPage ^> ^> *pages;
};

static void onDestroy(uiTab *);

uiWindowsDefineControlWithOnDestroy(
	uiTab,								// type name
	uiTabType,							// type function
	tab,									// handle
	onDestroy(hthis);						// on destroy
)

static void onDestroy(uiTab *t)
{
	List<tabPage ^> ^pages;

	pages = *(t->pages);
	while (pages->Count != 0) {
		pages[0]->border->Child = nullptr;
		uiControlSetParent(pages[0]->c, NULL);
		uiControlDestroy(pages[0]->c);
		pages->RemoveAt(0);
		(*(t->tab))->Items->RemoveAt(0);
	}
	delete t->pages;
}

void uiTabAppend(uiTab *t, const char *name, uiControl *c)
{
	uiTabInsertAt(t, name, (*(t->pages))->Count, c);
}

void uiTabInsertAt(uiTab *t, const char *name, uintmax_t before, uiControl *c)
{
	tabPage ^p;

	p = gcnew tabPage();
	p->c = c;
	p->border = gcnew Border();
	p->item = gcnew TabItem();

	p->border->Child = genericHandle(p->c);
	p->item->Content = p->border;
	p->item->Header = fromUTF8(name);

	uiControlSetParent(p->c, uiControl(t));
	(*(t->tab))->Items->Insert(before, p->item);
	(*(t->pages))->Insert(before, p);
	uiControlSetParent(p->c, NULL);
}

void uiTabDelete(uiTab *t, uintmax_t index)
{
	tabPage ^p;
	List<tabPage ^> ^pages;

	pages = (*(t->pages));
	p = pages[index];
	pages->RemoveAt(index);
	(*(t->tab))->Items->RemoveAt(index);
	p->border->Child = nullptr;
}

uintmax_t uiTabNumPages(uiTab *t)
{
	return (*(t->pages))->Count;
}

int uiTabMargined(uiTab *t, uintmax_t page)
{
	List<tabPage ^> ^pages;

	pages = (*(t->pages));
	return pages[page]->margined;
}

void uiTabSetMargined(uiTab *t, uintmax_t page, int margined)
{
	List<tabPage ^> ^pages;

	pages = (*(t->pages));
	pages[page]->margined = margined;
	// TabItem margins/padding do NOT work the way we want them to
	// we have to use a Border here too
	// TODO Padding?
	if (pages[page]->margined)
		pages[page]->border->Margin = Thickness(10, 10, 10, 10);
	else
		pages[page]->border->Margin = Thickness(0, 0, 0, 0);
}

uiTab *uiNewTab(void)
{
	uiTab *t;

	t = (uiTab *) uiNewControl(uiTabType());

	t->tab = new gcroot<TabControl ^>();
	*(t->tab) = gcnew TabControl();

	t->pages = new gcroot<List<tabPage ^> ^>();
	*(t->pages) = gcnew List<tabPage ^>();

	uiWindowsFinishNewControl(t, uiTab, tab);

	return t;
}
