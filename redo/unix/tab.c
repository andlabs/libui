// 11 june 2015
#include "uipriv_unix.h"

struct uiTab {
	uiUnixControl c;

	GtkWidget *widget;
	GtkContainer *container;
	GtkNotebook *notebook;

	GArray *pages;
};

struct tabPage {
	GtkWidget *box;
	uiControl *c;
	int margined;
};

static void onDestroy(uiTab *);

uiUnixDefineControlWithOnDestroy(
	uiTab,								// type name
	uiTabType,							// type function
	onDestroy(this);						// on destroy
)

static void onDestroy(uiTab *t)
{
	guint i;
	struct tabPage *page;

	// the pages do not have a libui parent, so we can simply destroy them
	// we need to remove them from the tab first; see below
	for (i = 0; i < t->pages->len; i++) {
		page = &g_array_index(t->pages, struct tabPage, i);
		uiControlSetParent(page->c, NULL);
		uiControlDestroy(page->c);
		gtk_widget_destroy(page->box);
	}
	g_array_free(t->pages, TRUE);
}

// TODO tabContainerUpdateState()

void uiTabAppend(uiTab *t, const char *name, uiControl *child)
{
	uiTabInsertAt(t, name, t->pages->len, child);
}

void uiTabInsertAt(uiTab *t, const char *name, uintmax_t n, uiControl *child)
{
	struct tabPage page;

	page.c = child;
	page.box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add(GTK_CONTAINER(page.box),
		GTK_WIDGET(uiControlHandle(page.c)));

	gtk_container_add(t->container, page.box);
	gtk_notebook_set_tab_label_text(t->notebook, page.box, name);
	gtk_notebook_reorder_child(t->notebook, page.box, n);

	g_array_insert_val(t->pages, n, page);
}

void uiTabDelete(uiTab *t, uintmax_t n)
{
	struct tabPage *page;

	page = &g_array_index(t->pages, struct tabPage, n);

	// make sure the page's control isn't destroyed
	uiControlSetParent(page->c, NULL);
	gtk_container_remove(GTK_CONTAINER(page->box),
		GTK+WIDGET(uiControlHandle(page->c)));

	// this will also remove the tab
	gtk_widget_destroy(page->box);

	g_array_remove_index(t->pages, n);
}

uintmax_t uiTabNumPages(uiTab *t)
{
	return t->pages->len;
}

int uiTabMargined(uiTab *t, uintmax_t n)
{
	struct tabPage *page;

	page = &g_array_index(t->pages, struct tabPage, n);
	return page->margined;
}

void uiTabSetMargined(uiTab *t, uintmax_t n, int margined)
{
	struct tabPage *page;

	page = &g_array_index(t->pages, struct tabPage, n);
	page->margined = margined;
	setMargined(GTK_CONTAINER(page->box), page->margined);
}

uiTab *uiNewTab(void)
{
	uiTab *t;

	t = (uiTab *) uiNewControl(uiTabType());

	t->widget = gtk_notebook_new();
	t->container = GTK_CONTAINER(t->widget);
	t->notebook = GTK_NOTEBOOK(t->widget);

	gtk_notebook_set_scrollable(t->notebook, TRUE);

	t->pages = g_array_new(FALSE, TRUE, sizeof (struct tabPage));

	uiUnixFinishNewControl(t, uiTab);
//TODO	uiControl(t)->ContainerUpdateState = tabContainerUpdateState;

	return t;
}
