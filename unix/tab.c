// 12 april 2015
#include "uipriv_unix.h"

struct tab {
	uiTab t;
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

#define TAB(t) GTK_NOTEBOOK(uiControlHandle(uiControl(t)))

#define tabCapGrow 32

static void addPage(uiTab *tt, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) tt;
	GtkWidget *notebook;
	uiParent *content;

	if (t->len >= t->cap) {
		t->cap += tabCapGrow;
		t->pages = (uiParent **) uiRealloc(t->pages, t->cap * sizeof (uiParent *), "uiParent *[]");
	}

	notebook = GTK_WIDGET(TAB(t));
	content = uiNewParent((uintptr_t) notebook);
	uiParentSetChild(content, child);
	uiParentUpdate(content);
	gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook), GTK_WIDGET(uiParentHandle(content)), name);

	t->pages[t->len] = content;
	t->len++;
}

uiTab *uiNewTab(void)
{
	uiControl *c;
	struct tab *t;
	GtkWidget *widget;

	t = uiNew(struct tab);

	uiUnixNewControl(uiControl(t), GTK_TYPE_NOTEBOOK,
		FALSE, FALSE,
		NULL);

	widget = GTK_WIDGET(TAB(t));
	g_signal_connect(widget, "destroy", G_CALLBACK(onDestroy), t);

	uiTab(t)->AddPage = addPage;

	return uiTab(t);
}
