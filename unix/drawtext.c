// 17 january 2017
#include "uipriv_unix.h"
#include "draw.h"

struct uiDrawTextLayout {
	PangoLayout *layout;
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

	return tl;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *tl)
{
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
	// TODO unref?
}

void uiDrawTextLayoutLineGetMetrics(uiDrawTextLayout *tl, int line, uiDrawTextLayoutLineMetrics *m)
{
	PangoLayoutLine *pll;

	pll = pango_layout_get_line_readonly(tl->layout, line);
	// TODO unref?
}

// TODO
#if 0
{
	PangoLayoutLine *pll;

	pll = pango_layout_get_line_readonly(tl->layout, line);
	// TODO unref?
}
#endif

void uiDrawTextLayoutByteIndexToGraphemeRect(uiDrawTextLayout *tl, size_t pos, int *line, double *x, double *y, double *width, double *height)
{
}

void uiDrawTextLayoutHitTest(uiDrawTextLayout *tl, double x, double y, uiDrawTextLayoutHitTestResult *result)
{
}

void uiDrawTextLayoutByteRangeToRectangle(uiDrawTextLayout *tl, size_t start, size_t end, uiDrawTextLayoutByteRangeRectangle *r)
{
}
