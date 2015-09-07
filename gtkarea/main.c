// 4 september 2015
#include "area.h"

// #qo pkg-config: gtk+-3.0

struct handler {
	uiAreaHandler ah;
};

static GtkWidget *area;
static struct handler h;
static GtkWidget *nhspinb;
static GtkWidget *nvspinb;

static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p)
{
	uiDrawStrokeParams sp;

	uiDrawMoveTo(p->Context, p->ClipX + 5, p->ClipY + 5);
	uiDrawLineTo(p->Context, (p->ClipX + p->ClipWidth) - 5, (p->ClipY + p->ClipHeight) - 5);
	sp.RGB = 0xFF0000;
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, &sp);

	uiDrawMoveTo(p->Context, p->ClipX, p->ClipY);
	uiDrawLineTo(p->Context, p->ClipX + p->ClipWidth, p->ClipY);
	uiDrawLineTo(p->Context, 50, 150);
	uiDrawLineTo(p->Context, 50, 50);
	uiDrawCloseFigure(p->Context);
	sp.RGB = 0x000080;
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinRound;
	sp.Thickness = 5;
	uiDrawStroke(p->Context, &sp);
}

static uintmax_t handlerHScrollMax(uiAreaHandler *a, uiArea *area)
{
	return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(nhspinb));
}

static uintmax_t handlerVScrollMax(uiAreaHandler *a, uiArea *area)
{
	return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(nvspinb));
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

	h.ah.Draw = handlerDraw;
	h.ah.HScrollMax = handlerHScrollMax;
	h.ah.VScrollMax = handlerVScrollMax;

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
		gtk_label_new("V Count"),
		0, 1, 1, 1);
	nvspinb = makeSpinbox(0);
	gtk_grid_attach(GTK_GRID(grid), nvspinb,
		1, 1, 1, 1);

	area = newArea((uiAreaHandler *) (&h));
	gtk_container_add(GTK_CONTAINER(scroller), area);

	gtk_widget_show_all(mainwin);
	gtk_main();
	return 0;
}
