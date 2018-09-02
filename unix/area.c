// 4 september 2015
#include "uipriv_unix.h"
#include "areacommon.h"

// notes:
// - G_DECLARE_DERIVABLE/FINAL_INTERFACE() requires glib 2.44 and that's starting with debian stretch (testing) (GTK+ 3.18) and ubuntu 15.04 (GTK+ 3.14) - debian jessie has 2.42 (GTK+ 3.14)
#define areaWidgetType (areaWidget_get_type())
#define areaWidget(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), areaWidgetType, areaWidget))
#define isAreaWidget(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), areaWidgetType))
#define areaWidgetClass(class) (G_TYPE_CHECK_CLASS_CAST((class), areaWidgetType, areaWidgetClass))
#define isAreaWidgetClass(class) (G_TYPE_CHECK_CLASS_TYPE((class), areaWidget))
#define getAreaWidgetClass(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), areaWidgetType, areaWidgetClass))

G_DEFINE_TYPE(areaWidget, areaWidget, GTK_TYPE_DRAWING_AREA)

static gboolean areaWidget_draw(GtkWidget *w, cairo_t *cr)
{
	uiArea *a = areaWidget(w)->a;
	uiAreaDrawParams dp;
	double clipX0, clipY0, clipX1, clipY1;

	dp.Context = uiprivNewContext(cr,
		gtk_widget_get_style_context(a->widget));

	uiprivLoadAreaSize(a, &(dp.AreaWidth), &(dp.AreaHeight));

	cairo_clip_extents(cr, &clipX0, &clipY0, &clipX1, &clipY1);
	dp.ClipX = clipX0;
	dp.ClipY = clipY0;
	dp.ClipWidth = clipX1 - clipX0;
	dp.ClipHeight = clipY1 - clipY0;

	// no need to save or restore the graphics state to reset transformations; GTK+ does that for us
	(*(a->ah->Draw))(a->ah, a, &dp);

	uiprivFreeContext(dp.Context);
	return FALSE;
}

uiprivUnixAreaAllDefaultsExceptDraw(areaWidget)

enum {
	pArea = 1,
	nProps,
};

static GParamSpec *pspecArea;

