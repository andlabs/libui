// 11 june 2015
#include "uipriv_unix.h"

#define WIDGET(c) (GTK_WIDGET(uiControlHandle((c))))

static void singleWidgetCommitDestroy(uiControl *c)
{
	g_object_unref(WIDGET(c));
}

static void singleWidgetCommitSetParent(uiControl *c, uiControl *parent)
{
	GtkWidget *newParent;
	GtkWidget *oldParent;

	if (parent == NULL) {
		oldParent = WIDGET(uiControlParent(c));
		gtk_container_remove(GTK_CONTAINER(oldParent), WIDGET(c));
		return;
	}
	newParent = WIDGET(parent);;
	gtk_container_add(GTK_CONTAINER(newParent), WIDGET(c));
}

static void singleWidgetPreferredSize(uiControl *c, uiSizing *d, intmax_t *width, intmax_t *height)
{
	// use the natural height; it makes more sense
	GtkAllocation natural;

	gtk_widget_get_preferred_size(WIDGET(c), NULL, &natural);
	*width = (intmax_t) (natural.width);
	*height = (intmax_t) (natural.height);
}

static void singleWidgetResize(uiControl *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height, uiSizing *d)
{
	GtkAllocation a;

	a.x = x;
	a.y = y;
	a.width = width;
	a.height = height;
	gtk_widget_set_allocation(WIDGET(c), &a);
}

static uiSizing *singleWidgetSizing(uiControl *c)
{
	return uiUnixNewSizing();
}

static void singleWidgetCommitShow(uiControl *c)
{
	gtk_widget_show(WIDGET(c));
}

static void singleWidgetCommitHide(uiControl *c)
{
	gtk_widget_hide(WIDGET(c));
}

static void singleWidgetCommitEnable(uiControl *c)
{
	gtk_widget_set_sensitive(WIDGET(c), TRUE);
}

static void singleWidgetCommitDisable(uiControl *c)
{
	gtk_widget_set_sensitive(WIDGET(c), FALSE);
}

static uintptr_t singleWidgetStartZOrder(uiControl *c)
{
	// we don't need to do anything; GTK+ does it for us
	return 0;
}

static uintptr_t singleWidgetSetZOrder(uiControl *c, uintptr_t insertAfter)
{
	// we don't need to do anything; GTK+ does it for us
	return 0;
}

static int singleWidgetHasTabStops(uiControl *c)
{
	complain("singleWidgetHasTabStops() meaningless on GTK+");
	return 0;		// keep compiler happy
}

// called after creating the control's widget
void uiUnixMakeSingleWidgetControl(uiControl *c, GtkWidget *widget)
{
	// we have to sink the widget so we can reparent it
	g_object_ref_sink(widget);

	uiControl(c)->CommitDestroy = singleWidgetCommitDestroy;
	uiControl(c)->CommitSetParent = singleWidgetCommitSetParent;
	uiControl(c)->PreferredSize = singleWidgetPreferredSize;
	uiControl(c)->Resize = singleWidgetResize;
	uiControl(c)->Sizing = singleWidgetSizing;
	uiControl(c)->CommitShow = singleWidgetCommitShow;
	uiControl(c)->CommitHide = singleWidgetCommitHide;
	uiControl(c)->CommitEnable = singleWidgetCommitEnable;
	uiControl(c)->CommitDisable = singleWidgetCommitDisable;
	uiControl(c)->StartZOrder = singleWidgetStartZOrder;
	uiControl(c)->SetZOrder = singleWidgetSetZOrder;
	uiControl(c)->HasTabStops = singleWidgetHasTabStops;
}

void queueResize(uiControl *c)
{
	gtk_widget_queue_resize(WIDGET(c));
}
