// 9 june 2016
#include "uipriv_unix.h"
#include "fixedContainer.c"

struct fixedChild {
	uiControl *c;
	int width;
	int height;
};

struct uiFixed {
	uiUnixControl c;
	GtkWidget *widget;
	GtkContainer *container;
	fixedContainer *fixed;
	GArray *children;
};

uiUnixControlAllDefaultsExceptDestroy(uiFixed)

#define ctrl(g, i) &g_array_index(g->children, struct fixedChild, i)

static void uiFixedDestroy(uiControl *c)
{
	uiFixed *g = uiFixed(c);
	struct fixedChild *gc;
	guint i;

	// free all controls
	for (i = 0; i < g->children->len; i++) {
		gc = ctrl(g, i);
		uiControlSetParent(gc->c, NULL);
		uiUnixControlSetContainer(uiUnixControl(gc->c), g->container, TRUE);
		uiControlDestroy(gc->c);
	}
	g_array_free(g->children, TRUE);
	// and then ourselves
	g_object_unref(g->widget);
	uiFreeControl(uiControl(g));
}

#define TODO_MASSIVE_HACK(c) \
	if (!uiUnixControl(c)->addedBefore) { \
		g_object_ref_sink(GTK_WIDGET(uiControlHandle(uiControl(c)))); \
		gtk_widget_show(GTK_WIDGET(uiControlHandle(uiControl(c)))); \
		uiUnixControl(c)->addedBefore = TRUE; \
	}

static void uiFixedSizeCallback(GtkWidget *widget, GtkAllocation *allocation, struct fixedChild *data)
{
	if (allocation->height == 0 || allocation->width == 0)
			return;
	data->height = allocation->height;
	data->width = allocation->width;
}

static GtkWidget *prepare(struct fixedChild *gc, uiControl *c)
{
	GtkWidget *widget;

	gc->c = c;
	widget = GTK_WIDGET(uiControlHandle(gc->c));
	return widget;
}

void uiFixedAppend(uiFixed *g, uiControl *c, int x, int y)
{
	struct fixedChild gc;
	GtkWidget *widget;

	widget = prepare(&gc, c);
	uiControlSetParent(gc.c, uiControl(g));
	TODO_MASSIVE_HACK(uiUnixControl(gc.c));
	fixedContainer_put(g->fixed, widget, x, y);
	g_array_append_val(g->children, gc);

	g_signal_connect(widget, "size-allocate", G_CALLBACK(uiFixedSizeCallback), &gc);
}

void uiFixedMove(uiFixed *g, uiControl *c, int x, int y)
{
	fixedContainer_move(g->fixed, GTK_WIDGET(uiControlHandle(c)), x, y);
}

void uiFixedSize(uiFixed *g, uiControl *c, int *width, int *height)
{
	struct fixedChild *gc;
	guint i;

	for (i = 0; i < g->children->len; i++) {
		gc = ctrl(g, i);
		if (gc->c == c) {
			if (gc->width == 0 || gc->height == 0) {
				GtkRequisition natural_size;
				gtk_widget_get_preferred_size(GTK_WIDGET(uiControlHandle(c)), NULL, &natural_size);
				if (natural_size.width == 0 || natural_size.height == 0)
					return;
				*width = natural_size.width;
				*height = natural_size.height;
			} else {
				*width = gc->width;
				*height = gc->height;
			}
		}
	}
}

void uiFixedSetSize(uiFixed *g, uiControl *c, int width, int height)
{
	gtk_widget_set_size_request(GTK_WIDGET(uiControlHandle(c)), width, height);
}

uiFixed *uiNewFixed(void)
{
	uiFixed *g;

	uiUnixNewControl(uiFixed, g);

	g->widget = fixedContainer_new();
	g->container = GTK_CONTAINER(g->widget);
	g->fixed = FIXEDCONTAINER(g->widget);

	g->children = g_array_new(FALSE, TRUE, sizeof (struct fixedChild));

	return g;
}
