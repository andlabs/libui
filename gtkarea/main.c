// 4 september 2015
#define _GNU_SOURCE
#include <math.h>
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

	uiDrawBeginPathRGB(p->Context, 0xFF, 0x00, 0x00);
	uiDrawMoveTo(p->Context, p->ClipX + 5, p->ClipY + 5);
	uiDrawLineTo(p->Context, (p->ClipX + p->ClipWidth) - 5, (p->ClipY + p->ClipHeight) - 5);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, &sp);

	uiDrawBeginPathRGB(p->Context, 0x00, 0x00, 0xC0);
	uiDrawMoveTo(p->Context, p->ClipX, p->ClipY);
	uiDrawLineTo(p->Context, p->ClipX + p->ClipWidth, p->ClipY);
	uiDrawLineTo(p->Context, 50, 150);
	uiDrawLineTo(p->Context, 50, 50);
	uiDrawCloseFigure(p->Context);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinRound;
	sp.Thickness = 5;
	uiDrawStroke(p->Context, &sp);

	uiDrawBeginPathRGBA(p->Context, 0x00, 0xC0, 0x00, 0x80);
	uiDrawRectangle(p->Context, 120, 80, 50, 50);
	uiDrawFill(p->Context, uiDrawFillModeWinding);

	uiDrawBeginPathRGB(p->Context, 0x00, 0x80, 0x00);
	uiDrawMoveTo(p->Context, 5, 10);
	uiDrawLineTo(p->Context, 5, 50);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, &sp);

	uiDrawBeginPathRGB(p->Context, 0x80, 0xC0, 0x00);
	uiDrawMoveTo(p->Context, 400, 100);
	uiDrawArcTo(p->Context,
		400, 100,
		50,
		30. * (M_PI / 180.),
		// note the end angle here
		// in GDI, the second angle to AngleArc() is relative to the start, not to 0
		330. * (M_PI / 180.),
		1);
	// TODO add a checkbox for this
	uiDrawLineTo(p->Context, 400, 100);
	uiDrawArcTo(p->Context,
		510, 100,
		50,
		30. * (M_PI / 180.),
		330. * (M_PI / 180.),
		0);
	uiDrawCloseFigure(p->Context);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
	uiDrawStroke(p->Context, &sp);

	uiDrawBeginPathRGB(p->Context, 0x00, 0x80, 0xC0);
	uiDrawMoveTo(p->Context, 300, 300);
	uiDrawBezierTo(p->Context,
		350, 320,
		310, 390,
		435, 372);
	sp.Cap = uiDrawLineCapFlat;
	sp.Join = uiDrawLineJoinMiter;
	sp.Thickness = 1;
	sp.MiterLimit = uiDrawDefaultMiterLimit;
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

static int handlerRedrawOnResize(uiAreaHandler *a, uiArea *area)
{
	return 1;
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
	h.ah.RedrawOnResize = handlerRedrawOnResize;

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
