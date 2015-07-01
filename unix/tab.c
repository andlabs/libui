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

uiTab *uiNewTab(void)
{
	struct tab *t;

	t = uiNew(struct tab);

	uiControl(t)->Show = tabShow;

	uiTab(t)->AppendPage = tabAppendPage;
	uiTab(t)->InsertPageBefore = tabInsertPageBefore;
	uiTab(t)->DeletePage = tabDeletePage;
	uiTab(t)->NumPages = tabNumPages;
	uiTab(t)->Margined = tabMargined;
	uiTab(t)->SetMargined = tabSetMargined;

	return uiTab(t);
}
