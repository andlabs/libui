// 12 february 2017
#include "uipriv_windows.hpp"
#include "draw.hpp"

// we need to combine color and underline style into one unit for IDWriteLayout::SetDrawingEffect()
// we also need to collect all the OpenType features and background blocks and add them all at once
// TODO(TODO does not seem to apply here) this is the wrong approach; it causes Pango to end runs early, meaning attributes like the ligature attributes never get applied properly
// TODO contextual alternates override ligatures?
// TODO rename this struct to something that isn't exclusively foreach-ing?
struct foreachParams {
	const uint16_t *s;
	IDWriteTextLayout *layout;
	std::map<size_t, textDrawingEffect *> *effects;
	std::map<size_t, IDWriteTypography *> *features;
//TODO	GPtrArray *backgroundClosures;
};

#define isCodepointStart(w) ((((uint16_t) (w)) < 0xDC00) || (((uint16_t) (w)) >= 0xE000))

static void ensureEffectsInRange(struct foreachParams *p, size_t start, size_t end, std::function<void(textDrawingEffect *)> f)
{
	size_t i;
	size_t *key;
	textDrawingEffect *t;

	for (i = start; i < end; i++) {
		// don't create redundant entries; see below
		if (!isCodepointStart(p->s[i]))
			continue;
		t = (*(p->effects))[i];
		if (t != NULL) {
			f(t);
			continue;
		}
		t = new textDrawingEffect;
		f(t);
		(*(p->effects))[i] = t;
	}
}

static void ensureFeaturesInRange(struct foreachParams *p, size_t start, size_t end)
{
	size_t i;
	size_t *key;
	IDWriteTypography *t;
	HRESULT hr;

	for (i = start; i < end; i++) {
		// don't create redundant entries; see below
		if (!isCodepointStart(p->s[i]))
			continue;
		t = (*(p->features))[i];
		if (t != NULL)
			continue;
		hr = dwfactory->CreateTypography(&t);
		if (hr != S_OK)
			logHRESULT(L"error creating IDWriteTypography", hr);
		(*(p->features))[i] = t;
	}
}

#if 0 /* TODO */
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
#endif

struct otParam {
	struct foreachParams *p;
	size_t start;
	size_t end;
};

static void doOpenType(const char *featureTag, uint32_t param, void *data)
{
	struct otParam *p = (struct otParam *) data;
	size_t i;
	IDWriteTypography *t;
	DWRITE_FONT_FEATURE feature;
	HRESULT hr;

	feature.nameTag = (DWRITE_FONT_FEATURE_TAG) DWRITE_MAKE_OPENTYPE_TAG(
		featureTag[0],
		featureTag[1],
		featureTag[2],
		featureTag[3]);
	feature.parameter = param;
	ensureFeaturesInRange(p->p, p->start, p->end);
	for (i = p->start; i < p->end; i++) {
		// don't use redundant entries; see below
		if (!isCodepointStart(p->p->s[i]))
			continue;
		t = (*(p->p->features))[i];
		hr = t->AddFontFeature(feature);
		if (hr != S_OK)
			logHRESULT(L"error adding feature to IDWriteTypography", hr);
	}
}

