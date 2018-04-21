// 6 september 2015
#include "uipriv_unix.h"
#include "draw.h"

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

int uiDrawFontFamiliesNumFamilies(uiDrawFontFamilies *ff)
{
	return ff->n;
}

char *uiDrawFontFamiliesFamily(uiDrawFontFamilies *ff, int n)
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

struct uiDrawTextFont {
	PangoFont *f;
};

uiDrawTextFont *mkTextFont(PangoFont *f, gboolean ref)
{
	uiDrawTextFont *font;

	font = uiNew(uiDrawTextFont);
	font->f = f;
	if (ref)
		g_object_ref(font->f);
	return font;
}



PangoFont *pangoDescToPangoFont(PangoFontDescription *pdesc)
{
	PangoFont *f;
	PangoContext *context;

	// in this case, the context is necessary for the metrics to be correct
	context = mkGenericPangoCairoContext();
	f = pango_font_map_load_font(pango_cairo_font_map_get_default(), context, pdesc);
	if (f == NULL) {
		// LONGTERM
		g_error("[libui] no match in pangoDescToPangoFont(); report to andlabs");
	}
	g_object_unref(context);
	return f;
}

uiDrawTextFont *uiDrawLoadClosestFont(const uiDrawTextFontDescriptor *desc)
{
	PangoFont *f;
	PangoFontDescription *pdesc;

	pdesc = pango_font_description_new();
	pango_font_description_set_family(pdesc,
		desc->Family);
	pango_font_description_set_size(pdesc,
		(gint) (desc->Size * PANGO_SCALE));
	pango_font_description_set_weight(pdesc,
		pangoWeights[desc->Weight]);
	pango_font_description_set_style(pdesc,
		pangoItalics[desc->Italic]);
	pango_font_description_set_stretch(pdesc,
		pangoStretches[desc->Stretch]);
	f = pangoDescToPangoFont(pdesc);
	pango_font_description_free(pdesc);
	return mkTextFont(f, FALSE);			// we hold the initial reference; no need to ref
}

void uiDrawFreeTextFont(uiDrawTextFont *font)
{
	g_object_unref(font->f);
	uiFree(font);
}

uintptr_t uiDrawTextFontHandle(uiDrawTextFont *font)
{
	return (uintptr_t) (font->f);
}

void uiDrawTextFontDescribe(uiDrawTextFont *font, uiDrawTextFontDescriptor *desc)
{
	PangoFontDescription *pdesc;

	// this creates a copy; we free it later
	pdesc = pango_font_describe(font->f);

	// TODO

	pango_font_description_free(pdesc);
}

// See https://developer.gnome.org/pango/1.30/pango-Cairo-Rendering.html#pango-Cairo-Rendering.description
// Note that we convert to double before dividing to make sure the floating-point stuff is right
#define pangoToCairo(pango) (((double) (pango)) / PANGO_SCALE)
#define cairoToPango(cairo) ((gint) ((cairo) * PANGO_SCALE))

void uiDrawTextFontGetMetrics(uiDrawTextFont *font, uiDrawTextFontMetrics *metrics)
{
	PangoFontMetrics *pm;

	pm = pango_font_get_metrics(font->f, NULL);
	metrics->Ascent = pangoToCairo(pango_font_metrics_get_ascent(pm));
	metrics->Descent = pangoToCairo(pango_font_metrics_get_descent(pm));
	// Pango doesn't seem to expose this :( Use 0 and hope for the best.
	metrics->Leading = 0;
	metrics->UnderlinePos = pangoToCairo(pango_font_metrics_get_underline_position(pm));
	metrics->UnderlineThickness = pangoToCairo(pango_font_metrics_get_underline_thickness(pm));
	pango_font_metrics_unref(pm);
}

// note: PangoCairoLayouts are tied to a given cairo_t, so we can't store one in this device-independent structure
struct uiDrawTextLayout {
	char *s;
	ptrdiff_t *graphemes;
	PangoFont *defaultFont;
	double width;
	PangoAttrList *attrs;
};

uiDrawTextLayout *uiDrawNewTextLayout(const char *text, uiDrawTextFont *defaultFont, double width)
{
	uiDrawTextLayout *layout;
	PangoContext *context;

	layout = uiNew(uiDrawTextLayout);
	layout->s = g_strdup(text);
	context = mkGenericPangoCairoContext();
	layout->graphemes = graphemes(layout->s, context);
	g_object_unref(context);
	layout->defaultFont = defaultFont->f;
	g_object_ref(layout->defaultFont);		// retain a copy
	uiDrawTextLayoutSetWidth(layout, width);
	layout->attrs = pango_attr_list_new();
	return layout;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *layout)
{
	pango_attr_list_unref(layout->attrs);
	g_object_unref(layout->defaultFont);
	uiFree(layout->graphemes);
	g_free(layout->s);
	uiFree(layout);
}

void uiDrawTextLayoutSetWidth(uiDrawTextLayout *layout, double width)
{
	layout->width = width;
}

static void prepareLayout(uiDrawTextLayout *layout, PangoLayout *pl)
{
	// again, this makes a copy
	desc = pango_font_describe(layout->defaultFont);

	pango_layout_set_attributes(pl, layout->attrs);
}

void uiDrawText(uiDrawContext *c, double x, double y, uiDrawTextLayout *layout)
{
	PangoLayout *pl;

	pl = pango_cairo_create_layout(c->cr);
}

static void addAttr(uiDrawTextLayout *layout, PangoAttribute *attr, int startChar, int endChar)
{
	attr->start_index = layout->graphemes[startChar];
	attr->end_index = layout->graphemes[endChar];
	pango_attr_list_insert(layout->attrs, attr);
	// pango_attr_list_insert() takes attr; we don't free it
}

void uiDrawTextLayoutSetColor(uiDrawTextLayout *layout, int startChar, int endChar, double r, double g, double b, double a)
{
	PangoAttribute *attr;
	guint16 rr, gg, bb, aa;

	rr = (guint16) (r * 65535);
	gg = (guint16) (g * 65535);
	bb = (guint16) (b * 65535);
	aa = (guint16) (a * 65535);

	attr = pango_attr_foreground_new(rr, gg, bb);
	addAttr(layout, attr, startChar, endChar);

	// TODO what if aa == 0?
	attr = FUTURE_pango_attr_foreground_alpha_new(aa);
	if (attr != NULL)
		addAttr(layout, attr, startChar, endChar);
}
