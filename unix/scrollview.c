// 9 september 2021

#include "uipriv_unix.h"

struct uiScrollView {
	uiUnixControl c;
	GtkWidget *widget;
	GtkScrolledWindow *scroll;
	GtkContainer *container;

	uiprivChild *child;

	int margined;
};

uiUnixControlAllDefaultsExceptDestroy(uiScrollView)

void uiScrollViewDestroy(uiControl *c)
{
	uiScrollView *v = uiScrollView(c);

	if (v->child != NULL)
		uiprivChildDestroy(v->child);
	g_object_unref(v->widget);
	uiFreeControl(c);
}

void uiScrollViewSetChild(uiScrollView *v, uiControl *child)
{
	if (v->child != NULL)
		uiprivChildRemove(v->child);
	v->child = uiprivNewChildWithBox(child, uiControl(v), v->container, v->margined);
}

int uiScrollViewMargined(uiScrollView *v)
{
	return v->margined;
}

void uiScrollViewSetMargined(uiScrollView *v, int margined)
{
	v->margined = margined;
	if (v->child != NULL)
		uiprivChildSetMargined(v->child, v->margined);
}

uiScrollView *uiNewScrollView(void)
{
	uiScrollView *v;

	uiUnixNewControl(uiScrollView, v);
	v->widget = gtk_scrolled_window_new(NULL, NULL);
	v->scroll = GTK_SCROLLED_WINDOW(v->widget);
	v->container = GTK_CONTAINER(v->widget);

	return v;
}
