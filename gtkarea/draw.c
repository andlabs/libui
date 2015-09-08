// 6 september 2015
#include "area.h"

struct uiDrawContext {
	cairo_t *cr;
};

uiDrawContext *newContext(cairo_t *cr)
{
	uiDrawContext *c;

	// TODO use uiNew
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

void uiDrawBeginPathRGBA(uiDrawContext *c, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	cairo_set_source_rgba(c->cr,
		((double) r) / 255,
		((double) g) / 255,
		((double) b) / 255,
		((double) a) / 255);
	cairo_new_path(c->cr);
}

void uiDrawMoveTo(uiDrawContext *c, intmax_t x, intmax_t y)
{
	cairo_move_to(c->cr, ((double) x) + 0.5, ((double) y) + 0.5);
}

void uiDrawLineTo(uiDrawContext *c, intmax_t x, intmax_t y)
{
	cairo_line_to(c->cr, ((double) x) + 0.5, ((double) y) + 0.5);
}

void uiDrawRectangle(uiDrawContext *c, intmax_t x, intmax_t y, intmax_t width, intmax_t height)
{
	cairo_rectangle(c->cr, ((double) x) + 0.5, ((double) y) + 0.5, width, height);
}

void uiDrawArc(uiDrawContext *c, intmax_t xCenter, intmax_t yCenter, intmax_t radius, double startAngle, double endAngle, int lineFromCurrentPointToStart)
{
	if (!lineFromCurrentPointToStart)
		cairo_new_sub_path(c->cr);
	// the Windows AngleArc() function only goes counterclockwise, so our uiDrawArc() function does too
	// simulate it in cairo by drawing a negative arc from end to start
	cairo_arc_negative(c->cr,
		((double) xCenter) + 0.5,
		((double) yCenter) + 0.5,
		radius,
		endAngle,
		startAngle);
}

void uiDrawBezierTo(uiDrawContext *c, intmax_t c1x, intmax_t c1y, intmax_t c2x, intmax_t c2y, intmax_t endX, intmax_t endY)
{
	cairo_curve_to(c->cr,
		((double) c1x) + 0.5,
		((double) c1y) + 0.5,
		((double) c2x) + 0.5,
		((double) c2y) + 0.5,
		((double) endX) + 0.5,
		((double) endY) + 0.5);
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

void uiDrawFill(uiDrawContext *c, uiDrawFillMode mode)
{
	switch (mode) {
	case uiDrawFillModeWinding:
		cairo_set_fill_rule(c->cr, CAIRO_FILL_RULE_WINDING);
		break;
	case uiDrawFillModeAlternate:
		cairo_set_fill_rule(c->cr, CAIRO_FILL_RULE_EVEN_ODD);
		break;
	}
	cairo_fill(c->cr);
}
