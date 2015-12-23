// 6 september 2015
#include "uipriv_unix.h"

struct uiDrawPath {
	GArray *pieces;
	uiDrawFillMode fillMode;
	gboolean ended;
};

struct piece {
	int type;
	double d[8];
	int b;
};

enum {
	newFigure,
	newFigureArc,
	lineTo,
	arcTo,
	bezierTo,
	closeFigure,
	addRect,
};

uiDrawPath *uiDrawNewPath(uiDrawFillMode mode)
{
	uiDrawPath *p;

	p = uiNew(uiDrawPath);
	p->pieces = g_array_new(FALSE, TRUE, sizeof (struct piece));
	p->fillMode = mode;
	return p;
}

void uiDrawFreePath(uiDrawPath *p)
{
	g_array_free(p->pieces, TRUE);
	uiFree(p);
}

static void add(uiDrawPath *p, struct piece *piece)
{
	if (p->ended)
		complain("path ended in add()");
	g_array_append_vals(p->pieces, piece, 1);
}

void uiDrawPathNewFigure(uiDrawPath *p, double x, double y)
{
	struct piece piece;

	piece.type = newFigure;
	piece.d[0] = x;
	piece.d[1] = y;
	add(p, &piece);
}

void uiDrawPathNewFigureWithArc(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double sweep, int negative)
{
	struct piece piece;

	if (sweep > 2 * M_PI)
		sweep = 2 * M_PI;
	piece.type = newFigureArc;
	piece.d[0] = xCenter;
	piece.d[1] = yCenter;
	piece.d[2] = radius;
	piece.d[3] = startAngle;
	piece.d[4] = sweep;
	piece.b = negative;
	add(p, &piece);
}

void uiDrawPathLineTo(uiDrawPath *p, double x, double y)
{
	struct piece piece;

	piece.type = lineTo;
	piece.d[0] = x;
	piece.d[1] = y;
	add(p, &piece);
}

void uiDrawPathArcTo(uiDrawPath *p, double xCenter, double yCenter, double radius, double startAngle, double sweep, int negative)
{
	struct piece piece;

	if (sweep > 2 * M_PI)
		sweep = 2 * M_PI;
	piece.type = arcTo;
	piece.d[0] = xCenter;
	piece.d[1] = yCenter;
	piece.d[2] = radius;
	piece.d[3] = startAngle;
	piece.d[4] = sweep;
	piece.b = negative;
	add(p, &piece);
}

void uiDrawPathBezierTo(uiDrawPath *p, double c1x, double c1y, double c2x, double c2y, double endX, double endY)
{
	struct piece piece;

	piece.type = bezierTo;
	piece.d[0] = c1x;
	piece.d[1] = c1y;
	piece.d[2] = c2x;
	piece.d[3] = c2y;
	piece.d[4] = endX;
	piece.d[5] = endY;
	add(p, &piece);
}

void uiDrawPathCloseFigure(uiDrawPath *p)
{
	struct piece piece;

	piece.type = closeFigure;
	add(p, &piece);
}

void uiDrawPathAddRectangle(uiDrawPath *p, double x, double y, double width, double height)
{
	struct piece piece;

	piece.type = addRect;
	piece.d[0] = x;
	piece.d[1] = y;
	piece.d[2] = width;
	piece.d[3] = height;
	add(p, &piece);
}

void uiDrawPathEnd(uiDrawPath *p)
{
	p->ended = TRUE;
}

static void runPath(uiDrawPath *p, cairo_t *cr)
{
	guint i;
	struct piece *piece;
	void (*arc)(cairo_t *, double, double, double, double, double);

	if (!p->ended)
		complain("path not ended in runPath()");
	cairo_new_path(cr);
	for (i = 0; i < p->pieces->len; i++) {
		piece = &g_array_index(p->pieces, struct piece, i);
		switch (piece->type) {
		case newFigure:
			cairo_move_to(cr, piece->d[0], piece->d[1]);
			break;
		case newFigureArc:
			cairo_new_sub_path(cr);
			// fall through
		case arcTo:
			arc = cairo_arc;
			if (piece->b)
				arc = cairo_arc_negative;
			(*arc)(cr,
				piece->d[0],
				piece->d[1],
				piece->d[2],
				piece->d[3],
				piece->d[3] + piece->d[4]);
			break;
		case lineTo:
			cairo_line_to(cr, piece->d[0], piece->d[1]);
			break;
		case bezierTo:
			cairo_curve_to(cr,
				piece->d[0],
				piece->d[1],
				piece->d[2],
				piece->d[3],
				piece->d[4],
				piece->d[5]);
			break;
		case closeFigure:
			cairo_close_path(cr);
			break;
		case addRect:
			cairo_rectangle(cr,
				piece->d[0],
				piece->d[1],
				piece->d[2],
				piece->d[3]);
			break;
		}
	}
}

