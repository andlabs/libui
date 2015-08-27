// 11 june 2015
#include "uipriv_unix.h"

struct tab {
	uiTab t;

	GtkWidget *widget;
	GtkContainer *container;
	GtkNotebook *notebook;

	GArray *pages;

	void (*baseCommitDestroy)(uiControl *);
};

struct tabPage {
	uiControl *bin;
	GtkWidget *binWidget;
	uiControl *c;
};

uiDefineControlType(uiTab, uiTypeTab, struct tab)

static void tabCommitDestroy(uiControl *c)
{
	struct tab *t = (struct tab *) c;
	guint i;
	struct tabPage *page;

	// the pages do not have a libui parent, so we can simply destroy them
	// we need to remove them from the tab first; see below
	for (i = 0; i < t->pages->len; i++) {
		page = &g_array_index(t->pages, struct tabPage, i);
		binSetChild(page->bin, NULL);
		uiControlDestroy(page->c);
		uiControlSetParent(page->bin, NULL);
		uiControlDestroy(page->bin);
	}
	// then free ourselves
	g_array_free(t->pages, TRUE);
	(*(t->baseCommitDestroy))(uiControl(t));
}

static uintptr_t tabHandle(uiControl *c)
{
	struct tab *t = (struct tab *) c;

	return (uintptr_t) (t->widget);
}

// TODO tabContainerUpdateState()?

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
	struct tabPage *page;

	page = &g_array_index(t->pages, struct tabPage, n);

	// make sure the page's control isn't destroyed
	binSetChild(page->bin, NULL);

	// now destroy the page
	// this will also remove the tab
	// why? simple: both gtk_notebook_remove_tab() and gtk_widget_destroy() call gtk_container_remove()
	// we need to remove them from the tab first, though, otherwise they won't really be destroyed properly
	// (the GtkNotebook will still have the tab in it because its reference ISN'T destroyed, and we crash resizing a bin that no longer exists)
	// TODO redo this comment
	uiControlSetParent(page->bin, NULL);
	uiControlDestroy(page->bin);

	g_array_remove_index(t->pages, n);
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
	t->baseCommitDestroy = uiControl(t)->CommitDestroy;
	uiControl(t)->CommitDestroy = tabCommitDestroy;

	uiTab(t)->Append = tabAppend;
	uiTab(t)->InsertAt = tabInsertAt;
	uiTab(t)->Delete = tabDelete;
	uiTab(t)->NumPages = tabNumPages;
	uiTab(t)->Margined = tabMargined;
	uiTab(t)->SetMargined = tabSetMargined;

	return uiTab(t);
}
