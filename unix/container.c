// 28 april 2015
#include "uipriv_unix.h"

#define containerWidgetType (containerWidget_get_type())
#define containerWidget(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), containerWidgetType, containerWidget))
#define IscontainerWidget(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), containerWidgetType))
#define containerWidgetClass(class) (G_TYPE_CHECK_CLASS_CAST((class), containerWidgetType, containerWidgetClass))
#define IscontainerWidgetClass(class) (G_TYPE_CHECK_CLASS_TYPE((class), containerWidget))
#define GetcontainerWidgetClass(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), containerWidgetType, containerWidgetClass))

typedef struct containerWidget containerWidget;
typedef struct containerWidgetClass containerWidgetClass;

struct containerWidget {
	GtkContainer parent_instance;
	uiControl *c;
	GPtrArray *widgets;		// for for_each()/for_all()
};

struct containerWidgetClass {
	GtkContainerClass parent_class;
};

G_DEFINE_TYPE(containerWidget, containerWidget, GTK_TYPE_CONTAINER)

static void containerWidget_init(containerWidget *c)
{
	c->widgets = g_ptr_array_new();
	gtk_widget_set_has_window(GTK_WIDGET(c), FALSE);
}

static void containerWidget_dispose(GObject *obj)
{
	G_OBJECT_CLASS(containerWidget_parent_class)->dispose(obj);
}

static void containerWidget_finalize(GObject *obj)
{
	containerWidget *c = containerWidget(obj);

	g_ptr_array_unref(c->widgets);
	G_OBJECT_CLASS(containerWidget_parent_class)->finalize(obj);
}

static void containerWidget_add(GtkContainer *container, GtkWidget *widget)
{
	containerWidget *c = containerWidget(container);

	gtk_widget_set_parent(widget, GTK_WIDGET(c));
	g_ptr_array_add(c->widgets, widget);
}

static void containerWidget_remove(GtkContainer *container, GtkWidget *widget)
{
	containerWidget *c = containerWidget(container);

	gtk_widget_unparent(widget);
	if (g_ptr_array_remove(c->widgets, widget) == FALSE)
		complain("widget %p not found in containerWidget gtk_container_remove()", widget);
}

static void containerWidget_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	containerWidget *c = containerWidget(widget);
	uiSizing *d;
	intmax_t x, y, width, height;

	gtk_widget_set_allocation(GTK_WIDGET(c), allocation);
	x = allocation->x;
	y = allocation->y;
	width = allocation->width;
	height = allocation->height;
	d = uiUnixNewSizing();
	uiControlResize(c->c, x, y, width, height, d);
	uiFreeSizing(d);
}

static void containerWidget_get_preferred_height(GtkWidget *widget, gint *minimum, gint *natural)
{
	containerWidget *c = containerWidget(widget);
	intmax_t width, height;
	uiSizing *d;

	d = uiUnixNewSizing();
	uiControlPreferredSize(uiControl(c->c), d, &width, &height);
	uiFreeSizing(d);
	*minimum = 0;			// allow arbitrary resize
	*natural = height;
}

static void containerWidget_get_preferred_width(GtkWidget *widget, gint *minimum, gint *natural)
{
	containerWidget *c = containerWidget(widget);
	intmax_t width, height;
	uiSizing *d;

	d = uiUnixNewSizing();
	uiControlPreferredSize(uiControl(c->c), d, &width, &height);
	uiFreeSizing(d);
	*minimum = 0;			// allow arbitrary resize
	*natural = width;
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

static void containerWidget_forall(GtkContainer *container, gboolean includeInternals, GtkCallback callback, gpointer data)
{
	containerWidget *c = containerWidget(container);
	struct forall s;

	s.callback = callback;
	s.data = data;
	g_ptr_array_foreach(c->widgets, doforall, &s);
}

static void containerWidget_class_init(containerWidgetClass *class)
{
	G_OBJECT_CLASS(class)->dispose = containerWidget_dispose;
	G_OBJECT_CLASS(class)->finalize = containerWidget_finalize;
	GTK_WIDGET_CLASS(class)->size_allocate = containerWidget_size_allocate;
	GTK_WIDGET_CLASS(class)->get_preferred_height = containerWidget_get_preferred_height;
	GTK_WIDGET_CLASS(class)->get_preferred_width = containerWidget_get_preferred_width;
	GTK_CONTAINER_CLASS(class)->add = containerWidget_add;
	GTK_CONTAINER_CLASS(class)->remove = containerWidget_remove;
	GTK_CONTAINER_CLASS(class)->forall = containerWidget_forall;
}

uintptr_t uiMakeContainer(uiControl *c)
{
	GtkWidget *widget;

	widget = GTK_WIDGET(g_object_new(containerWidgetType, NULL));
	uiUnixMakeSingleWidgetControl(c, widget);
	containerWidget(widget)->c = c;
	return (uintptr_t) widget;
}
