// 3 september 2015
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_32
#define GLIB_VERSION_MAX_ALLOWED GLIB_VERSION_2_32
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_4
#define GDK_VERSION_MAX_ALLOWED GDK_VERSION_3_4
#include <gtk/gtk.h>

#define dateTimePickerType (dateTimePicker_get_type())
#define dateTimePicker(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), dateTimePickerType, dateTimePicker))
#define isDateTimePicker(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), dateTimePickerType))
#define dateTimePickerClass(class) (G_TYPE_CHECK_CLASS_CAST((class), dateTimePickerType, dateTimePickerClass))
#define isDateTimePickerClass(class) (G_TYPE_CHECK_CLASS_TYPE((class), dateTimePickerType))
#define dateTimePickerGetClass(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), dateTimePickerType, dateTimePickerClass))

typedef struct dateTimePicker dateTimePicker;
typedef struct dateTimePickerClass dateTimePickerClass;

struct dateTimePicker {
	GtkContainer parent_instance;
};

struct dateTimePickerClass {
	GtkContainerClass parent_class;
};

G_DEFINE_TYPE(dateTimePicker, dateTimePicker, GTK_TYPE_CONTAINER)

static void dateTimePicker_init(dateTimePicker *d)
{
	GtkStyleContext *context;

	gtk_widget_set_has_window(GTK_WIDGET(d), FALSE);
	context = gtk_widget_get_style_context(GTK_WIDGET(d));
	gtk_style_context_add_class(context, GTK_STYLE_CLASS_ENTRY);
}

static void dateTimePicker_dispose(GObject *obj)
{
	G_OBJECT_CLASS(dateTimePicker_parent_class)->dispose(obj);
}

static void dateTimePicker_finalize(GObject *obj)
{
	G_OBJECT_CLASS(dateTimePicker_parent_class)->finalize(obj);
}

// All of this is to imitate GtkEntry without actually using its code
// TODO when updating to GTK+ 3.10, copy its imitations too

// This is a hard-coded value that has remained unchanged since 3.4.
#define minEntryWidth 150

struct metrics {
	gint ascent;
	gint descent;
	gboolean interiorFocus;
	gint focusWidth;
	GtkBorder border;
};

static struct metrics metrics(dateTimePicker *d)
{
	struct metrics m;
	GtkStyleContext *scontext;
	PangoContext *pcontext;
	GtkStateFlags stateflags;
	PangoFontMetrics *pmetrics;
	GtkBorder padding;

	scontext = gtk_widget_get_style_context(GTK_WIDGET(d));
	pcontext = gtk_widget_get_pango_context(GTK_WIDGET(d));
	stateflags = gtk_widget_get_state_flags(GTK_WIDGET(d));

	pmetrics = pango_context_get_metrics(pcontext,
		gtk_style_context_get_font(scontext, stateflags),
		pango_context_get_language(pcontext));

	m.ascent = pango_font_metrics_get_ascent(pmetrics);
	m.descent = pango_font_metrics_get_descent(pmetrics);

	pango_font_metrics_unref(pmetrics);

	gtk_widget_style_get(GTK_WIDGET(d),
		"interior-focus", &(m.interiorFocus),
		"focus-line-width", &(m.focusWidth),
		NULL);

	gtk_style_context_get_border(scontext, 0, &(m.border));
	gtk_style_context_get_padding(scontext, 0, &padding);
	m.border.left += padding.left;
	m.border.top += padding.top;
	m.border.right += padding.right;
	m.border.bottom += padding.bottom;
	if (!m.interiorFocus) {
		m.border.left += m.focusWidth;
		m.border.top += m.focusWidth;
		m.border.right += m.focusWidth;
		m.border.bottom += m.focusWidth;
	}

	return m;
}

static void dateTimePicker_get_preferred_width(GtkWidget *w, gint *minimum, gint *natural)
{
	dateTimePicker *d = dateTimePicker(w);
	struct metrics m;

	m = metrics(d);
	// TODO base width on content
	*minimum = minEntryWidth + m.border.left + m.border.right;
	*natural = *minimum;
}

static void dateTimePicker_get_preferred_height(GtkWidget *w, gint *minimum, gint *natural)
{
	dateTimePicker *d = dateTimePicker(w);
	struct metrics m;

	m = metrics(d);
	// TODO base width on content
	*minimum = PANGO_PIXELS(m.ascent + m.descent) + m.border.top + m.border.bottom;
	*natural = *minimum;
}

static void dateTimePicker_size_allocate(GtkWidget *w, GtkAllocation *allocation)
{
	gtk_widget_set_allocation(w, allocation);
}

static gint dateTimePicker_draw(GtkWidget *w, cairo_t *cr)
{
	dateTimePicker *d = dateTimePicker(w);
	GtkStyleContext *context;
	gint x, y, width, height;
	struct metrics m;
	GtkAllocation allocation;
	GtkRequisition minimum;
	gint minHeight;

	context = gtk_widget_get_style_context(GTK_WIDGET(d));
	x = 0;
	y = 0;
	m = metrics(d);

	// draw background, shadow, focus, etc.
	cairo_save(cr);
	gtk_widget_get_allocation(GTK_WIDGET(d), &allocation);
	gtk_widget_get_preferred_size(GTK_WIDGET(d), &minimum, NULL);
	minHeight = minimum.height - gtk_widget_get_margin_top(GTK_WIDGET(d)) - gtk_widget_get_margin_bottom(GTK_WIDGET(d));
	x = 0;
	y = (allocation.height - minHeight) / 2;
	width = allocation.width;
	height = minHeight;
	// cairo translate framex framey
	if (gtk_widget_has_focus(GTK_WIDGET(d)) & !m.interiorFocus) {
		x += m.focusWidth;
		y += m.focusWidth;
		width -= 2 * m.focusWidth;
		height -= 2 * m.focusWidth;
	}
	gtk_render_background(context, cr, x, y, width, height);
	gtk_render_frame(context, cr, x, y, width, height);
	if (gtk_widget_has_visible_focus(GTK_WIDGET(d)) & !m.interiorFocus) {
		x -= m.focusWidth;
		y -= m.focusWidth;
		width += 2 * m.focusWidth;
		height += 2 * m.focusWidth;
		// TODO 0,0? this was removed in 3.14...
		gtk_render_focus(context, cr, 0, 0, width, height);
	}
	cairo_restore(cr);

	return FALSE;
}

static void dateTimePicker_class_init(dateTimePickerClass *class)
{
	G_OBJECT_CLASS(class)->dispose = dateTimePicker_dispose;
	G_OBJECT_CLASS(class)->finalize = dateTimePicker_finalize;
	GTK_WIDGET_CLASS(class)->get_preferred_width = dateTimePicker_get_preferred_width;
	GTK_WIDGET_CLASS(class)->get_preferred_height = dateTimePicker_get_preferred_height;
	GTK_WIDGET_CLASS(class)->size_allocate = dateTimePicker_size_allocate;
	GTK_WIDGET_CLASS(class)->draw = dateTimePicker_draw;
//	GTK_CONTAINER_CLASS(class)->forall = dateTimePicker_forall;
}

// #qo pkg-config: gtk+-3.0
int main(void)
{
	GtkWidget *mainwin;
	GtkWidget *dtp;

	gtk_init(NULL, NULL);

	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(mainwin), 12);
	g_signal_connect(mainwin, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	dtp = GTK_WIDGET(g_object_new(dateTimePickerType, NULL));
	gtk_container_add(GTK_CONTAINER(mainwin), dtp);

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
