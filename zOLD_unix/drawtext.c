// 11 march 2018
#include "uipriv_unix.h"
#include "draw.h"
#include "attrstr.h"

struct uiDrawTextLayout {
	PangoLayout *layout;
};

// we need a context for a few things
// the documentation suggests creating cairo_t-specific, GdkScreen-specific, or even GtkWidget-specific contexts, but we can't really do that because we want our uiDrawTextFonts and uiDrawTextLayouts to be context-independent
// we could use pango_font_map_create_context(pango_cairo_font_map_get_default()) but that will ignore GDK-specific settings
// so let's use gdk_pango_context_get() instead; even though it's for the default screen only, it's good enough for us
#define mkGenericPangoCairoContext() (gdk_pango_context_get())

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

	desc = uiprivFontDescriptorToPangoFontDescription(p->DefaultFont);
	pango_layout_set_font_description(tl->layout, desc);
	// this is safe; the description is copied
	pango_font_description_free(desc);

	pangoWidth = cairoToPango(p->Width);
	if (p->Width < 0)
		pangoWidth = -1;
	pango_layout_set_width(tl->layout, pangoWidth);

	pango_layout_set_alignment(tl->layout, pangoAligns[p->Align]);

	attrs = uiprivAttributedStringToPangoAttrList(p);
	pango_layout_set_attributes(tl->layout, attrs);
	pango_attr_list_unref(attrs);

	return tl;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *tl)
{
	g_object_unref(tl->layout);
	uiprivFree(tl);
}

void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y)
{
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
