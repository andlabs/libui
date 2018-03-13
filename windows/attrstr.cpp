// 12 february 2017
#include "uipriv_windows.hpp"
#include "attrstr.hpp"

// TODO this whole file needs cleanup

// we need to collect all the background blocks and add them all at once
// TODO contextual alternates override ligatures?
// TODO rename this struct to something that isn't exclusively foreach-ing?
struct foreachParams {
	const uint16_t *s;
	size_t len;
	IDWriteTextLayout *layout;
	std::vector<backgroundFunc> *backgroundFuncs;
};

// we need to combine color and underline style into one unit for IDWriteLayout::SetDrawingEffect()
// we also want to combine identical effects, which DirectWrite doesn't seem to provide a way to do
// we can at least try to goad it into doing so if we can deduplicate effects once they're all computed
// so what we do is use this class to store in-progress effects, much like uiprivCombinedFontAttr on the OS X code
// we then deduplicate them later while converting them into a form suitable for drawing with; see applyEffectsAttributes() below
class combinedEffectsAttr : public IUnknown {
	ULONG refcount;
	uiAttribute *colorAttr;
	uiAttribute *underlineAttr;
	uiAttribute *underlineColorAttr;

	void setAttribute(uiAttribute *a)
	{
		if (a == NULL)
			return;
		switch (uiAttributeGetType(a)) {
		case uiAttributeTypeColor:
			if (this->colorAttr != NULL)
				uiprivAttributeRelease(this->colorAttr);
			this->colorAttr = uiprivAttributeRetain(a);
			break;
		case uiAttributeTypeUnderline:
			if (this->underlineAttr != NULL)
				uiprivAttributeRelease(this->underlineAttr);
			this->underlineAttr = uiprivAttributeRetain(a);
			break;
		case uiAttributeTypeUnderlineColor:
			if (this->underlineAttr != NULL)
				uiprivAttributeRelease(this->underlineAttr);
			this->underlineColorAttr = uiprivAttributeRetain(a);
			break;
		}
	}
public:
	combinedEffectsAttr(uiAttribute *a)
	{
		this->refcount = 1;
		this->colorAttr = NULL;
		this->underlineAttr = NULL;
		this->underlineColorAttr = NULL;
		this->setAttribute(a);
	}

	~combinedEffectsAttr()
	{
		if (this->colorAttr != NULL)
			uiprivAttributeRelease(this->colorAttr);
		if (this->underlineAttr != NULL)
			uiprivAttributeRelease(this->underlineAttr);
		if (this->underlineColorAttr != NULL)
			uiprivAttributeRelease(this->underlineColorAttr);
	}

	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
	{
		if (ppvObject == NULL)
			return E_POINTER;
		if (riid == IID_IUnknown) {
			this->AddRef();
			*ppvObject = this;
			return S_OK;
		}
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		this->refcount++;
		return this->refcount;
	}

	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		this->refcount--;
		if (this->refcount == 0) {
			delete this;
			return 0;
		}
		return this->refcount;
	}

	combinedEffectsAttr *cloneWith(uiAttribute *a)
	{
		combinedEffectsAttr *b;

		b = new combinedEffectsAttr(this->colorAttr);
		b->setAttribute(this->underlineAttr);
		b->setAttribute(this->underlineColorAttr);
		b->setAttribute(a);
		return b;
	}
};

