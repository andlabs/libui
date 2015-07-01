// 11 june 2015
#include "uipriv_unix.h"

struct tab {
	uiTab t;

	GtkWidget *widget;
	GtkContainer *container;
	GtkNotebook *notebook;

	GArray *pages;
};

struct tabPage {
	uiControl *bin;
	GtkWidget *binWidget;
	uiControl *c;
};

uiDefineControlType(uiTab, uiTypeTab, struct tab)

static uintptr_t tabHandle(uiControl *c)
{
	struct tab *t = (struct tab *) c;

	return (uintptr_t) (t->widget);
}

static void tabAppend(uiTab *tt, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) tt;

	uiTabInsertAt(tt, name, t->pages->len, child);
}

static void tabInsertAt(uiTab *tt, const char *name, uintmax_t n, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	struct tabPage page;

	page.c = child;
	page.bin = newBin();
	page.binWidget = GTK_WIDGET(uiControlHandle(page.bin));
	binSetChild(page.bin, page.c);

	uiControlSetParent(page.bin, uiControl(t));
	gtk_notebook_set_tab_label_text(t->notebook, page.binWidget, name);
	gtk_notebook_reorder_child(t->notebook, page.binWidget, n);

	g_array_insert_val(t->pages, n, page);
}

static void tabDelete(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;

	PUT_CODE_HERE;
}

static uintmax_t tabNumPages(uiTab *tt)
{
	struct tab *t = (struct tab *) tt;

	return t->pages->len;
}

static int tabMargined(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;
	struct tabPage *page;

	page = &g_array_index(t->pages, struct tabPage, n);
	return binMargined(page->bin);
}

static void tabSetMargined(uiTab *tt, uintmax_t n, int margined)
{
	struct tab *t = (struct tab *) tt;
	struct tabPage *page;

	page = &g_array_index(t->pages, struct tabPage, n);
	binSetMargined(page->bin, margined);
}

uiTab *uiNewTab(void)
{
	struct tab *t;

	t = (struct tab *) uiNewControl(uiTypeTab());

	t->widget = gtk_notebook_new();
	t->container = GTK_CONTAINER(t->widget);
	t->notebook = GTK_NOTEBOOK(t->widget);
	uiUnixMakeSingleWidgetControl(uiControl(t), t->widget);

	gtk_notebook_set_scrollable(t->notebook, TRUE);

	t->pages = g_array_new(FALSE, TRUE, sizeof (struct tabPage));

	uiControl(t)->Handle = tabHandle;

	uiTab(t)->Append = tabAppend;
	uiTab(t)->InsertAt = tabInsertAt;
	uiTab(t)->Delete = tabDelete;
	uiTab(t)->NumPages = tabNumPages;
	uiTab(t)->Margined = tabMargined;
	uiTab(t)->SetMargined = tabSetMargined;

	return uiTab(t);
}
