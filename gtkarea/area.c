// 4 september 2015
#include "area.h"

struct areaPrivate {
	uiArea *a;
	uiAreaHandler *ah;

	GtkAdjustment *ha;
	GtkAdjustment *va;
	int clientWidth;
	int clientHeight;
	// needed for GtkScrollable
	GtkScrollablePolicy hpolicy, vpolicy;
};

static void areaWidget_scrollable_init(GtkScrollable *);

G_DEFINE_ABSTRACT_TYPE_WITH_CODE(areaWidget, areaWidget, GTK_TYPE_DRAWING_AREA,
	G_IMPLEMENT_INTERFACE(GTK_TYPE_SCROLLABLE, areaWidget_scrollable_init))

static void updateScroll(areaWidget *a)
{
	struct areaPrivate *ap = a->priv;
	uintmax_t count, pixelsPer;

	// don't call if too early
	if (ap->ha == NULL || ap->va == NULL)
		return;

	(*(ap->ah->HScrollConfig))(ap->ah, ap->a,
		&count, &pixelsPer);
	gtk_adjustment_configure(ap->ha,
		gtk_adjustment_get_value(ap->ha),
		0,
		count,
		1,
		ap->clientWidth / pixelsPer,
		ap->clientWidth / pixelsPer);

	(*(ap->ah->VScrollConfig))(ap->ah, ap->a,
		&count, &pixelsPer);
	gtk_adjustment_configure(ap->va,
		gtk_adjustment_get_value(ap->va),
		0,
		count,
		1,
		ap->clientHeight / pixelsPer,
		ap->clientHeight / pixelsPer);

	// TODO notify adjustment changes?
	g_object_notify(G_OBJECT(a), "hadjustment");
	g_object_notify(G_OBJECT(a), "vadjustment");
}

static void areaWidget_init(areaWidget *a)
{
	a->priv = G_TYPE_INSTANCE_GET_PRIVATE(a, areaWidgetType, struct areaPrivate);

	// for events
	gtk_widget_add_events(GTK_WIDGET(a),
		GDK_POINTER_MOTION_MASK |
		GDK_BUTTON_MOTION_MASK |
		GDK_BUTTON_PRESS_MASK |
		GDK_BUTTON_RELEASE_MASK |
		GDK_KEY_PRESS_MASK |
		GDK_KEY_RELEASE_MASK);

	// for scrolling
	// TODO do we need GDK_TOUCH_MASK?
	gtk_widget_add_events(GTK_WIDGET(a),
		GDK_SCROLL_MASK |
		GDK_TOUCH_MASK |
		GDK_SMOOTH_SCROLL_MASK);

	gtk_widget_set_can_focus(GTK_WIDGET(a), TRUE);
}

static void areaWidget_dispose(GObject *obj)
{
	struct areaPrivate *ap = areaWidget(obj)->priv;

	if (ap->ha != NULL) {
		g_object_unref(ap->ha);
		ap->ha = NULL;
	}
	if (ap->va != NULL) {
		g_object_unref(ap->va);
		ap->va = NULL;
	}
	G_OBJECT_CLASS(areaWidget_parent_class)->dispose(obj);
}

static void areaWidget_finalize(GObject *obj)
{
	G_OBJECT_CLASS(areaWidget_parent_class)->finalize(obj);
}

static void areaWidget_size_allocate(GtkWidget *w, GtkAllocation *allocation)
{
	struct areaPrivate *ap = areaWidget(w)->priv;

	// GtkDrawingArea has a size_allocate() implementation; we need to call it
	// this will call gtk_widget_set_allocation() for us
	GTK_WIDGET_CLASS(areaWidget_parent_class)->size_allocate(w, allocation);
	ap->clientWidth = allocation->width;
	ap->clientHeight = allocation->height;
	updateScroll(areaWidget(w));
}

// TODO draw

// TODO preferred height/width

// TODO events

