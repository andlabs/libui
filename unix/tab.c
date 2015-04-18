// 12 april 2015
#include "uipriv_unix.h"

struct tab {
	uiTab t;
	GtkWidget *widget;
	GtkContainer *container;
	GtkNotebook *notebook;
	uiParent **pages;
	uintmax_t len;
	uintmax_t cap;
};

static void onDestroy(GtkWidget *widget, gpointer data)
{
	struct tab *t = (struct tab *) data;

	uiFree(t->pages);
	uiFree(t);
}

#define tabCapGrow 32

static void tabAddPage(uiTab *tt, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	uiParent *content;

	if (t->len >= t->cap) {
		t->cap += tabCapGrow;
		t->pages = (uiParent **) uiRealloc(t->pages, t->cap * sizeof (uiParent *), "uiParent *[]");
	}

	content = uiNewParent((uintptr_t) (t->container));
	uiParentSetMainControl(content, child);
	uiParentUpdate(content);
	gtk_notebook_set_tab_label_text(t->notebook, GTK_WIDGET(uiParentHandle(content)), name);

	t->pages[t->len] = content;
	t->len++;
}

static void tabDeletePage(uiTab *tt, uintmax_t n)
{
	struct tab *t = (struct tab *) tt;
	uiParent *p;
	uintmax_t i;

	p = t->pages[n];
	for (i = n; i < t->len - 1; i++)
		t->pages[i] = t->pages[i + 1];
	t->pages[i] = NULL;
	t->len--;

	// make sure the page's control isn't destroyed
	uiParentSetMainControl(p, NULL);
	// TODO don't call uiParentDestroy() here as the following line will do so; figure out how to prevent

	gtk_notebook_remove_page(t->notebook, n);
}

uiTab *uiNewTab(void)
{
	struct tab *t;

	t = uiNew(struct tab);

	uiUnixNewControl(uiControl(t), GTK_TYPE_NOTEBOOK,
		FALSE, FALSE,
		NULL);

	t->widget = WIDGET(t);
	t->container = GTK_CONTAINER(t->widget);
	t->notebook = GTK_NOTEBOOK(t->widget);

	g_signal_connect(t->widget, "destroy", G_CALLBACK(onDestroy), t);

	uiTab(t)->AddPage = tabAddPage;
	uiTab(t)->DeletePage = tabDeletePage;

	return uiTab(t);
}
