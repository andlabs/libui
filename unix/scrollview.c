// 9 september 2021

#include "uipriv_unix.h"

struct uiScroll {
	uiUnixControl c;
	GtkWidget *widget;
	GtkScrolledWindow *scroll;
	GtkContainer *container;

	uiprivChild *child;

	int margined;
};

uiUnixControlAllDefaultsExceptDestroy(uiScroll)

void uiScrollDestroy(uiControl *c)
{
	uiScroll *v = uiScroll(c);

	if (v->child != NULL)
		uiprivChildDestroy(v->child);
	g_object_unref(v->widget);
	uiFreeControl(c);
}

void uiScrollSetChild(uiScroll *v, uiControl *child)
{
	if (v->child != NULL)
		uiprivChildRemove(v->child);
	v->child = uiprivNewChildWithBox(child, uiControl(v), v->container, v->margined);
}

uiScroll *uiNewScroll(void)
{
	uiScroll *v;

	uiUnixNewControl(uiScroll, v);
	v->widget = gtk_scrolled_window_new(NULL, NULL);
	v->scroll = GTK_SCROLLED_WINDOW(v->widget);
	v->container = GTK_CONTAINER(v->widget);

	return v;
}
