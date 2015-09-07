// 6 september 2015
#include "area.h"

struct uiDrawContext {
	cairo_t *cr;
	gboolean hasPath;
};

uiDrawContext *newContext(cairo_t *cr)
{
	uiDrawContext *c;

	c = (uiDrawContext *) g_malloc0(sizeof (uiDrawContext));
	c->cr = cr;
	c->hasPath = FALSE;
	return c;
}

static void prepPath(uiDrawContext *c)
{
	if (c->hasPath)
		return;
	cairo_new_path(c->cr);
	c->hasPath = TRUE;
}

void uiDrawMoveTo(uiDrawContext *c, intmax_t x, intmax_t y)
{
	prepPath(c);
	cairo_move_to(c->cr, (double) x + 0.5, (double) y + 0.5);
}

void uiDrawLineTo(uiDrawContext *c, intmax_t x, intmax_t y)
{
	prepPath(c);
	cairo_line_to(c->cr, (double) x + 0.5, (double) y + 0.5);
}

#define R(c) (((c) >> 16) & 0xFF)
#define G(c) (((c) >> 8) & 0xFF)
#define B(c) ((c) & 0xFF)

void uiDrawStroke(uiDrawContext *c, uiDrawStrokeParams *p)
{
	cairo_set_source_rgb(c->cr,
		((double) R(p->RGB)) / 255,
		((double) G(p->RGB)) / 255,
		((double) B(p->RGB)) / 255);
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
	cairo_set_line_width(c->cr, p->Thickness);
	cairo_stroke(c->cr);
	c->hasPath = FALSE;
}
