// 13 august 2014
#include "uipriv_unix.h"

// In GTK+, many containers (GtkWindow, GtkNotebook, GtkFrame) can only have one child.
// (In the case of GtkNotebook, each child widget is a single page.)
// GtkFrame and GtkLayout, the official "anything goes" containers, are buggy and ineffective.
// This custom container does what we need just fine.
// uiWindow, uiTab, and uiGroup will keep private instances of this special container, and it will be the OS container given to each uiControl that becomes a child of those three controls.

// This container maintains a "main control", which is the uiControl that is resized alongside the container.
// It also keeps track of all the GtkWidgets that are in the uiControl for the purposes of GTK+ internals.
// Finally, it also handles margining.
// In other words, it does everything uiWindow, uiTab, and uiGroup need to do to keep track of controls.

G_DEFINE_TYPE(uipOSContainer, uipOSContainer, GTK_TYPE_CONTAINER)

static void uipOSContainer_init(uipOSContainer *c)
{
	if (options.debugLogAllocations)
		fprintf(stderr, "%p alloc uipOSContainer\n", c);
	c->children = g_ptr_array_new();
	gtk_widget_set_has_window(GTK_WIDGET(c), FALSE);
}

// instead of having GtkContainer itself unref all our controls, we'll run our own uiControlDestroy() functions for child, which will do that and more
// we still chain up because we need to, but by that point there will be no children for GtkContainer to free
static void uipOSContainer_dispose(GObject *obj)
{
	uipOSContainer *c = uipOSContainer(obj);

	// don't free mainControl here; that should have been done by osContainerDestroy()
	if (!c->canDestroy)
		complain("attempt to dispose uipOSContainer at %p before osContainerDestroy()", c);
	if (c->children != NULL) {
		if (c->children->len != 0)
			complain("disposing uipOSContainer at %p while there are still children", c);
		g_ptr_array_unref(c->children);
		c->children = NULL;
	}
	G_OBJECT_CLASS(uipOSContainer_parent_class)->dispose(obj);
}

static void uipOSContainer_finalize(GObject *obj)
{
	uipOSContainer *c = uipOSContainer(obj);

	if (!c->canDestroy)
		complain("attempt to finalize uipOSContainer at %p before osContainerDestroy()", c);
	G_OBJECT_CLASS(uipOSContainer_parent_class)->finalize(obj);
	if (options.debugLogAllocations)
		fprintf(stderr, "%p free\n", obj);
}

static void uipOSContainer_add(GtkContainer *container, GtkWidget *widget)
{
	uipOSContainer *c = uipOSContainer(container);

	gtk_widget_set_parent(widget, GTK_WIDGET(c));
	if (c->children != NULL)
		g_ptr_array_add(c->children, widget);
}

static void uipOSContainer_remove(GtkContainer *container, GtkWidget *widget)
{
	uipOSContainer *c = uipOSContainer(container);

	gtk_widget_unparent(widget);
	if (c->children != NULL)
		if (g_ptr_array_remove(c->children, widget) == FALSE)
			complain("widget %p not found in uipOSContainer gtk_container_remove()", widget);
}

#define gtkXPadding 12
#define gtkYPadding 6

static void uipOSContainer_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	uipOSContainer *c = uipOSContainer(widget);
	uiSizing d;
	intmax_t x, y, width, height;

	gtk_widget_set_allocation(GTK_WIDGET(c), allocation);
	if (c->mainControl == NULL)
		return;
	x = allocation->x + c->marginLeft;
	y = allocation->y + c->marginTop;
	width = allocation->width - (c->marginLeft + c->marginRight);
	height = allocation->height - (c->marginTop + c->marginBottom);
	d.xPadding = gtkXPadding;
	d.yPadding = gtkYPadding;
	uiControlResize(c->mainControl, x, y, width, height, &d);
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

static void uipOSContainer_forall(GtkContainer *container, gboolean includeInternals, GtkCallback callback, gpointer data)
{
	uipOSContainer *c = uipOSContainer(container);
	struct forall s;

	s.callback = callback;
	s.data = data;
	if (c->children != NULL)
		g_ptr_array_foreach(c->children, doforall, &s);
}

static void uipOSContainer_class_init(uipOSContainerClass *class)
{
	G_OBJECT_CLASS(class)->dispose = uipOSContainer_dispose;
	G_OBJECT_CLASS(class)->finalize = uipOSContainer_finalize;
	GTK_WIDGET_CLASS(class)->size_allocate = uipOSContainer_size_allocate;
	GTK_CONTAINER_CLASS(class)->add = uipOSContainer_add;
	GTK_CONTAINER_CLASS(class)->remove = uipOSContainer_remove;
	GTK_CONTAINER_CLASS(class)->forall = uipOSContainer_forall;
}

GtkWidget *newOSContainer(void)
{
	GtkWidget *c;

	c = GTK_WIDGET(g_object_new(uipOSContainerType, NULL));
	// make it visible by default
	gtk_widget_show_all(c);
	// hold a reference to ourselves to keep ourselves alive when we're removed from whatever container we wind up in
	g_object_ref_sink(c);
	return c;
}

void osContainerDestroy(uipOSContainer *c)
{
	// first, destroy the main control
	if (c->mainControl != NULL) {
		// we have to do this before we can destroy controls
		// TODO clean this up a bit
		uiControlSetHasParent(c->mainControl, 0);
		uiControlSetOSContainer(c->mainControl, 0);
		uiControlDestroy(c->mainControl);
		c->mainControl = NULL;
	}
	// now we can mark the parent as ready to be destroyed
	c->canDestroy = TRUE;
	// finally, actually go ahead and destroy ourselves
	g_object_unref(c);
}

void osContainerSetMainControl(uipOSContainer *c, uiControl *mainControl)
{
	if (c->mainControl != NULL) {
		uiControlSetHasParent(c->mainControl, 0);
		uiControlSetOSContainer(c->mainControl, 0);
	}
	c->mainControl = mainControl;
	if (c->mainControl != NULL) {
		uiControlSetHasParent(c->mainControl, 1);
		uiControlSetOSContainer(c->mainControl, (uintptr_t) c);
	}
	uiUpdateOSContainer((uintptr_t) c);
}

void osContainerSetMargins(uipOSContainer *c, intmax_t left, intmax_t top, intmax_t right, intmax_t bottom)
{
	c->marginLeft = left;
	c->marginTop = top;
	c->marginRight = right;
	c->marginBottom = bottom;
	uiUpdateOSContainer((uintptr_t) c);
}

void uiUpdateOSContainer(uintptr_t c)
{
	gtk_widget_queue_resize(GTK_WIDGET(c));
}
