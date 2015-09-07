// 6 september 2015
#include "area.h"

struct uiDrawContext {
	cairo_t *cr;
};

uiDrawContext *newContext(cairo_t *cr)
{
	uiDrawContext *c;

	c = (uiDrawContext *) g_malloc0(sizeof (uiDrawContext));
	c->cr = cr;
	return c;
}

void uiDrawBeginPathRGB(uiDrawContext *c, uint8_t r, uint8_t g, uint8_t b)
{
	cairo_set_source_rgb(c->cr,
		((double) r) / 255,
		((double) g) / 255,
		((double) b) / 255);
	cairo_new_path(c->cr);
}

void uiDrawMoveTo(uiDrawContext *c, intmax_t x, intmax_t y)
{
	cairo_move_to(c->cr, (double) x + 0.5, (double) y + 0.5);
}

void uiDrawLineTo(uiDrawContext *c, intmax_t x, intmax_t y)
{
	cairo_line_to(c->cr, (double) x + 0.5, (double) y + 0.5);
}

void uiDrawCloseFigure(uiDrawContext *c)
{
	cairo_close_path(c->cr);
}

void uiDrawStroke(uiDrawContext *c, uiDrawStrokeParams *p)
{
	switch (p->Cap) {
	case uiDrawLineCapFlat:
		cairo_set_line_cap(c->cr, CAIRO_LINE_CAP_BUTT);
		break;
	case uiDrawLineCapRound:
		cairo_set_line_cap(c->cr, CAIRO_LINE_CAP_ROUND);
		break;
	case uiDrawLineCapSquare:
		cairo_set_line_cap(c->cr, CAIRO_LINE_CAP_SQUARE);
		break;
	}
	switch (p->Join) {
	case uiDrawLineJoinMiter:
		cairo_set_line_join(c->cr, CAIRO_LINE_JOIN_MITER);
		cairo_set_miter_limit(c->cr, p->MiterLimit);
		break;
	case uiDrawLineJoinRound:
		cairo_set_line_join(c->cr, CAIRO_LINE_JOIN_ROUND);
		break;
	case uiDrawLineJoinBevel:
		cairo_set_line_join(c->cr, CAIRO_LINE_JOIN_BEVEL);
		break;
	}
	// TODO comment the /2 here
	cairo_set_line_width(c->cr, ((double) p->Thickness) / 2);
	cairo_stroke(c->cr);
}
