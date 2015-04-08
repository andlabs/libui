// 7 april 2015
#include "uipriv_unix.h"

typedef struct uiSingleWidgetControl uiSingleWidgetControl;

struct uiSingleWidgetControl {
	uiControl control;
	GtkWidget *widget;
	GtkWidget *scrolledWindow;
	GtkWidget *immediate;		// the widget that is added to the parent container; either widget or scrolledWindow
	void *data;
};

#define S(c) ((uiSingleWidgetControl *) (c))

static void singleDestroy(uiControl *c)
{
	gtk_widget_destroy(S(c)->immediate);
}

static uintptr_t singleHandle(uiControl *c)
{
	return (uintptr_t) (S(c)->widget);
}

static void singleSetParent(uiControl *c, uintptr_t parent)
{
	gtk_container_add(GTK_CONTAINER(parent), S(c)->immediate);
}

static uiSize singlePreferredSize(uiControl *c, uiSizing *d)
{
	uiSize size;
	GtkRequisition natural;

	// use the natural size as the minimum size is an *absolute* minimum
	// for example, if a label has ellipsizing on, it can be the width of the ellipses, not the text
	// there is a warning about height-for-width sizing, but in my tests this isn't an issue
	gtk_widget_get_preferred_size(S(c)->widget, NULL, &natural);
	size.width = natural.width;
	size.height = natural.height;
	return size;
}

static void singleResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	GtkAllocation a;

	a.x = x;
	a.y = y;
	a.width = width;
	a.height = height;
	gtk_widget_size_allocate(S(c)->immediate, &a);
}

static void onDestroy(GtkWidget *widget, gpointer data)
{
	uiSingleWidgetControl *c = (uiSingleWidgetControl *) data;

	uiFree(c);
}

uiControl *uiUnixNewControl(GType type, gboolean inScrolledWindow, gboolean scrolledWindowHasBorder, void *data, const char *firstProperty, ...)
{
	uiSingleWidgetControl *c;
	va_list ap;

	c = uiNew(uiSingleWidgetControl);

	va_start(ap, firstProperty);
	c->widget = GTK_WIDGET(g_object_new_valist(type, firstProperty, ap));
	va_end(ap);
	c->immediate = c->widget;

	// TODO turn into bit field?
	if (inScrolledWindow) {
		c->scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
		if (!GTK_IS_SCROLLABLE(c->widget))
			gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(c->scrolledWindow), c->widget);
		else
			gtk_container_add(GTK_CONTAINER(c->scrolledWindow), c->widget);
		if (scrolledWindowHasBorder)
			gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(c->scrolledWindow), GTK_SHADOW_IN);
		c->immediate = c->scrolledWindow;
	}

	// we need to keep an extra reference on the immediate widget
	// this is so uiControlDestroy() can work regardless of when it is called and who calls it
	// without this:
	// - end user call works (only one ref)
	// - call in uiContainer destructor fails (uiContainer ref freed)
	// with this:
	// - end user call works (shoudn't be in any container)
	// - call in uiContainer works (both refs freed)
	g_object_ref_sink(c->immediate);
	// and let's free the uiSingleWidgetControl with it
	g_signal_connect(c->immediate, "destroy", G_CALLBACK(onDestroy), c);

	c->control.destroy = singleDestroy;
	c->control.handle = singleHandle;
	c->control.setParent = singleSetParent;
	c->control.preferredSize = singlePreferredSize;
	c->control.resize = singleResize;

	c->data = data;

	return (uiControl *) c;
}

void *uiUnixControlData(uiControl *c)
{
	return S(c)->data;
}