static void areaWidget_class_init(areaWidgetClass *class)
{
	G_OBJECT_CLASS(class)->dispose = areaWidget_dispose;
	G_OBJECT_CLASS(class)->finalize = areaWidget_finalize;
	G_OBJECT_CLASS(class)->set_property = areaWidget_set_property;
	G_OBJECT_CLASS(class)->get_property = areaWidget_get_property;

	GTK_WIDGET_CLASS(class)->size_allocate = areaWidget_size_allocate;
	GTK_WIDGET_CLASS(class)->draw = areaWidget_draw;
	GTK_WIDGET_CLASS(class)->get_preferred_height = areaWidget_get_preferred_height;
	GTK_WIDGET_CLASS(class)->get_preferred_width = areaWidget_get_preferred_width;
	GTK_WIDGET_CLASS(class)->button_press_event = areaWidget_button_press_event;
	GTK_WIDGET_CLASS(class)->button_release_event = areaWidget_button_release_event;
	GTK_WIDGET_CLASS(class)->motion_notify_event = areaWidget_motion_notify_event;
	GTK_WIDGET_CLASS(class)->enter_notify_event = areaWidget_enter_notify_event;
	GTK_WIDGET_CLASS(class)->leave_notify_event = areaWidget_leave_notify_event;
	GTK_WIDGET_CLASS(class)->key_press_event = areaWidget_key_press_event;
	GTK_WIDGET_CLASS(class)->key_release_event = areaWidget_key_release_event;

	pspecArea = g_param_spec_pointer("libui-area",
		"libui-area",
		"uiArea.",
		G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
	g_object_class_install_property(G_OBJECT_CLASS(class), pArea, pspecArea);
}

// control implementation

uiUnixControlAllDefaults(uiArea)

void uiAreaSetSize(uiArea *a, int width, int height)
{
	if (!a->scrolling)
		uiprivUserBug("You cannot call uiAreaSetSize() on a non-scrolling uiArea. (area: %p)", a);
	a->scrollWidth = width;
	a->scrollHeight = height;
	gtk_widget_queue_resize(a->areaWidget);
}

void uiAreaQueueRedrawAll(uiArea *a)
{
	gtk_widget_queue_draw(a->areaWidget);
}

void uiAreaScrollTo(uiArea *a, double x, double y, double width, double height)
{
	// TODO
	// TODO adjust adjustments and find source for that
}

void uiAreaBeginUserWindowMove(uiArea *a)
{
	GtkWidget *toplevel;

	if (a->dragevent == NULL)
		uiprivUserBug("cannot call uiAreaBeginUserWindowMove() outside of a Mouse() with Down != 0");
	// TODO don't we have a libui function for this? did I scrap it?
	// TODO widget or areaWidget?
	toplevel = gtk_widget_get_toplevel(a->widget);
	if (toplevel == NULL) {
		// TODO
		return;
	}
	// the docs say to do this
	if (!gtk_widget_is_toplevel(toplevel)) {
		// TODO
		return;
	}
	if (!GTK_IS_WINDOW(toplevel)) {
		// TODO
		return;
	}
	gtk_window_begin_move_drag(GTK_WINDOW(toplevel),
		a->dragevent->button,
		a->dragevent->x_root,		// TODO are these correct?
		a->dragevent->y_root,
		a->dragevent->time);
}

static const GdkWindowEdge edges[] = {
	[uiWindowResizeEdgeLeft] = GDK_WINDOW_EDGE_WEST,
	[uiWindowResizeEdgeTop] = GDK_WINDOW_EDGE_NORTH,
	[uiWindowResizeEdgeRight] = GDK_WINDOW_EDGE_EAST,
	[uiWindowResizeEdgeBottom] = GDK_WINDOW_EDGE_SOUTH,
	[uiWindowResizeEdgeTopLeft] = GDK_WINDOW_EDGE_NORTH_WEST,
	[uiWindowResizeEdgeTopRight] = GDK_WINDOW_EDGE_NORTH_EAST,
	[uiWindowResizeEdgeBottomLeft] = GDK_WINDOW_EDGE_SOUTH_WEST,
	[uiWindowResizeEdgeBottomRight] = GDK_WINDOW_EDGE_SOUTH_EAST,
};

void uiAreaBeginUserWindowResize(uiArea *a, uiWindowResizeEdge edge)
{
	GtkWidget *toplevel;

	if (a->dragevent == NULL)
		uiprivUserBug("cannot call uiAreaBeginUserWindowResize() outside of a Mouse() with Down != 0");
	// TODO don't we have a libui function for this? did I scrap it?
	// TODO widget or areaWidget?
	toplevel = gtk_widget_get_toplevel(a->widget);
	if (toplevel == NULL) {
		// TODO
		return;
	}
	// the docs say to do this
	if (!gtk_widget_is_toplevel(toplevel)) {
		// TODO
		return;
	}
	if (!GTK_IS_WINDOW(toplevel)) {
		// TODO
		return;
	}
	gtk_window_begin_resize_drag(GTK_WINDOW(toplevel),
		edges[edge],
		a->dragevent->button,
		a->dragevent->x_root,		// TODO are these correct?
		a->dragevent->y_root,
		a->dragevent->time);
}

uiArea *uiNewArea(uiAreaHandler *ah)
{
	uiArea *a;

	uiUnixNewControl(uiArea, a);

	a->ah = ah;
	a->scrolling = FALSE;

	a->areaWidget = GTK_WIDGET(g_object_new(areaWidgetType,
		"libui-area", a,
		NULL));
	a->drawingArea = GTK_DRAWING_AREA(a->areaWidget);
	a->area = areaWidget(a->areaWidget);

	a->widget = a->areaWidget;

	return a;
}

uiArea *uiNewScrollingArea(uiAreaHandler *ah, int width, int height)
{
	uiArea *a;

	uiUnixNewControl(uiArea, a);

	a->ah = ah;
	a->scrolling = TRUE;
	a->scrollWidth = width;
	a->scrollHeight = height;

	a->swidget = gtk_scrolled_window_new(NULL, NULL);
	a->scontainer = GTK_CONTAINER(a->swidget);
	a->sw = GTK_SCROLLED_WINDOW(a->swidget);

	a->areaWidget = GTK_WIDGET(g_object_new(areaWidgetType,
		"libui-area", a,
		NULL));
	a->drawingArea = GTK_DRAWING_AREA(a->areaWidget);
	a->area = areaWidget(a->areaWidget);

	a->widget = a->swidget;

	gtk_container_add(a->scontainer, a->areaWidget);
	// and make the area visible; only the scrolled window's visibility is controlled by libui
	gtk_widget_show(a->areaWidget);

	return a;
}
