// 12 april 2015
#include "uipriv_unix.h"

struct tab {
	uiTab t;
	GtkWidget *widget;
	GtkContainer *container;
	GtkNotebook *notebook;
	GArray *pages;
};

struct tabPage {
	uiBin *bin;
	// TODO remove the need for this
	GtkWidget *binWidget;
	int margined;
};

static void onDestroy(void *data)
{
	struct tab *t = (struct tab *) data;
	guint i;
	struct tabPage *page;

	// first hide ourselves to avoid flicker
	gtk_widget_hide(t->widget);
	// the pages do not have a libui parent, so we can simply destroy them
	// we need to remove them from the tab first; see below
	for (i = 0; i < t->pages->len; i++) {
		page = &g_array_index(t->pages, struct tabPage, i);
		uiBinRemoveOSParent(page->bin);
		uiControlDestroy(uiControl(page->bin));
	}
	// then free ourselves
	g_array_free(t->pages, TRUE);
	uiFree(t);
}

static void tabShow(uiControl *c)
{
	struct tab *t = (struct tab *) t;

	// don't call gtk_widget_show_all() like the default handler does; that'll override user hiding of children
	gtk_widget_show(t->widget);
}

#define tabCapGrow 32

static void tabAppendPage(uiTab *tt, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	struct tabPage page;

	page.bin = newBin();
	uiBinSetMainControl(page.bin, child);
	// and add it as a tab page
	uiBinSetOSParent(page.bin, (uintptr_t) (t->container));
	page.binWidget = GTK_WIDGET(uiControlHandle(uiControl(page.bin)));
	gtk_notebook_set_tab_label_text(t->notebook, page.binWidget, name);

	g_array_append_val(t->pages, page);
}

static void tabInsertPageBefore(uiTab *tt, const char *name, uintmax_t n, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	struct tabPage page;

	page.bin = newBin();
	uiBinSetMainControl(page.bin, child);
	// and add it as a tab page
	uiBinSetOSParent(page.bin, (uintptr_t) (t->container));
	page.binWidget = GTK_WIDGET(uiControlHandle(uiControl(page.bin)));
	gtk_notebook_set_tab_label_text(t->notebook, page.binWidget, name);

	gtk_notebook_reorder_child(t->notebook, page.binWidget, n);
	g_array_insert_val(t->pages, n, page);
}

static void tabDeletePage(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;
	struct tabPage *page;

	page = &g_array_index(t->pages, struct tabPage, n);

	// make sure the page's control isn't destroyed
	uiBinSetMainControl(page->bin, NULL);

	// now destroy the page
	// this will also remove the tab
	// why? simple: both gtk_notebook_remove_tab() and gtk_widget_destroy() call gtk_container_remove()
	// we need to remove them from the tab first, though, otherwise they won't really be destroyed properly
	// (the GtkNotebook will still have the tab in it because its reference ISN'T destroyed, and we crash resizing a bin that no longer exists
	// TODO redo this comment
	uiBinRemoveOSParent(page->bin);
	uiControlDestroy(uiControl(page->bin));

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
	return page->margined;
}

static void tabSetMargined(uiTab *tt, uintmax_t n, int margined)
{
	struct tab *t = (struct tab *) tt;
	struct tabPage *page;

	page = &g_array_index(t->pages, struct tabPage, n);
	page->margined = margined;
	if (page->margined)
		uiBinSetMargins(page->bin, gtkXMargin, gtkYMargin, gtkXMargin, gtkYMargin);
	else
		uiBinSetMargins(page->bin, 0, 0, 0, 0);
}

uiTab *uiNewTab(void)
{
	struct tab *t;

	t = uiNew(struct tab);

	uiUnixMakeControl(uiControl(t), GTK_TYPE_NOTEBOOK,
		FALSE, FALSE, onDestroy, t,
		"scrollable", TRUE,
		NULL);

	t->pages = g_array_new(FALSE, TRUE, sizeof (struct tabPage));

	t->widget = GTK_WIDGET(uiControlHandle(uiControl(t)));
	t->container = GTK_CONTAINER(t->widget);
	t->notebook = GTK_NOTEBOOK(t->widget);

	uiControl(t)->Show = tabShow;

	uiTab(t)->AppendPage = tabAppendPage;
	uiTab(t)->InsertPageBefore = tabInsertPageBefore;
	uiTab(t)->DeletePage = tabDeletePage;
	uiTab(t)->NumPages = tabNumPages;
	uiTab(t)->Margined = tabMargined;
	uiTab(t)->SetMargined = tabSetMargined;

	return uiTab(t);
}
