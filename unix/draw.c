// 6 september 2015
#include "uipriv_unix.h"
#include "draw.h"

uiDrawContext *uiprivNewContext(cairo_t *cr, GtkStyleContext *style)
{
	uiDrawContext *c;

	c = uiprivNew(uiDrawContext);
	c->cr = cr;
	c->style = style;
	return c;
}

void uiprivFreeContext(uiDrawContext *c)
{
	// free neither cr nor style; we own neither
	uiprivFree(c);
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
		uiprivImplBug("error creating pattern in mkbrush(): %s",
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

	uiprivRunPath(path, c->cr);
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

	uiprivRunPath(path, c->cr);
	pat = mkbrush(b);
	cairo_set_source(c->cr, pat);
	switch (uiprivPathFillMode(path)) {
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

void uiDrawTransform(uiDrawContext *c, uiDrawMatrix *m)
{
	cairo_matrix_t cm;

	uiprivM2C(m, &cm);
	cairo_transform(c->cr, &cm);
}

void uiDrawClip(uiDrawContext *c, uiDrawPath *path)
{
	uiprivRunPath(path, c->cr);
	switch (uiprivPathFillMode(path)) {
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

// bitmap API

uiDrawBitmap* uiDrawNewBitmap(uiDrawContext* c, int width, int height)
{
	uiDrawBitmap* bmp;

	bmp = uiprivNew(uiDrawBitmap);

	bmp->bmp = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
	if (cairo_surface_status(bmp->bmp) != CAIRO_STATUS_SUCCESS)
		uiprivImplBug("error creating bitmap: %s",
			cairo_status_to_string(cairo_surface_status(bmp->bmp)));

	bmp->Width = width;
	bmp->Height = height;
	bmp->Stride = cairo_image_surface_get_stride(bmp->bmp);

	return bmp;
}

void uiDrawBitmapUpdate(uiDrawBitmap* bmp, const void* data)
{
	unsigned char* src = data;
	unsigned char* dst = cairo_image_surface_get_data(bmp->bmp);
	int y;

	if (bmp->Stride == bmp->Width * 4) {
		// stride 'good', can just directly copy
		memcpy(dst, src, bmp->Stride*bmp->Height);
	} else {
		for (y = 0; y < bmp->Height; y++) {
			memcpy(dst, src, bmp->Width * 4);
			src += bmp->Width * 4;
			dst += bmp->Stride;
		}
	}

	cairo_surface_mark_dirty(bmp->bmp);
}

void uiDrawBitmapDraw(uiDrawContext* c, uiDrawBitmap* bmp, uiRect* srcrect, uiRect* dstrect, int filter)
{
	cairo_save(c->cr);
	cairo_rectangle(c->cr, dstrect->X, dstrect->Y, dstrect->Width, dstrect->Height);

	cairo_translate(c->cr, dstrect->X, dstrect->Y);
	if ((dstrect->Width != srcrect->Width) || (dstrect->Height != srcrect->Height))	{
		double sx = dstrect->Width / (double)srcrect->Width;
		double sy = dstrect->Height / (double)srcrect->Height;
		cairo_scale(c->cr, sx, sy);
	}

	cairo_set_source_surface(c->cr, bmp->bmp, -srcrect->X, -srcrect->Y);
	cairo_pattern_set_filter(cairo_get_source(c->cr), filter ? CAIRO_FILTER_BILINEAR : CAIRO_FILTER_NEAREST);
	cairo_clip(c->cr);
	cairo_paint(c->cr);

	cairo_restore(c->cr);
}

void uiDrawFreeBitmap(uiDrawBitmap* bmp)
{
	cairo_surface_destroy(bmp->bmp);
	uiprivFree(bmp);
}