struct uiDrawContext {
	cairo_t *cr;
};

uiDrawContext *newContext(cairo_t *cr)
{
	uiDrawContext *c;

	c = uiNew(uiDrawContext);
	c->cr = cr;
	return c;
}

void freeContext(uiDrawContext *c)
{
	uiFree(c);
}

static cairo_pattern_t *mkbrush(uiDrawBrush *b)
{
	cairo_pattern_t *pat;
	size_t i;

	switch (b->Type) {
	case uiDrawBrushTypeSolid:
		pat = cairo_pattern_create_rgba(b->R, b->G, b->B, b->A);
		break;
	case uiDrawBrushTypeLinearGradient:
		pat = cairo_pattern_create_linear(b->X0, b->Y0, b->X1, b->Y1);
		break;
	case uiDrawBrushTypeRadialGradient:
		// make the start circle radius 0 to make it a point
		pat = cairo_pattern_create_radial(
			b->X0, b->Y0, 0,
			b->X1, b->Y1, b->OuterRadius);
		break;
//	case uiDrawBrushTypeImage:
	}
	if (cairo_pattern_status(pat) != CAIRO_STATUS_SUCCESS)
		complain("error creating pattern in mkbrush(): %s",
			cairo_status_to_string(cairo_pattern_status(pat)));
	switch (b->Type) {
	case uiDrawBrushTypeLinearGradient:
	case uiDrawBrushTypeRadialGradient:
		for (i = 0; i < b->NumStops; i++)
			cairo_pattern_add_color_stop_rgba(pat,
				b->Stops[i].Pos,
				b->Stops[i].R,
				b->Stops[i].G,
				b->Stops[i].B,
				b->Stops[i].A);
	}
	return pat;
}

void uiDrawStroke(uiDrawContext *c, uiDrawPath *path, uiDrawBrush *b, uiDrawStrokeParams *p)
{
	cairo_pattern_t *pat;

	runPath(path, c->cr);
	pat = mkbrush(b);
	cairo_set_source(c->cr, pat);
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
	cairo_set_dash(c->cr, p->Dashes, p->NumDashes, p->DashPhase);
	cairo_stroke(c->cr);
	cairo_pattern_destroy(pat);
}

void uiDrawFill(uiDrawContext *c, uiDrawPath *path, uiDrawBrush *b)
{
	cairo_pattern_t *pat;

	runPath(path, c->cr);
	pat = mkbrush(b);
	cairo_set_source(c->cr, pat);
	switch (path->fillMode) {
	case uiDrawFillModeWinding:
		cairo_set_fill_rule(c->cr, CAIRO_FILL_RULE_WINDING);
		break;
	case uiDrawFillModeAlternate:
		cairo_set_fill_rule(c->cr, CAIRO_FILL_RULE_EVEN_ODD);
		break;
	}
	cairo_fill(c->cr);
	cairo_pattern_destroy(pat);
}

static void m2c(uiDrawMatrix *m, cairo_matrix_t *c)
{
	c->xx = m->M11;
	c->yx = m->M12;
	c->xy = m->M21;
	c->yy = m->M22;
	c->x0 = m->M31;
	c->y0 = m->M32;
}

static void c2m(cairo_matrix_t *c, uiDrawMatrix *m)
{
	m->M11 = c->xx;
	m->M12 = c->yx;
	m->M21 = c->xy;
	m->M22 = c->yy;
	m->M31 = c->x0;
	m->M32 = c->y0;
}

void uiDrawMatrixSetIdentity(uiDrawMatrix *m)
{
	setIdentity(m);
}

void uiDrawMatrixTranslate(uiDrawMatrix *m, double x, double y)
{
	cairo_matrix_t c;

	m2c(m, &c);
	cairo_matrix_translate(&c, x, y);
	c2m(&c, m);
}

