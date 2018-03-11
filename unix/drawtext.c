xx 11 march 2018
#import "uipriv_unix.h"
#import "draw.h"
#import "attrstr.h"

struct uiDrawTextLayout {
	PangoLayout *layout;
	GPtrArray *backgroundParams;
};

// we need a context for a few things
// the documentation suggests creating cairo_t-specific, GdkScreen-specific, or even GtkWidget-specific contexts, but we can't really do that because we want our uiDrawTextFonts and uiDrawTextLayouts to be context-independent
// we could use pango_font_map_create_context(pango_cairo_font_map_get_default()) but that will ignore GDK-specific settings
// so let's use gdk_pango_context_get() instead; even though it's for the default screen only, it's good enough for us
#define mkGenericPangoCairoContext() (gdk_pango_context_get())

const PangoStyle uiprivPangoItalics[] = {
	[uiTextItalicNormal] = PANGO_STYLE_NORMAL,
	[uiTextItalicOblique] = PANGO_STYLE_OBLIQUE,
	[uiTextItalicItalic] = PANGO_STYLE_ITALIC,
};

const PangoStretch uiprivPangoStretches[] = {
	[uiTextStretchUltraCondensed] = PANGO_STRETCH_ULTRA_CONDENSED,
	[uiTextStretchExtraCondensed] = PANGO_STRETCH_EXTRA_CONDENSED,
	[uiTextStretchCondensed] = PANGO_STRETCH_CONDENSED,
	[uiTextStretchSemiCondensed] = PANGO_STRETCH_SEMI_CONDENSED,
	[uiTextStretchNormal] = PANGO_STRETCH_NORMAL,
	[uiTextStretchSemiExpanded] = PANGO_STRETCH_SEMI_EXPANDED,
	[uiTextStretchExpanded] = PANGO_STRETCH_EXPANDED,
	[uiTextStretchExtraExpanded] = PANGO_STRETCH_EXTRA_EXPANDED,
	[uiTextStretchUltraExpanded] = PANGO_STRETCH_ULTRA_EXPANDED,
};

static const PangoAlignment pangoAligns[] = {
	[uiDrawTextAlignLeft] = PANGO_ALIGN_LEFT,
	[uiDrawTextAlignCenter] = PANGO_ALIGN_CENTER,
	[uiDrawTextAlignRight] = PANGO_ALIGN_RIGHT,
};

uiDrawTextLayout *uiDrawNewTextLayout(uiDrawTextLayoutParams *p)
{
	uiDrawTextLayout *tl;
	PangoContext *context;
	PangoFontDescription *desc;
	PangoAttrList *attrs;
	int pangoWidth;

	tl = uiprivNew(uiDrawTextLayout);

	// in this case, the context is necessary to create the layout
	// the layout takes a ref on the context so we can unref it afterward
	context = mkGenericPangoCairoContext();
	tl->layout = pango_layout_new(context);
	g_object_unref(context);

	// this is safe; pango_layout_set_text() copies the string
	pango_layout_set_text(tl->layout, uiAttributedStringString(p->String), -1);

	desc = pango_font_description_new();
	pango_font_description_set_family(desc, p->DefaultFont->Family);
	pango_font_description_set_style(desc, uiprivPangoItalics[p->DefaultFont->Italic]);
	// for the most part, pango weights correlate to ours
	// the differences:
	// - Book — libui: 350, Pango: 380
	// - Ultra Heavy — libui: 950, Pango: 1000
	// TODO figure out what to do about this misalignment
	pango_font_description_set_weight(desc, p->DefaultFont->Weight);
	pango_font_description_set_stretch(desc, uiprivPangoStretches[p->DefaultFont->Stretch]);
	// see https://developer.gnome.org/pango/1.30/pango-Fonts.html#pango-font-description-set-size and https://developer.gnome.org/pango/1.30/pango-Glyph-Storage.html#pango-units-from-double
	pango_font_description_set_size(desc, pango_units_from_double(p->DefaultFont->Size));
	pango_layout_set_font_description(tl->layout, desc);
	// this is safe; the description is copied
	pango_font_description_free(desc);

	pangoWidth = cairoToPango(p->Width);
	if (p->Width < 0)
		pangoWidth = -1;
	pango_layout_set_width(tl->layout, pangoWidth);

	pango_layout_set_alignment(tl->layout, pangoAligns[p->Align]);

	attrs = uiprivAttributedStringToPangoAttrList(p, &(tl->backgroundFeatures));
	pango_layout_set_attributes(tl->layout, attrs);
	pango_attr_list_unref(attrs);

	return tl;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *tl)
{
	g_ptr_array_unref(tl->backgroundFeatures);
	g_object_unref(tl->layout);
	uiprivFree(tl);
}

void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y)
{
	guint i;

	for (i = 0; i < tl->backgroundFeatures->len; i++) {
		// TODO
	}
	// TODO have an implicit save/restore on each drawing functions instead? and is this correct?
	cairo_set_source_rgb(c->cr, 0.0, 0.0, 0.0);
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
