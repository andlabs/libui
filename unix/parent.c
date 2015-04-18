// 13 august 2014
#include "uipriv_unix.h"

#define uipParentType (uipParent_get_type())
#define uipParent(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), uipParentType, uipParent))
#define uipIsParent(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), uipParentType))
#define uipParentClass(class) (G_TYPE_CHECK_CLASS_CAST((class), uipParentType, uipParentClass))
#define uipIsParentClass(class) (G_TYPE_CHECK_CLASS_TYPE((class), uipParent))
#define uipGetParentClass(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), uipParentType, uipParentClass))

typedef struct uipParent uipParent;
typedef struct uipParentClass uipParentClass;

struct uipParent {
	GtkContainer parent_instance;
	uiControl *mainControl;
	GPtrArray *children;		// for forall()
	intmax_t marginLeft;
	intmax_t marginTop;
	intmax_t marginRight;
	intmax_t marginBottom;
	gboolean canDestroy;
};

struct uipParentClass {
	GtkContainerClass parent_class;
};

G_DEFINE_TYPE(uipParent, uipParent, GTK_TYPE_CONTAINER)

static void uipParent_init(uipParent *p)
{
	if (options.debugLogAllocations)
		fprintf(stderr, "%p alloc uipParent\n", p);
	p->children = g_ptr_array_new();
	gtk_widget_set_has_window(GTK_WIDGET(p), FALSE);
}

// instead of having GtkContainer itself unref all our controls, we'll run our own uiControlDestroy() functions for child, which will do that and more
// we still chain up because we need to, but by that point there will be no children for GtkContainer to free
static void uipParent_dispose(GObject *obj)
{
	uipParent *p = uipParent(obj);

	// don't free mainControl here; that should have been done by uiParentDestroy()
	if (!p->canDestroy)
		// TODO switch to complain()
		g_error("attempt to dispose uiParent with uipParent at %p before uiParentDestroy()", p);
	if (p->children != NULL) {
		g_ptr_array_unref(p->children);
		p->children = NULL;
	}
	G_OBJECT_CLASS(uipParent_parent_class)->dispose(obj);
}

static void uipParent_finalize(GObject *obj)
{
	uipParent *p = uipParent(obj);

	if (!p->canDestroy)
		// TODO switch to complain()
		g_error("attempt to finalize uiParent with uipParent at %p before uiParentDestroy()", p);
	G_OBJECT_CLASS(uipParent_parent_class)->finalize(obj);
	if (options.debugLogAllocations)
		fprintf(stderr, "%p free\n", obj);
}

static void uipParent_add(GtkContainer *container, GtkWidget *widget)
{
	uipParent *p = uipParent(container);

	gtk_widget_set_parent(widget, GTK_WIDGET(p));
	if (p->children != NULL)
		g_ptr_array_add(p->children, widget);
}

static void uipParent_remove(GtkContainer *container, GtkWidget *widget)
{
	uipParent *p = uipParent(container);

	gtk_widget_unparent(widget);
	if (p->children != NULL)
		g_ptr_array_remove(p->children, widget);
}

#define gtkXPadding 12
#define gtkYPadding 6

static void uipParent_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	uipParent *p = uipParent(widget);
	uiSizing d;
	intmax_t x, y, width, height;

	gtk_widget_set_allocation(GTK_WIDGET(p), allocation);
	if (p->mainControl == NULL)
		return;
	x = allocation->x + p->marginLeft;
	y = allocation->y + p->marginTop;
	width = allocation->width - (p->marginLeft + p->marginRight);
	height = allocation->height - (p->marginTop + p->marginBottom);
	d.xPadding = gtkXPadding;
	d.yPadding = gtkYPadding;
	uiControlResize(p->mainControl, x, y, width, height, &d);
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

static void uipParent_forall(GtkContainer *container, gboolean includeInternals, GtkCallback callback, gpointer data)
{
	uipParent *p = uipParent(container);
	struct forall s;

	s.callback = callback;
	s.data = data;
	if (p->children != NULL)
		g_ptr_array_foreach(p->children, doforall, &s);
}

static void uipParent_class_init(uipParentClass *class)
{
	G_OBJECT_CLASS(class)->dispose = uipParent_dispose;
	G_OBJECT_CLASS(class)->finalize = uipParent_finalize;
	GTK_WIDGET_CLASS(class)->size_allocate = uipParent_size_allocate;
	GTK_CONTAINER_CLASS(class)->add = uipParent_add;
	GTK_CONTAINER_CLASS(class)->remove = uipParent_remove;
	GTK_CONTAINER_CLASS(class)->forall = uipParent_forall;
}

// TODO convert other methods of this + other backends to pp arg p instance variable

static void parentDestroy(uiParent *pp)
{
	uipParent *p = uipParent(pp->Internal);

	// first, destroy the main control
	if (p->mainControl != NULL) {
		uiControlDestroy(p->mainControl);
		p->mainControl = NULL;
	}
	// now we can mark the parent as ready to be destroyed
	p->canDestroy = TRUE;
	// finally, destroy the parent
	gtk_widget_destroy(GTK_WIDGET(p));
}

static uintptr_t parentHandle(uiParent *p)
{
	uipParent *pp = uipParent(p->Internal);

	return (uintptr_t) pp;
}

static void parentSetMainControl(uiParent *pp, uiControl *mainControl)
{
	uipParent *p = uipParent(pp->Internal);

	if (p->mainControl != NULL)
		uiControlSetParent(p->mainControl, NULL);
	p->mainControl = mainControl;
	if (p->mainControl != NULL)
		uiControlSetParent(p->mainControl, pp);
}

static void parentSetMargins(uiParent *p, intmax_t left, intmax_t top, intmax_t right, intmax_t bottom)
{
	uipParent *pp = uipParent(p->Internal);

	pp->marginLeft = left;
	pp->marginTop = top;
	pp->marginRight = right;
	pp->marginBottom = bottom;
}

static void parentUpdate(uiParent *p)
{
	uipParent *pp = uipParent(p->Internal);

	gtk_widget_queue_resize(GTK_WIDGET(pp));
}

uiParent *uiNewParent(uintptr_t osParent)
{
	uiParent *p;

	p = uiNew(uiParent);
	p->Internal = g_object_new(uipParentType, NULL);
	p->Destroy = parentDestroy;
	p->Handle = parentHandle;
	p->SetMainControl = parentSetMainControl;
	p->SetMargins = parentSetMargins;
	p->Update = parentUpdate;
	gtk_container_add(GTK_CONTAINER(osParent), GTK_WIDGET(p->Internal));
	// and make it visible by default
	gtk_widget_show_all(GTK_WIDGET(p->Internal));
	return p;
}