static HRESULT addEffectAttributeToRange(struct foreachParams *p, size_t start, size_t end, uiAttribute *attr)
{
	IUnknown *u;
	combinedEffectsAttr *cea;
	DWRITE_TEXT_RANGE range;
	size_t diff;
	HRESULT hr;

	while (start < end) {
		hr = p->layout->GetDrawingEffect(start, &u, &range);
		if (hr != S_OK)
{logHRESULT(L"HELP", hr);
			return hr;
}		cea = (combinedEffectsAttr *) u;
		if (cea == NULL)
			cea = new combinedEffectsAttr(attr);
		else
			cea = cea->cloneWith(attr);
		// clamp range within [start, end)
		if (range.startPosition < start) {
			diff = start - range.startPosition;
			range.startPosition = start;
			range.length -= diff;
		}
		if ((range.startPosition + range.length) > end)
			range.length = end - range.startPosition;
		hr = p->layout->SetDrawingEffect(cea, range);
		if (hr != S_OK)
			return hr;
		// TODO figure out what and when needs to be released
		start += range.length;
	}
	return S_OK;
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

static uiForEach processAttribute(const uiAttributedString *s, const uiAttribute *attr, size_t start, size_t end, void *data)
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
	// TODO fix const correctness
	start = attrstrUTF8ToUTF16((uiAttributedString *) s, start);
	end = attrstrUTF8ToUTF16((uiAttributedString *) s, end);
	range.startPosition = start;
	range.length = end - start;
	switch (uiAttributeGetType(attr)) {
	case uiAttributeTypeFamily:
		wfamily = toUTF16(uiAttributeFamily(attr));
		hr = p->layout->SetFontFamilyName(wfamily, range);
		if (hr != S_OK)
			logHRESULT(L"error applying family name attribute", hr);
		uiFree(wfamily);
		break;
	case uiAttributeTypeSize:
		hr = p->layout->SetFontSize(
// TODO unify with fontmatch.cpp and/or attrstr.hpp
#define pointSizeToDWriteSize(size) (size * (96.0 / 72.0))
			pointSizeToDWriteSize(uiAttributeSize(attr)),
			range);
		if (hr != S_OK)
			logHRESULT(L"error applying size attribute", hr);
		break;
	case uiAttributeTypeWeight:
		hr = p->layout->SetFontWeight(
			uiprivWeightToDWriteWeight(uiAttributeWeight(attr)),
			range);
		if (hr != S_OK)
			logHRESULT(L"error applying weight attribute", hr);
		break;
	case uiAttributeTypeItalic:
		hr = p->layout->SetFontStyle(
			uiprivItalicToDWriteStyle(uiAttributeItalic(attr)),
			range);
		if (hr != S_OK)
			logHRESULT(L"error applying italic attribute", hr);
		break;
	case uiAttributeTypeStretch:
		hr = p->layout->SetFontStretch(
			uiprivStretchToDWriteStretch(uiAttributeStretch(attr)),
			range);
		if (hr != S_OK)
			logHRESULT(L"error applying stretch attribute", hr);
		break;
	case uiAttributeTypeUnderline:
		// mark that we have an underline; otherwise, DirectWrite will never call our custom renderer's DrawUnderline() method
		hasUnderline = FALSE;
		if (uiAttributeUnderline(attr) != uiUnderlineNone)
			hasUnderline = TRUE;
		hr = p->layout->SetUnderline(hasUnderline, range);
		if (hr != S_OK)
			logHRESULT(L"error applying underline attribute", hr);
		// and fall through to set the underline style through the drawing effect
	case uiAttributeTypeColor:
	case uiAttributeTypeUnderlineColor:
		hr = addEffectAttributeToRange(p, start, end, attr);
		if (hr != S_OK)
			logHRESULT(L"error applying effect (color, underline, or underline color) attribute", hr);
		break;
	case uiAttributeBackground:
		p->backgroundFuncs->push_back(
			mkBackgroundFunc(ostart, oend,
				spec->R, spec->G, spec->B, spec->A));
		break;
#if 0
TODO
		switch (spec->Value) {
		case uiDrawUnderlineColorCustom:
			x(p, start, end, [=](textDrawingEffect *t) {
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
			x(p, start, end, [=](textDrawingEffect *t) {
				t->hasUnderlineColor = true;
				t->ur = 1.0;
				t->ug = 0.0;
				t->ub = 0.0;
				t->ua = 1.0;
			});
			break;
		case uiDrawUnderlineColorGrammar:
			x(p, start, end, [=](textDrawingEffect *t) {
				t->hasUnderlineColor = true;
				t->ur = 0.0;
				t->ug = 1.0;
				t->ub = 0.0;
				t->ua = 1.0;
			});
			break;
		case uiDrawUnderlineColorAuxiliary:
			x(p, start, end, [=](textDrawingEffect *t) {
				t->hasUnderlineColor = true;
				t->ur = 0.0;
				t->ug = 0.0;
				t->ub = 1.0;
				t->ua = 1.0;
			});
			break;
		}
		break;
#endif
	case uiAttributeTypeFeatures:
		// only generate an attribute if not NULL
		// TODO do we still need to do this or not...
		if (uiAttributeFeatures(attr) == NULL)
			break;
		dt = uiprivOpenTypeFeaturesToIDWriteTypography(uiAttributeFeatures(attr));
		hr = p->layout->SetTypography(dt, range);
		if (hr != S_OK)
			logHRESULT(L"error applying features attribute", hr);
		dt->Release();
		break;
	}
	return uiForEachContinue;
}

$$$$TODO CONTINUE HERE

static void applyEffectsAttributes(struct foreachParams *p)
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
