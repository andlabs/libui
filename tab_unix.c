// 12 april 2015
#include "uipriv_unix.h"

struct tab {
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

uiControl *uiNewTab(void)
{
	uiControl *c;
	struct tab *t;
	GtkWidget *widget;

	c = uiUnixNewControl(GTK_TYPE_NOTEBOOK,
		FALSE, FALSE,
		NULL);

	widget = GTK_WIDGET(uiControlHandle(c));

	t = uiNew(struct tab);
	g_signal_connect(widget, "destroy", G_CALLBACK(onDestroy), t);
	c->data = t;

	return c;
}

#define tabCapGrow 32

void uiTabAddPage(uiControl *c, const char *name, uiControl *child)
{
	struct tab *t = (struct tab *) (c->data);
	GtkWidget *notebook;
	uiParent *content;

	if (t->len >= t->cap) {
		t->cap += tabCapGrow;
		t->pages = (uiParent **) uiRealloc(t->pages, t->cap * sizeof (uiParent *), "uiParent *[]");
	}

	notebook = GTK_WIDGET(uiControlHandle(c));
	content = uiNewParent((uintptr_t) notebook);
	uiParentSetChild(content, child);
	uiParentUpdate(content);
	gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook), GTK_WIDGET(uiParentHandle(content)), name);

	t->pages[t->len] = content;
	t->len++;
}
