// 12 february 2017
#include "uipriv_unix.h"

// we need to collect all the OpenType features and background blocks and add them all at once
// TODO rename this struct to something that isn't exclusively foreach-ing?
struct foreachParams {
	PangoAttrList *attrs;
	// keys are pointers to size_t maintained by g_new0()/g_free()
	// values are GStrings
	GHashTable *features;
//TODO	GArray *backgroundBlocks;
};

static gboolean featurePosEqual(gconstpointer a, gconstpointer b)
{
	size_t *sa = (size_t *) a;
	size_t *sb = (size_t *) b;

	return *sa == *sb;
}

static guint featurePosHash(gconstpointer n)
{
	size_t *sn = (size_t *) n;

	return (guint) (*sn);
}

static void freeFeatureString(gpointer s)
{
	g_string_free((GString *) s, TRUE);
}

static void ensureFeaturesInRange(struct foreachParams *p, size_t start, size_t end)
{
	size_t i;
	size_t *key;
	GString *new;

	for (i = start; i < end; i++) {
		new = (GString *) g_hash_table_lookup(p->features, &i);
		if (new != NULL)
			continue;
		new = g_string_new("");
		key = g_new0(size_t, 1);
		*key = i;
		g_hash_table_replace(p->features, key, new);
	}
}

#if 0 /* TODO */
static backgroundBlock mkBackgroundBlock(size_t start, size_t end, double r, double g, double b, double a)
{
	return Block_copy(^(uiDrawContext *c, uiDrawTextLayout *layout, double x, double y) {
		uiDrawBrush brush;

		brush.Type = uiDrawBrushTypeSolid;
		brush.R = r;
		brush.G = g;
		brush.B = b;
		brush.A = a;
		drawTextBackground(c, x, y, layout, start, end, &brush, 0);
	});
}
#endif

struct otParam {
	struct foreachParams *p;
	size_t start;
	size_t end;
};

// see https://developer.mozilla.org/en/docs/Web/CSS/font-feature-settings
static void doOpenType(const char *featureTag, uint32_t param, void *data)
{
	struct otParam *p = (struct otParam *) data;
	size_t i;
	GString *s;

	ensureFeaturesInRange(p->p, p->start, p->end);
	for (i = p->start; i < p->end; i++) {
		s = (GString *) g_hash_table_lookup(p->p->features, &i);
		g_string_append_printf(s, "\"%s\" %" PRIu32 ", ",
			featureTag, param);
	}
}

static void addattr(struct foreachParams *p, size_t start, size_t end, PangoAttribute *attr)
{
	if (attr == NULL)		// in case of a future attribute
		return;
	attr->start_index = start;
	attr->end_index = end;
	pango_attr_list_insert(p->attrs, attr);
}

