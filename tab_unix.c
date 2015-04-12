// 12 april 2015
#include "uipriv_unix.h"

struct tab {
	struct tabPage *pages;
	uintmax_t len;
	uintmax_t cap;
};

struct tabPage {
	GtkWidget *container;
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
	GtkWidget *container;

	if (t->len >= t->cap) {
		t->cap += tabCapGrow;
		t->pages = (struct tabPage *) uiRealloc(t->pages, t->cap * sizeof (struct tabPage), "struct tabPage[]");
	}

	container = newContainer();
	uiContainer(container)->child = child;
	uiControlSetParent(uiContainer(container)->child, (uintptr_t) (container));
	notebook = GTK_WIDGET(uiControlHandle(c));
	gtk_container_add(GTK_CONTAINER(notebook), container);
	gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(notebook), container, name);
	gtk_widget_show_all(container);

	t->pages[t->len].container = container;
	t->len++;
}
