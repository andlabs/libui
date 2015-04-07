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

static void singleContainerShow(uiControl *c)
{
	gtk_widget_show_all(S(c)->immediate);
}

static void singleContainerHide(uiControl *c)
{
	gtk_widget_hide(S(c)->immediate);
}

// TODO connect free function

uiControl *uiUnixNewControl(GType type, guint nConstructParams, GParameter *constructParams, gboolean inScrolledWindow, gboolean needsViewport, gboolean scrolledWindowHasBorder, void *data)
{
	uiSingleHWNDControl *c;

	c = g_new0(uiSingleHWNDControl, 1);
	c->widget = GTK_WIDGET(g_object_newv(type, nConstructParams, constructParams));
	c->immediate = c->widget;

	// TODO turn into bit field?
	// TODO should we check to see if the GType implements GtkScrollable instead of having this passed as a parameter?
	if (inScrolledWindow) {
		c->scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
		if (needsViewport)
			gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(c->scrolledWindow), c->widget);
		else
			gtk_container_add(GTK_CONTAINER(c->scrolledWindow), c->widget);
		if (scrolledWindowHasBorder)
			gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(c->scrolledWindow), GTK_SHADOW_IN);
		c->immediate = c->scrolledWindow;
	}

	c->control.handle = singleHandle;
	c->control.setParent = singleSetParent;
	c->control.preferredSize = singlePreferredSize;
	c->control.resize = singleResize;
	c->control.containerShow = singleContainerShow;
	c->control.containerHide = singleContainerHide;

	c->data = data;

	return (uiControl *) c;
}

void *uiUnixControlData(uiControl *c)
{
	return S(c)->data;
}
