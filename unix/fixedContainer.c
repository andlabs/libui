#include <gtk/gtk.h>

GType      gtk_fixed_get_type          (void) G_GNUC_CONST;

#define FIXEDCONTAINER_TYPE (fixedContainer_get_type())
#define FIXEDCONTAINER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), FIXEDCONTAINER_TYPE, fixedContainer))
#define IS_FIXEDCONTAINER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), FIXEDCONTAINER_TYPE))
#define FIXEDCONTAINER_CLASS(class) (G_TYPE_CHECK_CLASS_CAST((class), FIXEDCONTAINER_TYPE, fixedContainerClass))
#define IS_FIXEDCONTAINER_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE((class), FIXEDCONTAINER_TYPE))
#define FIXEDCONTAINER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), FIXEDCONTAINER_TYPE, fixedContainerClass))

typedef struct fixedContainer fixedContainer;
typedef struct fixedContainerClass fixedContainerClass;
typedef struct fixedContainerChild fixedContainerChild;

struct fixedContainer {
    GtkContainer parent_instance;
    GList *children;
};

struct fixedContainerClass {
    GtkContainerClass parent_class;
};

struct fixedContainerChild {
    GtkWidget *widget;
    gint x;
    gint y;
};

G_DEFINE_TYPE(fixedContainer, fixedContainer, GTK_TYPE_CONTAINER)

static fixedContainerChild* get_child (fixedContainer  *fixed, GtkWidget *widget)
{
  GList *children;

  for (children = fixed->children; children; children = children->next)
    {
      fixedContainerChild *child;

      child = children->data;

      if (child->widget == widget)
        return child;
    }

  return NULL;
}

void fixedContainer_put(fixedContainer *fixed, GtkWidget *widget, gint x, gint y) {
    g_return_if_fail (IS_FIXEDCONTAINER (fixed));
    g_return_if_fail (GTK_IS_WIDGET (widget));

    fixedContainerChild *child_info = g_new(fixedContainerChild, 1);
    child_info->widget = widget;
    child_info->x = x;
    child_info->y = y;

    gtk_widget_set_parent(widget, GTK_WIDGET(fixed));

    fixed->children = g_list_append(fixed->children, child_info);
}

void fixedContainer_move(fixedContainer *fixed, GtkWidget *widget, gint x, gint y) {
    fixedContainerChild *child = get_child(fixed, widget);
    g_return_if_fail (IS_FIXEDCONTAINER (fixed));
  g_return_if_fail (gtk_widget_get_parent (child->widget) == GTK_WIDGET (fixed));
    
    child->x = x;
    child->y = y;

  if (gtk_widget_get_visible (child->widget) && gtk_widget_get_visible (GTK_WIDGET (fixed)))
    gtk_widget_queue_resize (GTK_WIDGET (fixed));
}

static void fixedContainer_init(fixedContainer *c) {
    c->children = NULL;
    gtk_widget_set_has_window(GTK_WIDGET(c), FALSE);
}

static void fixedContainer_dispose(GObject *obj) {
    g_list_free(FIXEDCONTAINER(obj)->children);
    G_OBJECT_CLASS(fixedContainer_parent_class)->dispose(obj);
}

static void fixedContainer_finalize(GObject *obj) {
    G_OBJECT_CLASS(fixedContainer_parent_class)->finalize(obj);
}

static void fixedContainer_add(GtkContainer *container, GtkWidget *widget) {
    fixedContainer_put(FIXEDCONTAINER(container), widget, 0, 0);
}

static void fixedContainer_remove(GtkContainer *container, GtkWidget *widget) {
  fixedContainer *fixed = FIXEDCONTAINER (container);
  fixedContainerChild *child;
  GtkWidget *widget_container = GTK_WIDGET (container);
  GList *children;

  for (children = fixed->children; children; children = children->next)
    {
      child = children->data;

      if (child->widget == widget)
        {
          gboolean was_visible = gtk_widget_get_visible (widget);

          gtk_widget_unparent (widget);

          fixed->children = g_list_remove_link (fixed->children, children);
          g_list_free (children);
          g_free (child);

          if (was_visible && gtk_widget_get_visible (widget_container))
            gtk_widget_queue_resize (widget_container);

          break;
        }
    }
}

static void fixedContainer_size_allocate(GtkWidget *widget, GtkAllocation *allocation) {
    fixedContainer *fixed = FIXEDCONTAINER (widget);
    fixedContainerChild *child;
    GtkAllocation child_allocation;
    GtkRequisition child_requisition;
    GList *children;

    for (children = fixed->children; children; children = children->next)
    {
        child = children->data;

        if (!gtk_widget_get_visible (child->widget))
            continue;

        gtk_widget_get_preferred_size (child->widget, &child_requisition, NULL);
        child_allocation.x = child->x;
        child_allocation.y = child->y;

        child_allocation.width = child_requisition.width;
        child_allocation.height = child_requisition.height;
        gtk_widget_size_allocate (child->widget, &child_allocation);
    }
}

static void fixedContainer_forall(GtkContainer *container, gboolean includeInternals, GtkCallback callback, gpointer callback_data) {
      fixedContainer *fixed = FIXEDCONTAINER (container);
  fixedContainerChild *child;
  GList *children;

  children = fixed->children;
  while (children)
    {
      child = children->data;
      children = children->next;

      (* callback) (child->widget, callback_data);
    }
}

static void fixedContainer_class_init(fixedContainerClass *class) {
    G_OBJECT_CLASS(class)->dispose = fixedContainer_dispose;
	G_OBJECT_CLASS(class)->finalize = fixedContainer_finalize;
	GTK_WIDGET_CLASS(class)->size_allocate = fixedContainer_size_allocate;
	GTK_CONTAINER_CLASS(class)->add = fixedContainer_add;
	GTK_CONTAINER_CLASS(class)->remove = fixedContainer_remove;
	GTK_CONTAINER_CLASS(class)->forall = fixedContainer_forall;
}

GtkWidget* fixedContainer_new(void) {
    return g_object_new(FIXEDCONTAINER_TYPE, NULL);
}
