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
	std::vector<backgroundFunc> *backgroundFuncs;
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

static backgroundFunc mkBackgroundFunc(size_t start, size_t end, double r, double g, double b, double a)
{
	return [=](uiDrawContext *c, uiDrawTextLayout *layout, double x, double y) {
		uiDrawBrush brush;

		brush.Type = uiDrawBrushTypeSolid;
		brush.R = r;
		brush.G = g;
		brush.B = b;
		brush.A = a;
		drawTextBackground(c, x, y, layout, start, end, &brush, 0);
	};
}

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
	size_t ostart, oend;
	BOOL hasUnderline;
	struct otParam op;
	HRESULT hr;

	ostart = start;
	oend = end;
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
	case uiAttributeBackground:
		p->backgroundFuncs->push_back(
			mkBackgroundFunc(ostart, oend,
				spec->R, spec->G, spec->B, spec->A));
		break;
	case uiAttributeUnderline:
		ensureEffectsInRange(p, start, end, [=](textDrawingEffect *t) {
			t->hasUnderline = true;
			t->u = (uiDrawUnderlineStyle) (spec->Value);
		});
		// mark that we have an underline; otherwise, DirectWrite will never call our custom renderer's DrawUnderline() method
		hasUnderline = FALSE;
		if ((uiDrawUnderlineStyle) (spec->Value) != uiDrawUnderlineStyleNone)
			hasUnderline = TRUE;
		hr = p->layout->SetUnderline(hasUnderline, range);
		if (hr != S_OK)
			logHRESULT(L"error applying underline attribute", hr);
		break;
	case uiAttributeUnderlineColor:
		switch (spec->Value) {
		case uiDrawUnderlineColorCustom:
			ensureEffectsInRange(p, start, end, [=](textDrawingEffect *t) {
				t->hasUnderlineColor = true;
				t->ur = spec->R;
				t->ug = spec->G;
				t->ub = spec->B;
				t->ua = spec->A;
			});
			break;
		// TODO see if Microsoft has any standard colors for this
		case uiDrawUnderlineColorSpelling:
			// TODO GtkTextView style property error-underline-color
			ensureEffectsInRange(p, start, end, [=](textDrawingEffect *t) {
				t->hasUnderlineColor = true;
				t->ur = 1.0;
				t->ug = 0.0;
				t->ub = 0.0;
				t->ua = 1.0;
			});
			break;
		case uiDrawUnderlineColorGrammar:
			ensureEffectsInRange(p, start, end, [=](textDrawingEffect *t) {
				t->hasUnderlineColor = true;
				t->ur = 0.0;
				t->ug = 1.0;
				t->ub = 0.0;
				t->ua = 1.0;
			});
			break;
		case uiDrawUnderlineColorAuxiliary:
			ensureEffectsInRange(p, start, end, [=](textDrawingEffect *t) {
				t->hasUnderlineColor = true;
				t->ur = 0.0;
				t->ug = 0.0;
				t->ub = 1.0;
				t->ua = 1.0;
			});
			break;
		}
		break;
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

void attrstrToIDWriteTextLayoutAttrs(uiDrawTextLayoutParams *p, IDWriteTextLayout *layout, std::vector<backgroundFunc> **backgroundFuncs)
{
	struct foreachParams fep;

	fep.s = attrstrUTF16(p->String);
	fep.layout = layout;
	fep.effects = new std::map<size_t, textDrawingEffect *>;
	fep.features = new std::map<size_t, IDWriteTypography *>;
	fep.backgroundFuncs = new std::vector<backgroundFunc>;
	uiAttributedStringForEachAttribute(p->String, processAttribute, &fep);
	applyAndFreeEffectsAttributes(&fep);
	applyAndFreeFeatureAttributes(&fep);
	*backgroundFuncs = fep.backgroundFuncs;
}
