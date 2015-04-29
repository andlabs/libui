// 12 april 2015
#include "uipriv_unix.h"

struct tab {
	uiTab t;
	GtkWidget *widget;
	GtkContainer *container;
	GtkNotebook *notebook;
	// TODO switch to GArray or GPtrArray
	uiContainer **pages;
	uintmax_t len;
	uintmax_t cap;
};

static void onDestroy(void *data)
{
	struct tab *t = (struct tab *) data;
	uintmax_t i;

	// first hide ourselves to avoid flicker
	gtk_widget_hide(t->widget);
	// the pages do not have a libui parent, so we can simply destroy them
	// TODO verify if this is sufficient
	for (i = 0; i < t->len; i++)
		uiControlDestroy(uiControl(t->pages[i]));
	// then free ourselves
	uiFree(t->pages);
	uiFree(t);
}

#define tabCapGrow 32

static void tabAppendPage(uiTab *tt, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	uiContainer *page;
	GtkWidget *pageWidget;

	if (t->len >= t->cap) {
		t->cap += tabCapGrow;
		t->pages = (uiContainer **) uiRealloc(t->pages, t->cap * sizeof (uiContainer *), "uiContainer *[]");
	}

	page = newBin();
	binSetMainControl(page, child);
	// and add it as a tab page
	binSetParent(page, (uintptr_t) (t->container));
	pageWidget = GTK_WIDGET(uiControlHandle(uiControl(page)));
	gtk_notebook_set_tab_label_text(t->notebook, pageWidget, name);

	t->pages[t->len] = page;
	t->len++;
}

static void tabDeletePage(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;
	uiContainer *page;
	uintmax_t i;

	page = t->pages[n];
	for (i = n; i < t->len - 1; i++)
		t->pages[i] = t->pages[i + 1];
	t->pages[i] = NULL;
	t->len--;

	// make sure the page's control isn't destroyed
	binSetMainControl(page, NULL);

	// now destroy the page
	// this will also remove the tab
	// why? simple: both gtk_notebook_remove_tab() and gtk_widget_destroy() call gtk_container_remove()
	// TODO verify this
	uiControlDestroy(uiControl(page));
}

uiTab *uiNewTab(void)
{
	struct tab *t;

	t = uiNew(struct tab);

	uiUnixNewControl(uiControl(t), GTK_TYPE_NOTEBOOK,
		FALSE, FALSE, onDestroy, t,
		NULL);

	t->widget = GTK_WIDGET(uiControlHandle(uiControl(t)));
	t->container = GTK_CONTAINER(t->widget);
	t->notebook = GTK_NOTEBOOK(t->widget);

	uiTab(t)->AppendPage = tabAppendPage;
	uiTab(t)->DeletePage = tabDeletePage;

	return uiTab(t);
}
