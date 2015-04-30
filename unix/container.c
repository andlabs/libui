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
	uiContainer *c;
	GPtrArray *widgets;		// for for_each()/for_all()
	uiContainer *parent;
	int hidden;
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

#define gtkXPadding 12
#define gtkYPadding 6

static void containerWidget_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	containerWidget *c = containerWidget(widget);
	uiSizing d;

	gtk_widget_set_allocation(GTK_WIDGET(c), allocation);
	d.xPadding = gtkXPadding;
	d.yPadding = gtkYPadding;
	uiContainerResizeChildren(c->c, allocation->x, allocation->y, allocation->width, allocation->height, &d);
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
	GTK_CONTAINER_CLASS(class)->add = containerWidget_add;
	GTK_CONTAINER_CLASS(class)->remove = containerWidget_remove;
	GTK_CONTAINER_CLASS(class)->forall = containerWidget_forall;
}

// subclasses override this and call back here when all children are destroyed
static void containerDestroy(uiControl *cc)
{
	containerWidget *c = containerWidget(cc->Internal);

	if (c->parent != NULL)
		complain("attempt to destroy uiContainer %p while it has a parent", cc);
	g_object_unref(c);		// release our initial reference, which destroys the widget
}

static uintptr_t containerHandle(uiControl *cc)
{
	containerWidget *c = containerWidget(cc->Internal);

	return (uintptr_t) c;
}

static void containerSetParent(uiControl *cc, uiContainer *parent)
{
	containerWidget *c = containerWidget(cc->Internal);
	uiContainer *oldparent;
	GtkContainer *oldcontainer;
	GtkContainer *newcontainer;

	oldparent = c->parent;
	c->parent = parent;
	if (oldparent != NULL) {
		oldcontainer = GTK_CONTAINER(uiControlHandle(uiControl(oldparent)));
		gtk_container_remove(oldcontainer, GTK_WIDGET(c));
	}
	if (c->parent != NULL) {
		newcontainer = GTK_CONTAINER(uiControlHandle(uiControl(c->parent)));
		gtk_container_add(newcontainer, GTK_WIDGET(c));
	}
	if (oldparent != NULL)
		uiContainerUpdate(oldparent);
	if (c->parent != NULL)
		uiContainerUpdate(c->parent);
}

static void containerResize(uiControl *cc, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	containerWidget *c = containerWidget(cc->Internal);
	GtkAllocation a;

	a.x = x;
	a.y = y;
	a.width = width;
	a.height = height;
	gtk_widget_size_allocate(GTK_WIDGET(c), &a);
}

static int containerVisible(uiControl *cc)
{
	containerWidget *c = containerWidget(cc->Internal);

	return !c->hidden;
}

static void containerShow(uiControl *cc)
{
	containerWidget *c = containerWidget(cc->Internal);

	// don't use gtk_widget_show_all(); that'll show every widget, including ones hidden by the user
	gtk_widget_show(GTK_WIDGET(c));
	// hidden controls don't count in boxes and grids
	c->hidden = 0;
	if (c->parent != NULL)
		uiContainerUpdate(c->parent);
}

static void containerHide(uiControl *cc)
{
	containerWidget *c = containerWidget(cc->Internal);

	gtk_widget_hide(GTK_WIDGET(c));
	c->hidden = 1;
	if (c->parent != NULL)
		uiContainerUpdate(c->parent);
}

static void containerEnable(uiControl *cc)
{
	containerWidget *c = containerWidget(cc->Internal);

	gtk_widget_set_sensitive(GTK_WIDGET(c), TRUE);
}

static void containerDisable(uiControl *cc)
{
	containerWidget *c = containerWidget(cc->Internal);

	gtk_widget_set_sensitive(GTK_WIDGET(c), FALSE);
}

static void containerUpdate(uiContainer *cc)
{
	containerWidget *c = containerWidget(uiControl(cc)->Internal);

	gtk_widget_queue_resize(GTK_WIDGET(c));
}

void uiMakeContainer(uiContainer *cc)
{
	containerWidget *c;

	c = containerWidget(g_object_new(containerWidgetType, NULL));
	c->c = cc;
	// keep a reference to our container so it stays alive when reparented
	g_object_ref_sink(c);
	// and make it visible
	gtk_widget_show_all(GTK_WIDGET(c));

	uiControl(cc)->Internal = c;
	uiControl(cc)->Destroy = containerDestroy;
	uiControl(cc)->Handle = containerHandle;
	uiControl(cc)->SetParent = containerSetParent;
	// PreferredSize() is provided by subclasses
	uiControl(cc)->Resize = containerResize;
	uiControl(cc)->Visible = containerVisible;
	uiControl(cc)->Show = containerShow;
	uiControl(cc)->Hide = containerHide;
	uiControl(cc)->Enable = containerEnable;
	uiControl(cc)->Disable = containerDisable;

	// ResizeChildren() is provided by subclasses
	uiContainer(cc)->Update = containerUpdate;
}