static int processAttribute(uiAttributedString *s, uiAttributeSpec *spec, size_t start, size_t end, void *data)
{
	struct foreachParams *p = (struct foreachParams *) data;
	DWRITE_TEXT_RANGE range;
	WCHAR *wfamily;
#if 0 /* TODO */
	GClosure *closure;
	PangoGravity gravity;
#endif
	WCHAR *localeName;
	struct otParam op;
	HRESULT hr;

	start = attrstrUTF8ToUTF16(s, start);
	end = attrstrUTF8ToUTF16(s, end);
	range.startPosition = start;
	range.length = end - start;
	switch (spec->Type) {
	case uiAttributeFamily:
		wfamily = toUTF16((char *) (spec->Value));
		hr = p->layout->SetFontFamilyName(wfamily, range);
		if (hr != S_OK)
			logHRESULT(L"error applying family name attribute", hr);
		uiFree(wfamily);
		break;
	case uiAttributeSize:
		hr = p->layout->SetFontSize(
// TODO unify with drawtext.cpp
#define pointSizeToDWriteSize(size) (size * (96.0 / 72.0))
			pointSizeToDWriteSize(spec->Double),
			range);
		if (hr != S_OK)
			logHRESULT(L"error applying size attribute", hr);
		break;
	case uiAttributeWeight:
		// TODO reverse the misalignment from drawtext.cpp if it is corrected 
		hr = p->layout->SetFontWeight(
			(DWRITE_FONT_WEIGHT) (spec->Value),
			range);
		if (hr != S_OK)
			logHRESULT(L"error applying weight attribute", hr);
		break;
	case uiAttributeItalic:
		hr = p->layout->SetFontStyle(
			dwriteItalics[(uiDrawTextItalic) (spec->Value)],
			range);
		if (hr != S_OK)
			logHRESULT(L"error applying italic attribute", hr);
		break;
	case uiAttributeStretch:
		hr = p->layout->SetFontStretch(
			dwriteStretches[(uiDrawTextStretch) (spec->Value)],
			range);
		if (hr != S_OK)
			logHRESULT(L"error applying stretch attribute", hr);
		break;
	case uiAttributeColor:
		ensureEffectsInRange(p, start, end, [=](textDrawingEffect *t) {
			t->hasColor = true;
			t->r = spec->R;
			t->g = spec->G;
			t->b = spec->B;
			t->a = spec->A;
		});
		break;
#if 0
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
#endif
	// locale names are specified as BCP 47: https://msdn.microsoft.com/en-us/library/windows/desktop/dd373814(v=vs.85).aspx https://www.ietf.org/rfc/rfc4646.txt
	case uiAttributeLanguage:
		localeName = toUTF16((char *) (spec->Value));
		hr = p->layout->SetLocaleName(localeName, range);
		if (hr != S_OK)
			logHRESULT(L"error applying locale name attribute", hr);
		uiFree(localeName);
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

static void applyAndFreeEffectsAttributes(struct foreachParams *p)
{
	DWRITE_TEXT_RANGE range;
	HRESULT hr;

	for (auto iter = p->effects->begin(); iter != p->effects->end(); iter++) {
		// make sure we cover an entire code point
		range.startPosition = iter->first;
		range.length = 1;
		if (!isCodepointStart(p->s[iter->first]))
			range.length = 2;
		hr = p->layout->SetDrawingEffect(iter->second, range);
		if (hr != S_OK)
			logHRESULT(L"error applying drawing effects attributes", hr);
		iter->second->Release();
	}
	delete p->effects;
}

static void applyAndFreeFeatureAttributes(struct foreachParams *p)
{
	DWRITE_TEXT_RANGE range;
	HRESULT hr;

	for (auto iter = p->features->begin(); iter != p->features->end(); iter++) {
		// make sure we cover an entire code point
		range.startPosition = iter->first;
		range.length = 1;
		if (!isCodepointStart(p->s[iter->first]))
			range.length = 2;
		hr = p->layout->SetTypography(iter->second, range);
		if (hr != S_OK)
			logHRESULT(L"error applying typographic features attributes", hr);
		iter->second->Release();
	}
	delete p->features;
}

#if 0 /* TODO */
static void unrefClosure(gpointer data)
{
	g_closure_unref((GClosure *) data);
}
#endif

void attrstrToIDWriteTextLayoutAttrs(uiDrawTextLayoutParams *p, IDWriteTextLayout *layout/*TODO, GPtrArray **backgroundClosures*/)
{
	struct foreachParams fep;

	fep.s = attrstrUTF16(p->String);
	fep.layout = layout;
	fep.effects = new std::map<size_t, textDrawingEffect *>;
	fep.features = new std::map<size_t, IDWriteTypography *>;
//TODO	fep.backgroundClosures = g_ptr_array_new_with_free_func(unrefClosure);
	uiAttributedStringForEachAttribute(p->String, processAttribute, &fep);
	applyAndFreeEffectsAttributes(&fep);
	applyAndFreeFeatureAttributes(&fep);
//TODO	*backgroundClosures = fep.backgroundClosures;
}

#if 0
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
#endif
