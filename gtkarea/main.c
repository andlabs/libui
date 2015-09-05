// 4 september 2015
#include "area.h"

// #qo pkg-config: gtk+-3.0

struct handler {
	uiAreaHandler ah;
};

static GtkWidget *area;
static struct handler h;
static GtkWidget *nhspinb;
static GtkWidget *pphspinb;
static GtkWidget *nvspinb;
static GtkWidget *ppvspinb;

static void handlerHScrollConfig(uiAreaHandler *a, uiArea *area, uintmax_t *n, uintmax_t *pixelsPer)
{
	*n = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(nhspinb));
	*pixelsPer = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pphspinb));
}

static void handlerVScrollConfig(uiAreaHandler *a, uiArea *area, uintmax_t *n, uintmax_t *pixelsPer)
{
	*n = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(nvspinb));
	*pixelsPer = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ppvspinb));
}

static void recalcScroll(GtkSpinButton *sb, gpointer data)
{
	areaUpdateScroll(area);
}

static GtkWidget *makeSpinbox(int min)
{
	GtkWidget *sb;

	sb = gtk_spin_button_new_with_range(min, 100000, 1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(sb), 0);
	g_signal_connect(sb, "value-changed", G_CALLBACK(recalcScroll), NULL);
	return sb;
}

int main(void)
{
	GtkWidget *mainwin;
	GtkWidget *box;
	GtkWidget *scroller;
	GtkWidget *grid;

	h.ah.HScrollConfig = handlerHScrollConfig;
	h.ah.VScrollConfig = handlerVScrollConfig;

	gtk_init(NULL, NULL);

	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(mainwin), 12);
	g_signal_connect(mainwin, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
	gtk_container_add(GTK_CONTAINER(mainwin), box);

	scroller = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_hexpand(scroller, TRUE);
	gtk_widget_set_halign(scroller, GTK_ALIGN_FILL);
	gtk_widget_set_vexpand(scroller, TRUE);
	gtk_widget_set_valign(scroller, GTK_ALIGN_FILL);
	gtk_container_add(GTK_CONTAINER(box), scroller);

	// TODO area

	grid = gtk_grid_new();
	gtk_widget_set_halign(grid, GTK_ALIGN_START);
	gtk_container_add(GTK_CONTAINER(box), grid);

	gtk_grid_attach(GTK_GRID(grid),
		gtk_label_new("H Count"),
		0, 0, 1, 1);
	nhspinb = makeSpinbox(0);
	gtk_grid_attach(GTK_GRID(grid), nhspinb,
		1, 0, 1, 1);

	gtk_grid_attach(GTK_GRID(grid),
		gtk_label_new("H Pixels Per"),
		0, 1, 1, 1);
	pphspinb = makeSpinbox(1);
	gtk_grid_attach(GTK_GRID(grid), pphspinb,
		1, 1, 1, 1);

	gtk_grid_attach(GTK_GRID(grid),
		gtk_label_new("V Count"),
		0, 2, 1, 1);
	nvspinb = makeSpinbox(0);
	gtk_grid_attach(GTK_GRID(grid), nvspinb,
		1, 2, 1, 1);

	gtk_grid_attach(GTK_GRID(grid),
		gtk_label_new("V Pixels Per"),
		0, 3, 1, 1);
	ppvspinb = makeSpinbox(1);
	gtk_grid_attach(GTK_GRID(grid), ppvspinb,
		1, 3, 1, 1);

	area = newArea((uiAreaHandler *) (&h));
	gtk_container_add(GTK_CONTAINER(scroller), area);

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
