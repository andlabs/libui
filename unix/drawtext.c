// 17 january 2017
#include "uipriv_unix.h"
#include "draw.h"

// TODO
// - if the RTL override is at the beginning of a line, the preceding space is included?
// - the space at the end of a line seems to capture the entire end of that line, especially in hit-testing :S it should work like it does on other platforms

struct uiDrawTextLayout {
	PangoLayout *layout;
	uiDrawTextLayoutLineMetrics *lineMetrics;
};

// See https://developer.gnome.org/pango/1.30/pango-Cairo-Rendering.html#pango-Cairo-Rendering.description
// For the conversion, see https://developer.gnome.org/pango/1.30/pango-Glyph-Storage.html#pango-units-to-double and https://developer.gnome.org/pango/1.30/pango-Glyph-Storage.html#pango-units-from-double
#define pangoToCairo(pango) (pango_units_to_double(pango))
#define cairoToPango(cairo) (pango_units_from_double(cairo))

// we need a context for a few things
// the documentation suggests creating cairo_t-specific, GdkScreen-specific, or even GtkWidget-specific contexts, but we can't really do that because we want our uiDrawTextFonts and uiDrawTextLayouts to be context-independent
// we could use pango_font_map_create_context(pango_cairo_font_map_get_default()) but that will ignore GDK-specific settings
// so let's use gdk_pango_context_get() instead; even though it's for the default screen only, it's good enough for us
#define mkGenericPangoCairoContext() (gdk_pango_context_get())

static const PangoStyle pangoItalics[] = {
	[uiDrawTextItalicNormal] = PANGO_STYLE_NORMAL,
	[uiDrawTextItalicOblique] = PANGO_STYLE_OBLIQUE,
	[uiDrawTextItalicItalic] = PANGO_STYLE_ITALIC,
};

static const PangoStretch pangoStretches[] = {
	[uiDrawTextStretchUltraCondensed] = PANGO_STRETCH_ULTRA_CONDENSED,
	[uiDrawTextStretchExtraCondensed] = PANGO_STRETCH_EXTRA_CONDENSED,
	[uiDrawTextStretchCondensed] = PANGO_STRETCH_CONDENSED,
	[uiDrawTextStretchSemiCondensed] = PANGO_STRETCH_SEMI_CONDENSED,
	[uiDrawTextStretchNormal] = PANGO_STRETCH_NORMAL,
	[uiDrawTextStretchSemiExpanded] = PANGO_STRETCH_SEMI_EXPANDED,
	[uiDrawTextStretchExpanded] = PANGO_STRETCH_EXPANDED,
	[uiDrawTextStretchExtraExpanded] = PANGO_STRETCH_EXTRA_EXPANDED,
	[uiDrawTextStretchUltraExpanded] = PANGO_STRETCH_ULTRA_EXPANDED,
};

// TODO neither these nor the overall extents seem to include trailing whitespace... we need to figure that out too
static void computeLineMetrics(uiDrawTextLayout *tl)
{
	PangoLayoutIter *iter;
	PangoLayoutLine *pll;
	PangoRectangle lineStartPos, lineExtents;
	int i, n;
	uiDrawTextLayoutLineMetrics *m;

	n = pango_layout_get_line_count(tl->layout);
	tl->lineMetrics = (uiDrawTextLayoutLineMetrics *) uiAlloc(n * sizeof (uiDrawTextLayoutLineMetrics), "uiDrawTextLayoutLineMetrics[] (text layout)");
	iter = pango_layout_get_iter(tl->layout);

	m = tl->lineMetrics;
	for (i = 0; i < n; i++) {
		int baselineY;

		// TODO we use this instead of _get_yrange() because of the block of text in that function's description about how line spacing is distributed in Pango; we have to worry about this when we start adding line spacing...
		baselineY = pango_layout_iter_get_baseline(iter);
		pll = pango_layout_iter_get_line_readonly(iter);
		pango_layout_index_to_pos(tl->layout, pll->start_index, &lineStartPos);
		pango_layout_line_get_extents(pll, NULL, &lineExtents);
		// TODO unref pll?

		// TODO is this correct for RTL glyphs?
		m->X = pangoToCairo(lineStartPos.x);
		// TODO fix the whole combined not being updated shenanigans in the static build (here because ugh)
		m->Y = pangoToCairo(baselineY - PANGO_ASCENT(lineExtents));
		m->Width = pangoToCairo(lineExtents.width);
		m->Height = pangoToCairo(lineExtents.height);

		m->BaselineY = pangoToCairo(baselineY);
		m->Ascent = pangoToCairo(PANGO_ASCENT(lineExtents));
		m->Descent = pangoToCairo(PANGO_DESCENT(lineExtents));
		m->Leading = 0;		// TODO

		m->ParagraphSpacingBefore = 0;		// TODO
		m->LineHeightSpace = 0;				// TODO
		m->LineSpacing = 0;				// TODO
		m->ParagraphSpacing = 0;			// TODO

		// don't worry about the return value; we're not using this after the last line
		pango_layout_iter_next_line(iter);
		m++;
	}

	pango_layout_iter_free(iter);
}

