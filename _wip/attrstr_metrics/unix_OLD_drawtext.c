// 17 january 2017
#include "uipriv_unix.h"
#include "draw.h"

// TODO
// - if the RTL override is at the beginning of a line, the preceding space is included?
// - nLines == 0: mostly works, except the width is wrong if the paragraph alignment is center or right...
// - TODO check whitespace and line bounds

struct uiDrawTextLayout {
	PangoLayout *layout;
	GPtrArray *backgroundClosures;
	uiDrawTextLayoutLineMetrics *lineMetrics;
	// TODO change everything to use this
	int nLines;
};

// TODO neither these nor the overall extents seem to include trailing whitespace... we need to figure that out too
static void computeLineMetrics(uiDrawTextLayout *tl)
{
	PangoLayoutIter *iter;
	PangoLayoutLine *pll;
	PangoRectangle lineStartPos, lineExtents;
	int i, n;
	uiDrawTextLayoutLineMetrics *m;

	n = tl->nLines;		// TODO remove this variable
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
		// TODO this does not include the last space if any
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

uiDrawTextLayout *uiDrawNewTextLayout(uiDrawTextLayoutParams *p)
{
	uiDrawTextLayout *tl;
	PangoContext *context;
	PangoFontDescription *desc;
	PangoAttrList *attrs;
	int pangoWidth;

	tl = uiNew(uiDrawTextLayout);

	// in this case, the context is necessary to create the layout
	// the layout takes a ref on the context so we can unref it afterward
	context = mkGenericPangoCairoContext();
	tl->layout = pango_layout_new(context);
	g_object_unref(context);

	// this is safe; pango_layout_set_text() copies the string
	pango_layout_set_text(tl->layout, uiAttributedStringString(p->String), -1);

	desc = pango_font_description_new();
	pango_font_description_set_family(desc, p->DefaultFont->Family);
	pango_font_description_set_style(desc, pangoItalics[p->DefaultFont->Italic]);
	// for the most part, pango weights correlate to ours
	// the differences:
	// - Book — libui: 350, Pango: 380
	// - Ultra Heavy — libui: 950, Pango: 1000
	// TODO figure out what to do about this misalignment
	pango_font_description_set_weight(desc, p->DefaultFont->Weight);
	pango_font_description_set_stretch(desc, pangoStretches[p->DefaultFont->Stretch]);
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

	attrs = attrstrToPangoAttrList(p, &(tl->backgroundClosures));
	pango_layout_set_attributes(tl->layout, attrs);
	pango_attr_list_unref(attrs);

	tl->nLines = pango_layout_get_line_count(tl->layout);
	computeLineMetrics(tl);

	return tl;
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

// note: Pango will not let us place the cursor at the end of a line the same way other OSs do; see https://git.gnome.org/browse/pango/tree/pango/pango-layout.c?id=f4cbd27f4e5bf8490ea411190d41813e14f12165#n4204
// ideally there'd be a way to say "I don't need this hack; I'm well behaved" but GTK+ 2 and 3 AND Qt 4 and 5 all behave like this, with the behavior seeming to date back to TkTextView, so...
void uiDrawTextLayoutHitTest(uiDrawTextLayout *tl, double x, double y, size_t *pos, int *line)
{
	int p, trailing;
	int i;

	// this is layout-global, so it takes line origins into account
	pango_layout_xy_to_index(tl->layout,
		cairoToPango(x), cairoToPango(y),
		&p, &trailing);
	// on a trailing hit, align to the nearest cluster
	// fortunately Pango provides that info directly
	if (trailing != 0)
		p += trailing;
	*pos = p;

	for (i = 0; i < tl->nLines; i++) {
		double ltop, lbottom;

		ltop = tl->lineMetrics[i].Y;
		lbottom = ltop + tl->lineMetrics[i].Height;
		// y will already >= ltop at this point since the past lbottom should == ltop
		if (y < lbottom)
			break;
	}
	if (i == pango_layout_get_line_count(tl->layout))
		i--;
	*line = i;
}

double uiDrawTextLayoutByteLocationInLine(uiDrawTextLayout *tl, size_t pos, int line)
{
	PangoLayoutLine *pll;
	gboolean trailing;
	int pangox;

	if (line < 0 || line >= tl->nLines)
		return -1;
	pll = pango_layout_get_line_readonly(tl->layout, line);
	// note: >, not >=, because the position at end is valid!
	if (pos < pll->start_index || pos > (pll->start_index + pll->length))
		return -1;
	// this behavior seems correct
	// there's also PadWrite's TextEditor::GetCaretRect() but that requires state...
	// TODO where does this fail?
	// TODO optimize everything to avoid calling strlen()
	trailing = 0;
	if (pos != 0 && pos != strlen(pango_layout_get_text(tl->layout)) && pos == (pll->start_index + pll->length)) {
		pos--;
		trailing = 1;
	}
	pango_layout_line_index_to_x(pll, pos, trailing, &pangox);
	// TODO unref pll?
	// this is relative to the beginning of the line
	return pangoToCairo(pangox) + tl->lineMetrics[line].X;
}

// note: we can't use gtk_render_insertion_cursor() because that doesn't take information about what line to render on
// we'll just recreate what it does
void caretDrawParams(uiDrawContext *c, double height, struct caretDrawParams *p)
{
	GdkColor *color;
	GdkRGBA rgba;
	gfloat aspectRatio;
	gint width, xoff;

	gtk_style_context_get_style(c->style,
		"cursor-color", &color,
		"cursor-aspect-ratio", &aspectRatio,
		NULL);
	if (color != NULL) {
		p->r = ((double) (color->red)) / 65535.0;
		p->g = ((double) (color->green)) / 65535.0;
		p->b = ((double) (color->blue)) / 65535.0;
		p->a = 1.0;
		gdk_color_free(color);
	} else {
		gtk_style_context_get_color(c->style, GTK_STATE_FLAG_NORMAL, &rgba);
		p->r = rgba.red;
		p->g = rgba.green;
		p->b = rgba.blue;
		p->a = rgba.alpha;
	}

	// GTK+ itself uses integer arithmetic here; let's do the same
	width = height * aspectRatio + 1;
	// TODO this is for LTR
	xoff = width / 2;

	p->xoff = xoff;
	p->width = width;
}