void uiDrawMatrixScale(uiDrawMatrix *m, double xCenter, double yCenter, double x, double y)
{
	cairo_matrix_t c;
	double xt, yt;

	m2c(m, &c);
	// TODO explain why the translation must come first
	xt = x;
	yt = y;
	scaleCenter(xCenter, yCenter, &xt, &yt);
	cairo_matrix_translate(&c, xt, yt);
	cairo_matrix_scale(&c, x, y);
	// TODO undo the translation?
	c2m(&c, m);
}

void uiDrawMatrixRotate(uiDrawMatrix *m, double x, double y, double amount)
{
	cairo_matrix_t c;

	m2c(m, &c);
	cairo_matrix_translate(&c, x, y);
	cairo_matrix_rotate(&c, amount);
	// TODO undo the translation? also cocoa backend
	cairo_matrix_translate(&c, -x, -y);
	c2m(&c, m);
}

void uiDrawMatrixSkew(uiDrawMatrix *m, double x, double y, double xamount, double yamount)
{
	fallbackSkew(m, x, y, xamount, yamount);
}

void uiDrawMatrixMultiply(uiDrawMatrix *dest, uiDrawMatrix *src)
{
	cairo_matrix_t c;
	cairo_matrix_t d;

	m2c(dest, &c);
	m2c(src, &d);
	cairo_matrix_multiply(&c, &c, &d);
	c2m(&c, dest);
}

int uiDrawMatrixInvertible(uiDrawMatrix *m)
{
	cairo_matrix_t c;

	m2c(m, &c);
	return cairo_matrix_invert(&c) == CAIRO_STATUS_SUCCESS;
}

int uiDrawMatrixInvert(uiDrawMatrix *m)
{
	cairo_matrix_t c;

	m2c(m, &c);
	if (cairo_matrix_invert(&c) != CAIRO_STATUS_SUCCESS)
		return 0;
	c2m(&c, m);
	return 1;
}

void uiDrawMatrixTransformPoint(uiDrawMatrix *m, double *x, double *y)
{
	cairo_matrix_t c;

	m2c(m, &c);
	cairo_matrix_transform_point(&c, x, y);
}

void uiDrawMatrixTransformSize(uiDrawMatrix *m, double *x, double *y)
{
	cairo_matrix_t c;

	m2c(m, &c);
	cairo_matrix_transform_distance(&c, x, y);
}

void uiDrawTransform(uiDrawContext *c, uiDrawMatrix *m)
{
	cairo_matrix_t cm;

	m2c(m, &cm);
	cairo_transform(c->cr, &cm);
}

void uiDrawClip(uiDrawContext *c, uiDrawPath *path)
{
	runPath(path, c->cr);
	switch (path->fillMode) {
	case uiDrawFillModeWinding:
		cairo_set_fill_rule(c->cr, CAIRO_FILL_RULE_WINDING);
		break;
	case uiDrawFillModeAlternate:
		cairo_set_fill_rule(c->cr, CAIRO_FILL_RULE_EVEN_ODD);
		break;
	}
	cairo_clip(c->cr);
}

void uiDrawSave(uiDrawContext *c)
{
	cairo_save(c->cr);
}

void uiDrawRestore(uiDrawContext *c)
{
	cairo_restore(c->cr);
}

struct uiDrawFontFamilies {
	PangoFontFamily **f;
	int n;
};

uiDrawFontFamilies *uiDrawListFontFamilies(void)
{
	uiDrawFontFamilies *ff;
	PangoFontMap *map;

	ff = uiNew(uiDrawFontFamilies);
	map = pango_cairo_font_map_get_default();
	pango_font_map_list_families(map, &(ff->f), &(ff->n));
	// do not free map; it's a shared resource
	return ff;
}

uintmax_t uiDrawFontFamiliesNumFamilies(uiDrawFontFamilies *ff)
{
	return ff->n;
}

char *uiDrawFontFamiliesFamily(uiDrawFontFamilies *ff, uintmax_t n)
{
	PangoFontFamily *f;

	f = ff->f[n];
	return uiUnixStrdupText(pango_font_family_get_name(f));
}

void uiDrawFreeFontFamilies(uiDrawFontFamilies *ff)
{
	g_free(ff->f);
	uiFree(ff);
}