uiDrawTextLayout *uiDrawNewTextLayout(uiAttributedString *s, uiDrawFontDescriptor *defaultFont, double width)
{
	uiDrawTextLayout *tl;
	PangoContext *context;
	PangoFontDescription *desc;
	int pangoWidth;

	tl = uiNew(uiDrawTextLayout);

	// in this case, the context is necessary to create the layout
	// the layout takes a ref on the context so we can unref it afterward
	context = mkGenericPangoCairoContext();
	tl->layout = pango_layout_new(context);
	g_object_unref(context);

	// this is safe; pango_layout_set_text() copies the string
	pango_layout_set_text(tl->layout, uiAttributedStringString(s), -1);

	desc = pango_font_description_new();
	pango_font_description_set_family(desc, defaultFont->Family);
	pango_font_description_set_style(desc, pangoItalics[defaultFont->Italic]);
	// for the most part, pango weights correlate to ours
	// the differences:
	// - Book — libui: 350, Pango: 380
	// - Ultra Heavy — libui: 950, Pango: 1000
	// TODO figure out what to do about this misalignment
	pango_font_description_set_weight(desc, defaultFont->Weight);
	pango_font_description_set_stretch(desc, pangoStretches[defaultFont->Stretch]);
	// see https://developer.gnome.org/pango/1.30/pango-Fonts.html#pango-font-description-set-size and https://developer.gnome.org/pango/1.30/pango-Glyph-Storage.html#pango-units-from-double
	pango_font_description_set_size(desc, pango_units_from_double(defaultFont->Size));
	pango_layout_set_font_description(tl->layout, desc);
	// this is safe; the description is copied
	pango_font_description_free(desc);

	pangoWidth = cairoToPango(width);
	if (width < 0)
		pangoWidth = -1;
	pango_layout_set_width(tl->layout, pangoWidth);

	// TODO attributes

	computeLineMetrics(tl);

	return tl;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *tl)
{
	uiFree(tl->lineMetrics);
	g_object_unref(tl->layout);
	uiFree(tl);
}

void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y)
{
	cairo_move_to(c->cr, x, y);
	pango_cairo_show_layout(c->cr, tl->layout);
}

void uiDrawTextLayoutExtents(uiDrawTextLayout *tl, double *width, double *height)
{
	PangoRectangle logical;

	pango_layout_get_extents(tl->layout, NULL, &logical);
	*width = pangoToCairo(logical.width);
	*height = pangoToCairo(logical.height);
}

int uiDrawTextLayoutNumLines(uiDrawTextLayout *tl)
{
	return pango_layout_get_line_count(tl->layout);
}

void uiDrawTextLayoutLineByteRange(uiDrawTextLayout *tl, int line, size_t *start, size_t *end)
{
	PangoLayoutLine *pll;

	pll = pango_layout_get_line_readonly(tl->layout, line);
	*start = pll->start_index;
	*end = pll->start_index + pll->length;
	// TODO unref pll?
}

void uiDrawTextLayoutLineGetMetrics(uiDrawTextLayout *tl, int line, uiDrawTextLayoutLineMetrics *m)
{
	*m = tl->lineMetrics[line];
}

// TODO
#if 0
{
	PangoLayoutLine *pll;

	pll = pango_layout_get_line_readonly(tl->layout, line);
	// TODO unref?
}
#endif

void uiDrawTextLayoutHitTest(uiDrawTextLayout *tl, double x, double y, uiDrawTextLayoutHitTestResult *result)
{
	int index;
	int trailing;
	int line;
	double width, height;

	// disregard the return value; we do our own bounds checking
	// TODO see if there's a way we can use some other function (possibly this one again) to extrapolate more precise bounds information
	pango_layout_xy_to_index(tl->layout,
		cairoToPango(x), cairoToPango(y),
		&index, &trailing);
	// figure out what line that was
	pango_layout_index_to_line_x(tl->layout, index, trailing,
		&line, NULL);
	result->Pos = index;
	result->Line = line;

	uiDrawTextLayoutExtents(tl, &width, &height);
	result->XPosition = uiDrawTextLayoutHitTestPositionInside;
	if (x < 0)
		result->XPosition = uiDrawTextLayoutHitTestPositionBefore;
	else if (x >= width)
		result->XPosition = uiDrawTextLayoutHitTestPositionAfter;
	result->YPosition = uiDrawTextLayoutHitTestPositionInside;
	if (y < 0)
		result->YPosition = uiDrawTextLayoutHitTestPositionBefore;
	else if (y >= height)
		result->YPosition = uiDrawTextLayoutHitTestPositionAfter;
}

// TODO consider providing a uiDrawTextLayoutByteIndexToCursorPos() function
// TODO is this correct for RTL?
void uiDrawTextLayoutByteRangeToRectangle(uiDrawTextLayout *tl, size_t start, size_t end, uiDrawTextLayoutByteRangeRectangle *r)
{
	PangoRectangle startRect, endRect;
	int line, index;
	PangoLayoutLine *pll;

	pango_layout_index_to_pos(tl->layout, start, &startRect);

	// figure out what line that was
	// TODO talk about leading edge here
	pango_layout_index_to_line_x(tl->layout, start, 1,
		&line, NULL);
	pll = pango_layout_get_line_readonly(tl->layout, line);

	if (end > (pll->start_index + pll->length))
		end = pll->start_index + pll->length;
	pango_layout_index_to_pos(tl->layout, end, &endRect);

	r->X = pangoToCairo(startRect.x);
	r->Y = tl->lineMetrics[line].Y;
	r->Width = pangoToCairo(endRect.x) - r->X;
	r->Height = tl->lineMetrics[line].Height;

	// and figure out the correct start pos
	pango_layout_line_x_to_index(pll, startRect.x,
		&index, NULL);
	r->RealStart = index;
	r->RealEnd = end;

	// TODO unref pll?
}
