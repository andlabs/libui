// 13 august 2014
#include "uipriv_unix.h"

#define uipOSContainerType (uipOSContainer_get_type())
#define uipOSContainer(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), uipOSContainerType, uipOSContainer))
#define uipIsOSContainer(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), uipOSContainerType))
#define uipOSContainerClass(class) (G_TYPE_CHECK_CLASS_CAST((class), uipOSContainerType, uipOSContainerClass))
#define uipIsOSContainerClass(class) (G_TYPE_CHECK_CLASS_TYPE((class), uipOSContainer))
#define uipGetParentClass(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), uipOSContainerType, uipOSContainerClass))

typedef struct uipOSContainer uipOSContainer;
typedef struct uipOSContainerClass uipOSContainerClass;

struct uipOSContainer {
	GtkContainer parent_instance;
	uiControl *mainControl;
	GPtrArray *children;		// for forall()
	intmax_t marginLeft;
	intmax_t marginTop;
	intmax_t marginRight;
	intmax_t marginBottom;
	gboolean canDestroy;
};

struct uipOSContainerClass {
	GtkContainerClass parent_class;
};

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

	// don't free mainControl here; that should have been done by uiOSContainerDestroy()
	if (!c->canDestroy)
		complain("attempt to dispose uiOSContainer with uipOSContainer at %p before uiOSContainerDestroy()", c);
	if (c->children != NULL) {
		if (c->children->len != 0)
			complain("disposing uiOSContainer with uipOSContainer at %p while there are still children", c);
		g_ptr_array_unref(c->children);
		c->children = NULL;
	}
	G_OBJECT_CLASS(uipOSContainer_parent_class)->dispose(obj);
}

static void uipOSContainer_finalize(GObject *obj)
{
	uipOSContainer *c = uipOSContainer(obj);

	if (!c->canDestroy)
		complain("attempt to finalize uiOSContainer with uipOSContainer at %p before uiOSContainerDestroy()", c);
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

// TODO convert other methods of other backends to pp arg p instance variable

static void parentDestroy(uiOSContainer *cc)
{
	uipOSContainer *c = uipOSContainer(cc->Internal);

	// first, destroy the main control
	if (c->mainControl != NULL) {
		// we have to do this before we can destroy controls
		uiControlSetHasParent(c->mainControl, 0);
		uiControlSetOSContainer(c->mainControl, NULL);
		uiControlDestroy(c->mainControl);
		c->mainControl = NULL;
	}
	// now we can mark the parent as ready to be destroyed
	c->canDestroy = TRUE;
	// finally, destroy the parent
	g_object_unref(G_OBJECT(c));
	// and free ourselves
	uiFree(cc);
}

static uintptr_t parentHandle(uiOSContainer *cc)
{
	uipOSContainer *c = uipOSContainer(cc->Internal);

	return (uintptr_t) c;
}

static void parentSetMainControl(uiOSContainer *cc, uiControl *mainControl)
{
	uipOSContainer *c = uipOSContainer(cc->Internal);

	if (c->mainControl != NULL) {
		uiControlSetHasParent(c->mainControl, 0);
		uiControlSetOSContainer(c->mainControl, NULL);
	}
	c->mainControl = mainControl;
	if (c->mainControl != NULL) {
		uiControlSetHasParent(c->mainControl, 1);
		uiControlSetOSContainer(c->mainControl, cc);
	}
}

static void parentSetMargins(uiOSContainer *cc, intmax_t left, intmax_t top, intmax_t right, intmax_t bottom)
{
	uipOSContainer *c = uipOSContainer(cc->Internal);

	c->marginLeft = left;
	c->marginTop = top;
	c->marginRight = right;
	c->marginBottom = bottom;
}

static void parentUpdate(uiOSContainer *cc)
{
	uipOSContainer *c = uipOSContainer(cc->Internal);

	gtk_widget_queue_resize(GTK_WIDGET(c));
}

uiOSContainer *uiNewOSContainer(uintptr_t osParent)
{
	uiOSContainer *c;

	c = uiNew(uiOSContainer);
	c->Internal = g_object_new(uipOSContainerType, NULL);
	c->Destroy = parentDestroy;
	c->Handle = parentHandle;
	c->SetMainControl = parentSetMainControl;
	c->SetMargins = parentSetMargins;
	c->Update = parentUpdate;
	gtk_container_add(GTK_CONTAINER(osParent), GTK_WIDGET(c->Internal));
	// make it visible by default
	gtk_widget_show_all(GTK_WIDGET(c->Internal));
	// hold a reference to it to keep it alive
	g_object_ref(G_OBJECT(c->Internal));
	return c;
}
