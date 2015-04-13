// 7 april 2015
#include "uipriv_unix.h"

typedef struct singleWidget singleWidget;

struct singleWidget {
	GtkWidget *widget;
	GtkWidget *scrolledWindow;
	GtkWidget *immediate;		// the widget that is added to the parent container; either widget or scrolledWindow
	uiParent *parent;
	gboolean userHid;
	gboolean containerHid;
	gboolean userDisabled;
	gboolean containerDisabled;
};

static void singleDestroy(uiControl *c)
{
	singleWidget *s = (singleWidget *) (c->internal);

	gtk_widget_destroy(s->immediate);
}

static uintptr_t singleHandle(uiControl *c)
{
	singleWidget *s = (singleWidget *) (c->internal);

	return (uintptr_t) (s->widget);
}

static void singleSetParent(uiControl *c, uiParent *parent)
{
	singleWidget *s = (singleWidget *) (c->internal);

	s->parent = parent;
	gtk_container_add(GTK_CONTAINER(uiParentHandle(s->parent)), s->immediate);
	uiParentUpdate(s->parent);
}

static void singleRemoveParent(uiControl *c)
{
	singleWidget *s = (singleWidget *) (c->internal);
	uiParent *oldparent;

	oldparent = s->parent;
	s->parent = NULL;
	gtk_container_remove(GTK_CONTAINER(oldparent), s->immediate);
	uiParentUpdate(oldparent);
}

static void singlePreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	singleWidget *s = (singleWidget *) (c->internal);
	GtkRequisition natural;

	// use the natural size as the minimum size is an *absolute* minimum
	// for example, if a label has ellipsizing on, it can be the width of the ellipses, not the text
	// there is a warning about height-for-width sizing, but in my tests this isn't an issue
	gtk_widget_get_preferred_size(s->widget, NULL, &natural);
	*width = natural.width;
	*height = natural.height;
}

static void singleResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	singleWidget *s = (singleWidget *) (c->internal);
	GtkAllocation a;

	a.x = x;
	a.y = y;
	a.width = width;
	a.height = height;
	gtk_widget_size_allocate(s->immediate, &a);
}

static int singleVisible(uiControl *c)
{
	singleWidget *s = (singleWidget *) (c->internal);

	if (s->userHid)
		return 0;
	return 1;
}

static void singleShow(uiControl *c)
{
	singleWidget *s = (singleWidget *) (c->internal);

	s->userHid = FALSE;
	if (!s->containerHid) {
		gtk_widget_show_all(s->immediate);
		if (s->parent != NULL)
			uiParentUpdate(s->parent);
	}
}

static void singleHide(uiControl *c)
{
	singleWidget *s = (singleWidget *) (c->internal);

	s->userHid = TRUE;
	gtk_widget_hide(s->immediate);
	if (s->parent != NULL)
		uiParentUpdate(s->parent);
}

static void singleContainerShow(uiControl *c)
{
	singleWidget *s = (singleWidget *) (c->internal);

	s->containerHid = FALSE;
	if (!s->userHid) {
		gtk_widget_show_all(s->immediate);
		if (s->parent != NULL)
			uiParentUpdate(s->parent);
	}
}

static void singleContainerHide(uiControl *c)
{
	singleWidget *s = (singleWidget *) (c->internal);

	s->containerHid = TRUE;
	gtk_widget_hide(s->immediate);
	if (s->parent != NULL)
		uiParentUpdate(s->parent);
}

static void singleEnable(uiControl *c)
{
	singleWidget *s = (singleWidget *) (c->internal);

	s->userDisabled = FALSE;
	if (!s->containerDisabled)
		gtk_widget_set_sensitive(s->immediate, TRUE);
}

static void singleDisable(uiControl *c)
{
	singleWidget *s = (singleWidget *) (c->internal);

	s->userDisabled = TRUE;
	gtk_widget_set_sensitive(s->immediate, FALSE);
}

static void singleContainerEnable(uiControl *c)
{
	singleWidget *s = (singleWidget *) (c->internal);

	s->containerDisabled = FALSE;
	if (!s->userDisabled)
		gtk_widget_set_sensitive(s->immediate, TRUE);
}

static void singleContainerDisable(uiControl *c)
{
	singleWidget *s = (singleWidget *) (c->internal);

	s->containerDisabled = TRUE;
	gtk_widget_set_sensitive(s->immediate, FALSE);
}

static void onDestroy(GtkWidget *widget, gpointer data)
{
	uiControl *c = (uiControl *) data;
	singleWidget *s = (singleWidget *) (c->internal);

	uiFree(s);
	uiFree(c);
}

uiControl *uiUnixNewControl(GType type, gboolean inScrolledWindow, gboolean scrolledWindowHasBorder, const char *firstProperty, ...)
{
	uiControl *c;
	singleWidget *s;
	va_list ap;

	s = uiNew(singleWidget);

	va_start(ap, firstProperty);
	s->widget = GTK_WIDGET(g_object_new_valist(type, firstProperty, ap));
	va_end(ap);
	s->immediate = s->widget;

	if (inScrolledWindow) {
		s->scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
		if (!GTK_IS_SCROLLABLE(s->widget))
			gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(s->scrolledWindow), s->widget);
		else
			gtk_container_add(GTK_CONTAINER(s->scrolledWindow), s->widget);
		if (scrolledWindowHasBorder)
			gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(s->scrolledWindow), GTK_SHADOW_IN);
		s->immediate = s->scrolledWindow;
	}

	// we need to keep an extra reference on the immediate widget
	// this is so uiControlDestroy() can work regardless of when it is called and who calls it
	// without this:
	// - end user call works (only one ref)
	// - call in uiContainer destructor fails (uiContainer ref freed)
	// with this:
	// - end user call works (shoudn't be in any container)
	// - call in uiContainer works (both refs freed)
	// this also ensures singleRemoveParent() works properly
	g_object_ref_sink(s->immediate);

	c = uiNew(uiControl);
	// assign s later; we still need it for one more thing
	c->destroy = singleDestroy;
	c->handle = singleHandle;
	c->setParent = singleSetParent;
	c->removeParent = singleRemoveParent;
	c->preferredSize = singlePreferredSize;
	c->resize = singleResize;
	c->visible = singleVisible;
	c->show = singleShow;
	c->hide = singleHide;
	c->containerShow = singleContainerShow;
	c->containerHide = singleContainerHide;
	c->enable = singleEnable;
	c->disable = singleDisable;
	c->containerEnable = singleContainerEnable;
	c->containerDisable = singleContainerDisable;

	// and let's free everything with the immediate widget
	g_signal_connect(s->immediate, "destroy", G_CALLBACK(onDestroy), c);

	// finally, call gtk_widget_show_all() here to set the initial visibility of the widget
	gtk_widget_show_all(s->immediate);

	c->internal = s;
	return c;
}
