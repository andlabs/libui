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

static void uiContainer_finalize(GObject *obj)
{
	G_OBJECT_CLASS(uiContainer_parent_class)->finalize(obj);
#ifdef uiLogAllocations
	fprintf(stderr, "%p free\n", obj);
#endif
}

static void uiContainer_add(GtkContainer *container, GtkWidget *widget)
{
	uiContainer *c = uiContainer(container);

	gtk_widget_set_parent(widget, GTK_WIDGET(c));
	if (c->children != NULL)
		g_ptr_array_add(c->children, widget);
}

static void uiContainer_remove(GtkContainer *container, GtkWidget *widget)
{
	uiContainer *c = uiContainer(container);

	gtk_widget_unparent(widget);
	if (c->children != NULL)
		g_ptr_array_remove(c->children, widget);
}

static void uiContainer_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	uiContainer *c = uiContainer(widget);
	uiSizing d;

	gtk_widget_set_allocation(GTK_WIDGET(c), allocation);
	if (c->child != NULL)
		(*(c->child->resize))(c->child, allocation->x, allocation->y, allocation->width, allocation->height, &d);
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
	uiContainer *c = uiContainer(container);
	struct forall s;

	s.callback = callback;
	s.data = data;
	if (c->children != NULL)
		g_ptr_array_foreach(c->children, doforall, &s);
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
	return GTK_WIDGET(g_object_new(uiContainerType, NULL));
}

void updateParent(uintptr_t parent)
{
	if (parent != 0)
		gtk_widget_queue_resize(GTK_WIDGET(parent));
}