static int processAttribute(uiAttributedString *s, uiAttributeSpec *spec, size_t start, size_t end, void *data)
{
	struct foreachParams *p = (struct foreachParams *) data;
//TODO	backgroundBlock block;
	PangoGravity gravity;
	PangoUnderline underline;
	PangoLanguage *lang;
	struct otParam op;

	switch (spec->Type) {
	case uiAttributeFamily:
		addattr(p, start, end,
			pango_attr_family_new((const char *) (spec->Value)));
		break;
#if 0 /* TODO */
	case uiAttributeSize:
		addattr(p, start, end,
			pango_attr_size_new(cairoToPango(spec->Double)));
		break;
	case uiAttributeWeight:
		ensureFontInRange(p, start, end);
		adjustFontInRange(p, start, end, ^(struct fontParams *fp) {
			fp->desc.Weight = (uiDrawTextWeight) (spec->Value);
		});
		break;
	case uiAttributeItalic:
		ensureFontInRange(p, start, end);
		adjustFontInRange(p, start, end, ^(struct fontParams *fp) {
			fp->desc.Italic = (uiDrawTextItalic) (spec->Value);
		});
		break;
	case uiAttributeStretch:
		ensureFontInRange(p, start, end);
		adjustFontInRange(p, start, end, ^(struct fontParams *fp) {
			fp->desc.Stretch = (uiDrawTextStretch) (spec->Value);
		});
		break;
#endif
	case uiAttributeColor:
		addattr(p, start, end,
			pango_attr_foreground_new(
				(guint16) (spec->R * 65535.0),
				(guint16) (spec->G * 65535.0),
				(guint16) (spec->B * 65535.0)));
		addattr(p, start, end,
			FUTURE_pango_attr_foreground_alpha_new(
				(guint16) (spec->A * 65535.0)));
		break;
#if 0 /* TODO */
	case uiAttributeBackground:
		block = mkBackgroundBlock(ostart, oend,
			spec->R, spec->G, spec->B, spec->A);
		[p->backgroundBlocks addObject:block];
		Block_release(block);
		break;
#endif
	case uiAttributeVerticalForms:
		gravity = PANGO_GRAVITY_SOUTH;
		if (spec->Value != 0)
			gravity = PANGO_GRAVITY_EAST;
		addattr(p, start, end,
			pango_attr_gravity_new(gravity));
		break;
	case uiAttributeUnderline:
		switch (spec->Value) {
		case uiDrawUnderlineStyleNone:
			underline = PANGO_UNDERLINE_NONE;
			break;
		case uiDrawUnderlineStyleSingle:
			underline = PANGO_UNDERLINE_SINGLE;
			break;
		case uiDrawUnderlineStyleDouble:
			underline = PANGO_UNDERLINE_DOUBLE;
			break;
		case uiDrawUnderlineStyleSuggestion:
			underline = PANGO_UNDERLINE_ERROR;
			break;
		}
		addattr(p, start, end,
			pango_attr_underline_new(underline));
		break;
	case uiAttributeUnderlineColor:
		switch (spec->Value) {
		case uiDrawUnderlineColorCustom:
			addattr(p, start, end,
				pango_attr_underline_color_new(
					(guint16) (spec->R * 65535.0),
					(guint16) (spec->G * 65535.0),
					(guint16) (spec->B * 65535.0)));
			break;
		case uiDrawUnderlineColorSpelling:
			// TODO GtkTextView style property error-underline-color
			addattr(p, start, end,
				pango_attr_underline_color_new(65535, 0, 0));
			break;
		case uiDrawUnderlineColorGrammar:
			// TODO find a more appropriate color
			addattr(p, start, end,
				pango_attr_underline_color_new(0, 65535, 0));
			break;
		case uiDrawUnderlineColorAuxiliary:
			// TODO find a more appropriate color
			addattr(p, start, end,
				pango_attr_underline_color_new(0, 0, 65535));
			break;
		}
		break;
	// language strings are specified as BCP 47: https://developer.gnome.org/pango/1.30/pango-Scripts-and-Languages.html#pango-language-from-string https://www.ietf.org/rfc/rfc3066.txt
	case uiAttributeLanguage:
		lang = pango_language_from_string((const char *) (spec->Value));
		addattr(p, start, end,
			pango_attr_language_new(lang));
		// lang *cannot* be freed
		break;
	// TODO
	default:
		// handle typographic features
		op.p = p;
		op.start = start;
		op.end = end;
		// TODO check if unhandled and complain
		specToOpenType(spec, doOpenType, &op);
		break;
	}
	return 0;
}

static gboolean applyFeatures(gpointer key, gpointer value, gpointer data)
{
	struct foreachParams *p = (struct foreachParams *) data;
	size_t *pos = (size_t *) key;
	GString *s = (GString *) value;

	// remove the trailing comma/space
	g_string_truncate(s, s->len - 2);
	addattr(p, *pos, *pos + 1,
		FUTURE_pango_attr_font_features_new(s->str));
	return TRUE;		// always delete; we're emptying the map
}

static void applyAndFreeFeatureAttributes(struct foreachParams *p)
{
	g_hash_table_foreach_remove(p->features, applyFeatures, p);
	g_hash_table_destroy(p->features);
}

PangoAttrList *attrstrToPangoAttrList(uiDrawTextLayoutParams *p/*TODO, NSArray **backgroundBlocks*/)
{
	struct foreachParams fep;

	fep.attrs = pango_attr_list_new();
	fep.features = g_hash_table_new_full(
		featurePosHash, featurePosEqual,
		g_free, freeFeatureString);
	uiAttributedStringForEachAttribute(p->String, processAttribute, &fep);
	applyAndFreeFeatureAttributes(&fep);
	return fep.attrs;
}
