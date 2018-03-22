// 9 june 2016
#include "uipriv_unix.h"

struct fixedChild {
	uiControl *c;
};

struct uiFixed {
	uiUnixControl c;
	GtkWidget *widget;
	GtkContainer *container;
	GtkFixed *fixed;
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
	gtk_fixed_put(g->fixed, widget, x, y);
	g_array_append_val(g->children, gc);
}

void uiFixedMove(uiFixed *g, uiControl *c, int x, int y)
{
	gtk_fixed_move(g->fixed, GTK_WIDGET(uiControlHandle(c)), x, y);
}

uiFixed *uiNewFixed(void)
{
	uiFixed *g;

	uiUnixNewControl(uiFixed, g);

	g->widget = gtk_fixed_new();
	g->container = GTK_CONTAINER(g->widget);
	g->fixed = GTK_FIXED(g->widget);

	g->children = g_array_new(FALSE, TRUE, sizeof (struct fixedChild));

	return g;
}
