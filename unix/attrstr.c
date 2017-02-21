// 12 february 2017
#include "uipriv_unix.h"

// we need to collect all the OpenType features and background blocks and add them all at once
// TODO this is the wrong approach; it causes Pango to end runs early, meaning attributes like the ligature attributes never get applied properly
// TODO rename this struct to something that isn't exclusively foreach-ing?
struct foreachParams {
	const char *s;
	PangoAttrList *attrs;
	// keys are pointers to size_t maintained by g_new0()/g_free()
	// values are GStrings
	GHashTable *features;
	// TODO use pango's built-in background attribute?
	GPtrArray *backgroundClosures;
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

#define isCodepointStart(b) (((uint8_t) (b)) <= 0x7F || ((uint8_t) (b)) >= 0xC0)

static void ensureFeaturesInRange(struct foreachParams *p, size_t start, size_t end)
{
	size_t i;
	size_t *key;
	GString *new;

	for (i = start; i < end; i++) {
		// don't create redundant entries; see below
		if (!isCodepointStart(p->s[i]))
			continue;
		new = (GString *) g_hash_table_lookup(p->features, &i);
		if (new != NULL)
			continue;
		new = g_string_new("");
		key = g_new0(size_t, 1);
		*key = i;
		g_hash_table_replace(p->features, key, new);
	}
}

struct closureParams {
	size_t start;
	size_t end;
	double r;
	double g;
	double b;
	double a;
};

static void backgroundClosure(uiDrawContext *c, uiDrawTextLayout *layout, double x, double y, gpointer data)
{
	struct closureParams *p = (struct closureParams *) data;
	uiDrawBrush brush;

	brush.Type = uiDrawBrushTypeSolid;
	brush.R = p->r;
	brush.G = p->g;
	brush.B = p->b;
	brush.A = p->a;
	drawTextBackground(c, x, y, layout, p->start, p->end, &brush, 0);
}

static void freeClosureParams(gpointer data, GClosure *closure)
{
	uiFree((struct closureParams *) data);
}

static GClosure *mkBackgroundClosure(size_t start, size_t end, double r, double g, double b, double a)
{
	struct closureParams *p;
	GClosure *closure;

	p = uiNew(struct closureParams);
	p->start = start;
	p->end = end;
	p->r = r;
	p->g = g;
	p->b = b;
	p->a = a;
	closure = (GClosure *) g_cclosure_new(G_CALLBACK(backgroundClosure), p, freeClosureParams);
	// TODO write a specific marshaler
	// TODO or drop the closure stuff entirely
	g_closure_set_marshal(closure, g_cclosure_marshal_generic);
	return closure;
}

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
		// don't use redundant entries; see below
		if (!isCodepointStart(p->p->s[i]))
			continue;
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
	GClosure *closure;
	PangoGravity gravity;
	PangoUnderline underline;
	PangoLanguage *lang;
	struct otParam op;

	switch (spec->Type) {
	case uiAttributeFamily:
		addattr(p, start, end,
			pango_attr_family_new((const char *) (spec->Value)));
		break;
	case uiAttributeSize:
		addattr(p, start, end,
			pango_attr_size_new(cairoToPango(spec->Double)));
		break;
	case uiAttributeWeight:
		// TODO reverse the misalignment from drawtext.c if it is corrected 
		addattr(p, start, end,
			pango_attr_weight_new((PangoWeight) (spec->Value)));
		break;
	case uiAttributeItalic:
		addattr(p, start, end,
			pango_attr_style_new(pangoItalics[(uiDrawTextItalic) (spec->Value)]));
		break;
	case uiAttributeStretch:
		addattr(p, start, end,
			pango_attr_stretch_new(pangoStretches[(uiDrawTextStretch) (spec->Value)]));
		break;
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
	case uiAttributeBackground:
		closure = mkBackgroundClosure(start, end,
			spec->R, spec->G, spec->B, spec->A);
		g_ptr_array_add(p->backgroundClosures, closure);
		break;
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
	size_t n;

	// remove the trailing comma/space
	g_string_truncate(s, s->len - 2);
	// make sure we cover an entire code point
	// otherwise Pango will break apart multi-byte characters, spitting out U+FFFD characters at the invalid points
	n = 1;
	while (!isCodepointStart(p->s[*pos + n]))
		n++;
	addattr(p, *pos, *pos + n,
		FUTURE_pango_attr_font_features_new(s->str));
	return TRUE;		// always delete; we're emptying the map
}

static void applyAndFreeFeatureAttributes(struct foreachParams *p)
{
	g_hash_table_foreach_remove(p->features, applyFeatures, p);
	g_hash_table_destroy(p->features);
}

static void unrefClosure(gpointer data)
{
	g_closure_unref((GClosure *) data);
}

PangoAttrList *attrstrToPangoAttrList(uiDrawTextLayoutParams *p, GPtrArray **backgroundClosures)
{
	struct foreachParams fep;

	fep.s = uiAttributedStringString(p->String);
	fep.attrs = pango_attr_list_new();
	fep.features = g_hash_table_new_full(
		featurePosHash, featurePosEqual,
		g_free, freeFeatureString);
	fep.backgroundClosures = g_ptr_array_new_with_free_func(unrefClosure);
	uiAttributedStringForEachAttribute(p->String, processAttribute, &fep);
	applyAndFreeFeatureAttributes(&fep);
	*backgroundClosures = fep.backgroundClosures;
	return fep.attrs;
}

void invokeBackgroundClosure(GClosure *closure, uiDrawContext *c, uiDrawTextLayout *layout, double x, double y)
{
	GValue values[4] = {
		// the zero-initialization is needed for g_value_init() to work
		G_VALUE_INIT,
		G_VALUE_INIT,
		G_VALUE_INIT,
		G_VALUE_INIT,
	};

	g_value_init(values + 0, G_TYPE_POINTER);
	g_value_set_pointer(values + 0, c);
	g_value_init(values + 1, G_TYPE_POINTER);
	g_value_set_pointer(values + 1, layout);
	g_value_init(values + 2, G_TYPE_DOUBLE);
	g_value_set_double(values + 2, x);
	g_value_init(values + 3, G_TYPE_DOUBLE);
	g_value_set_double(values + 3, y);
	g_closure_invoke(closure,
		NULL,
		4, values,
		NULL);
}
