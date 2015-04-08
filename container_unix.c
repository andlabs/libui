// 13 august 2014
#include "uipriv_unix.h"

G_DEFINE_TYPE(uiContainer, uiContainer, GTK_TYPE_CONTAINER)

static void uiContainer_init(uiContainer *c)
{
#ifdef uiLogAllocations
	fprintf(stderr, "%p alloc uiContainer\n", c);
#endif
	c->children = g_ptr_array_new();
	gtk_widget_set_has_window(GTK_WIDGET(c), FALSE);
}

// instead of having GtkContainer itself unref all our controls, we'll run our own uiControlDestroy() functions for child, which will do that and more
// we still chain up because we need to, but by that point there will be no children for GtkContainer to free
static void uiContainer_dispose(GObject *obj)
{
	uiContainer *c = uiContainer(obj);

	if (c->children != NULL) {
		g_ptr_array_unref(c->children);
		c->children = NULL;
	}
	if (c->child != NULL) {
		uiControlDestroy(c->child);
		c->child = NULL;
	}
	G_OBJECT_CLASS(uiContainer_parent_class)->dispose(obj);
}

// TODO switch from using uiContainer() directly below

static void uiContainer_finalize(GObject *obj)
{
	G_OBJECT_CLASS(uiContainer_parent_class)->finalize(obj);
#ifdef uiLogAllocations
	fprintf(stderr, "%p free\n", obj);
#endif
}

static void uiContainer_add(GtkContainer *container, GtkWidget *widget)
{
	gtk_widget_set_parent(widget, GTK_WIDGET(container));
	if (uiContainer(container)->children != NULL)
		g_ptr_array_add(uiContainer(container)->children, widget);
}

static void uiContainer_remove(GtkContainer *container, GtkWidget *widget)
{
	gtk_widget_unparent(widget);
	if (uiContainer(container)->children != NULL)
		g_ptr_array_remove(uiContainer(container)->children, widget);
}

static void uiContainer_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	uiControl *c;
	uiSizing d;

	gtk_widget_set_allocation(widget, allocation);
	c = uiContainer(widget)->child;
	if (c != NULL)
		(*(c->resize))(c, allocation->x, allocation->y, allocation->width, allocation->height, &d);
}

struct forall {
	GtkCallback callback;
	gpointer data;
};

static void doforall(gpointer obj, gpointer data)
{
	struct forall *s = (struct forall *) data;

	(*(s->callback))(GTK_WIDGET(obj), s->data);
}

static void uiContainer_forall(GtkContainer *container, gboolean includeInternals, GtkCallback callback, gpointer data)
{
	struct forall s;

	s.callback = callback;
	s.data = data;
	if (uiContainer(container)->children != NULL)
		g_ptr_array_foreach(uiContainer(container)->children, doforall, &s);
}

static void uiContainer_class_init(uiContainerClass *class)
{
	G_OBJECT_CLASS(class)->dispose = uiContainer_dispose;
	G_OBJECT_CLASS(class)->finalize = uiContainer_finalize;
	GTK_WIDGET_CLASS(class)->size_allocate = uiContainer_size_allocate;
	GTK_CONTAINER_CLASS(class)->add = uiContainer_add;
	GTK_CONTAINER_CLASS(class)->remove = uiContainer_remove;
	GTK_CONTAINER_CLASS(class)->forall = uiContainer_forall;
}

GtkWidget *newContainer(void)
{
	uiContainer *c;

	c = uiContainer(g_object_new(uiContainerType, NULL));
	return GTK_WIDGET(c);
}
