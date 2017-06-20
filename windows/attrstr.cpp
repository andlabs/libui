// 12 february 2017
#include "uipriv_windows.hpp"
#include "draw.hpp"

// TODO this whole file needs cleanup

// we need to combine color and underline style into one unit for IDWriteLayout::SetDrawingEffect()
// we also need to collect all the background blocks and add them all at once
// TODO contextual alternates override ligatures?
// TODO rename this struct to something that isn't exclusively foreach-ing?
struct foreachParams {
	const uint16_t *s;
	size_t len;
	IDWriteTextLayout *layout;
	std::map<size_t, textDrawingEffect *> *effects;
	std::vector<backgroundFunc> *backgroundFuncs;
};

static void ensureEffectsInRange(struct foreachParams *p, size_t start, size_t end, std::function<void(textDrawingEffect *)> f)
{
	size_t i;
	size_t *key;
	textDrawingEffect *t;

	// TODO explain why we make one for every character
	for (i = start; i < end; i++) {
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

static uiForEach processAttribute(const uiAttributedString *s, const uiAttributeSpec *spec, size_t start, size_t end, void *data)
{
	struct foreachParams *p = (struct foreachParams *) data;
	DWRITE_TEXT_RANGE range;
	WCHAR *wfamily;
	size_t ostart, oend;
	BOOL hasUnderline;
	IDWriteTypography *dt;
	HRESULT hr;

	ostart = start;
	oend = end;
	start = attrstrUTF8ToUTF16(s, start);
	end = attrstrUTF8ToUTF16(s, end);
	range.startPosition = start;
	range.length = end - start;
	switch (spec->Type) {
	case uiAttributeFamily:
		wfamily = toUTF16(spec->Family);
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
	case uiAttributeFeatures:
		// only generate an attribute if spec->Features is not NULL
		if (spec->Features == NULL)
			break;
		dt = otfToDirectWrite(spec->Features);
		hr = p->layout->SetTypography(dt, range);
		if (hr != S_OK)
			logHRESULT(L"error applying features attribute", hr);
		dt->Release();
		break;
	default:
		// TODO complain
		;
	}
	return uiForEachContinue;
}

static void applyAndFreeEffectsAttributes(struct foreachParams *p)
{
	size_t i, n;
	textDrawingEffect *effect, *effectb;
	DWRITE_TEXT_RANGE range;
	static auto apply = [](IDWriteTextLayout *layout, textDrawingEffect *effect, DWRITE_TEXT_RANGE range) {
		HRESULT hr;

		if (effect == NULL)
			return;
		hr = layout->SetDrawingEffect(effect, range);
		if (hr != S_OK)
			logHRESULT(L"error applying drawing effects attributes", hr);
		effect->Release();
	};

	// go through, fililng in the effect attribute for successive ranges of identical textDrawingEffects
	// we are best off treating series of identical effects as single ranges ourselves for parity across platforms, even if Windows does something similar itself
	// this also avoids breaking apart surrogate pairs (though IIRC Windows doing the something similar itself might make this a non-issue here)
	effect = NULL;
	n = p->len;
	range.startPosition = 0;
	for (i = 0; i < n; i++) {
		effectb = (*(p->effects))[i];
		// run of no effect?
		if (effect == NULL && effectb == NULL)
			continue;
		// run of the same effect?
		if (effect != NULL && effectb != NULL)
			if (effect->same(effectb)) {
				effectb->Release();
				continue;
			}

		// the effect has changed; commit the old effect
		range.length = i - range.startPosition;
		apply(p->layout, effect, range);

		range.startPosition = i;
		effect = effectb;
	}
	// and apply the last effect
	range.length = i - range.startPosition;
	apply(p->layout, effect, range);

	delete p->effects;
}

void attrstrToIDWriteTextLayoutAttrs(uiDrawTextLayoutParams *p, IDWriteTextLayout *layout, std::vector<backgroundFunc> **backgroundFuncs)
{
	struct foreachParams fep;

	fep.s = attrstrUTF16(p->String);
	fep.len = attrstrUTF16Len(p->String);
	fep.layout = layout;
	fep.effects = new std::map<size_t, textDrawingEffect *>;
	fep.backgroundFuncs = new std::vector<backgroundFunc>;
	uiAttributedStringForEachAttribute(p->String, processAttribute, &fep);
	applyAndFreeEffectsAttributes(&fep);
	*backgroundFuncs = fep.backgroundFuncs;
}