enum {
	pHAdjustment = 1,
	pVAdjustment,
	pHScrollPolicy,
	pVScrollPolicy,
	nProps,
};

static void onValueChanged(GtkAdjustment *a, gpointer data)
{
	gtk_widget_queue_draw(GTK_WIDGET(data));
}

static void replaceAdjustment(areaWidget *a, GtkAdjustment **adj, const GValue *value)
{
	if (*adj != NULL) {
		g_signal_handlers_disconnect_by_func(*adj, G_CALLBACK(onValueChanged), cc);
		g_object_unref(*adj);
	}
	*adj = GTK_ADJUSTMENT(g_value_get_object(value));
	if (*adj != NULL)
		g_object_ref_sink(*adj);
	else
		*adj = gtk_adjustment_new(0, 0, 0, 0, 0, 0);
	g_signal_connect(*adj, "value-changed", G_CALLBACK(onValueChanged), a);
	updateScroll(a);
}

static void areaWidget_set_property(GObject *obj, guint prop, const GValue *value, GParamSpec *pspec)
{
	areaWidget *a = areaWidget(obj);
	struct areaPrivate *ap = a->priv;

	switch (prop) {
	case pHAdjustment:
		replaceAdjustment(a, &(ap->ha), value);
		return;
	case pVAdjustment:
		replaceAdjustment(a, &(ap->va), value);
		return;
	case pHScrollPolicy:
		ap->hpolicy = g_value_get_enum(value);
		return;
	case pVScrollPolicy:
		ap->vpolicy = g_value_get_enum(value);
		return;
	}
	G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop, pspec);
}

static void areaWidget_get_property(GObject *obj, guint prop, GValue *value, GParamSpec *pspec)
{
	areaWidget *a = areaWidget(obj);
	struct areaPrivate *ap = a->priv;

	switch (prop) {
	case pHAdjustment:
		g_value_set_object(value, ap->ha);
		return;
	case pVAdjustment:
		g_value_set_object(value, ap->va);
		return;
	case pHScrollPolicy:
		g_value_set_enum(value, ap->hpolicy);
		return;
	case pVScrollPolicy:
		g_value_set_enum(value, ap->vpolicy);
		return;
	}
	G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop, pspec);
}

static void areaWidget_class_init(areaWidgetClass *class)
{
	G_OBJECT_CLASS(class)->dispose = areaWidget_dispose;
	G_OBJECT_CLASS(class)->finalize = areaWidget_finalize;
	G_OBJECT_CLASS(class)->set_property = areaWidget_set_property;
	G_OBJECT_CLASS(class)->get_property = areaWidget_get_property;

	GTK_WIDGET_CLASS(class)->size_allocate = areaWidget_size_allocate;
//	GTK_WIDGET_CLASS(class)->draw = areaWidget_draw;
//	GTK_WIDGET_CLASS(class)->get_preferred_height = areaWidget_get_preferred_height;
//	GTK_WIDGET_CLASS(class)->get_preferred_width = areaWidget_get_preferred_width;
//	GTK_WIDGET_CLASS(class)->button_press_event = areaWidget_button_press_event;
//	GTK_WIDGET_CLASS(class)->button_release_event = areaWidget_button_release_event;
//	GTK_WIDGET_CLASS(class)->motion_notify_event = areaWidget_motion_notify_event;
//	GTK_WIDGET_CLASS(class)->key_press_event = areaWidget_key_press_event;

	// this is the actual interface implementation
	g_object_class_override_property(G_OBJECT_CLASS(class), pHAdjustment, "hadjustment");
	g_object_class_override_property(G_OBJECT_CLASS(class), pVAdjustment, "vadjustment");
	g_object_class_override_property(G_OBJECT_CLASS(class), pHScrollPolicy, "hscroll-policy");
	g_object_class_override_property(G_OBJECT_CLASS(class), pVScrollPolicy, "vscroll-policy");
}

static void areaWidget_scrollable_init(GtkScrollable *iface)
{
	// no need to do anything; the interface only has properties
}
