// 17 january 2017
#include "uipriv_windows.hpp"
#include "draw.hpp"
#include "attrstr.hpp"

// TODO verify our renderer is correct, especially with regards to snapping

struct uiDrawTextLayout {
	IDWriteTextFormat *format;
	IDWriteTextLayout *layout;
	std::vector<struct drawTextBackgroundParams *> *backgroundParams;
	// for converting DirectWrite indices from/to byte offsets
	size_t *u8tou16;
	size_t nUTF8;
	size_t *u16tou8;
	size_t nUTF16;
};

// TODO copy notes about DirectWrite DIPs being equal to Direct2D DIPs here

// typographic points are 1/72 inch; this parameter is 1/96 inch
// fortunately Microsoft does this too, in https://msdn.microsoft.com/en-us/library/windows/desktop/dd371554%28v=vs.85%29.aspx
#define pointSizeToDWriteSize(size) (size * (96.0 / 72.0))

// TODO move this and the layout creation stuff to attrstr.cpp like the other ports, or move the other ports into their drawtext.* files
// TODO should be const but then I can't operator[] on it; the real solution is to find a way to do designated array initializers in C++11 but I do not know enough C++ voodoo to make it work (it is possible but no one else has actually done it before)
static std::map<uiDrawTextAlign, DWRITE_TEXT_ALIGNMENT> dwriteAligns = {
	{ uiDrawTextAlignLeft, DWRITE_TEXT_ALIGNMENT_LEADING },
	{ uiDrawTextAlignCenter, DWRITE_TEXT_ALIGNMENT_CENTER },
	{ uiDrawTextAlignRight, DWRITE_TEXT_ALIGNMENT_TRAILING },
};

uiDrawTextLayout *uiDrawNewTextLayout(uiDrawTextLayoutParams *p)
{
	uiDrawTextLayout *tl;
	WCHAR *wDefaultFamily;
	DWRITE_WORD_WRAPPING wrap;
	FLOAT maxWidth;
	HRESULT hr;

	tl = uiprivNew(uiDrawTextLayout);

	wDefaultFamily = toUTF16(p->DefaultFont->Family);
	hr = dwfactory->CreateTextFormat(
		wDefaultFamily, NULL,
		uiprivWeightToDWriteWeight(p->DefaultFont->Weight),
		uiprivItalicToDWriteStyle(p->DefaultFont->Italic),
		uiprivStretchToDWriteStretch(p->DefaultFont->Stretch),
		pointSizeToDWriteSize(p->DefaultFont->Size),
		// see http://stackoverflow.com/questions/28397971/idwritefactorycreatetextformat-failing and https://msdn.microsoft.com/en-us/library/windows/desktop/dd368203.aspx
		// TODO use the current locale?
		L"",
		&(tl->format));
	uiprivFree(wDefaultFamily);
	if (hr != S_OK)
		logHRESULT(L"error creating IDWriteTextFormat", hr);
	hr = tl->format->SetTextAlignment(dwriteAligns[p->Align]);
	if (hr != S_OK)
		logHRESULT(L"error applying text layout alignment", hr);

	hr = dwfactory->CreateTextLayout(
		(const WCHAR *) uiprivAttributedStringUTF16String(p->String), uiprivAttributedStringUTF16Len(p->String),
		tl->format,
		// FLOAT is float, not double, so this should work... TODO
		FLT_MAX, FLT_MAX,
		&(tl->layout));
	if (hr != S_OK)
		logHRESULT(L"error creating IDWriteTextLayout", hr);

	// and set the width
	// this is the only wrapping mode (apart from "no wrap") available prior to Windows 8.1 (TODO verify this fact) (TODO this should be the default anyway)
	wrap = DWRITE_WORD_WRAPPING_WRAP;
	maxWidth = (FLOAT) (p->Width);
	if (p->Width < 0) {
		// TODO is this wrapping juggling even necessary?
		wrap = DWRITE_WORD_WRAPPING_NO_WRAP;
		// setting the max width in this case technically isn't needed since the wrap mode will simply ignore the max width, but let's do it just to be safe
		maxWidth = FLT_MAX;		// see TODO above
	}
	hr = tl->layout->SetWordWrapping(wrap);
	if (hr != S_OK)
		logHRESULT(L"error setting IDWriteTextLayout word wrapping mode", hr);
	hr = tl->layout->SetMaxWidth(maxWidth);
	if (hr != S_OK)
		logHRESULT(L"error setting IDWriteTextLayout max layout width", hr);

	uiprivAttributedStringApplyAttributesToDWriteTextLayout(p, tl->layout, &(tl->backgroundParams));

	// and finally copy the UTF-8/UTF-16 index conversion tables
	tl->u8tou16 = uiprivAttributedStringCopyUTF8ToUTF16Table(p->String, &(tl->nUTF8));
	tl->u16tou8 = uiprivAttributedStringCopyUTF16ToUTF8Table(p->String, &(tl->nUTF16));

	return tl;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *tl)
{
	uiprivFree(tl->u16tou8);
	uiprivFree(tl->u8tou16);
	for (auto p : *(tl->backgroundParams))
		uiprivFree(p);
	delete tl->backgroundParams;
	tl->layout->Release();
	tl->format->Release();
	uiprivFree(tl);
}

// TODO make this shared code somehow
static HRESULT mkSolidBrush(ID2D1RenderTarget *rt, double r, double g, double b, double a, ID2D1SolidColorBrush **brush)
{
	D2D1_BRUSH_PROPERTIES props;
	D2D1_COLOR_F color;

	ZeroMemory(&props, sizeof (D2D1_BRUSH_PROPERTIES));
	props.opacity = 1.0;
	// identity matrix
	props.transform._11 = 1;
	props.transform._22 = 1;
	color.r = r;
	color.g = g;
	color.b = b;
	color.a = a;
	return rt->CreateSolidColorBrush(
		&color,
		&props,
		brush);
}

static ID2D1SolidColorBrush *mustMakeSolidBrush(ID2D1RenderTarget *rt, double r, double g, double b, double a)
{
	ID2D1SolidColorBrush *brush;
	HRESULT hr;

	hr = mkSolidBrush(rt, r, g, b, a, &brush);
	if (hr != S_OK)
		logHRESULT(L"error creating solid brush", hr);
	return brush;
}

// some of the stuff we want to do isn't possible with what DirectWrite provides itself; we need to do it ourselves

drawingEffectsAttr::drawingEffectsAttr(void)
{
	this->refcount = 1;
	this->hasColor = false;
	this->hasUnderline = false;
	this->hasUnderlineColor = false;
}

HRESULT STDMETHODCALLTYPE drawingEffectsAttr::QueryInterface(REFIID riid, void **ppvObject)
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

ULONG STDMETHODCALLTYPE drawingEffectsAttr::AddRef(void)
{
	this->refcount++;
	return this->refcount;
}

ULONG STDMETHODCALLTYPE drawingEffectsAttr::Release(void)
{
	this->refcount--;
	if (this->refcount == 0) {
		delete this;
		return 0;
	}
	return this->refcount;
}

void drawingEffectsAttr::setColor(double r, double g, double b, double a)
{
	this->hasColor = true;
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

void drawingEffectsAttr::setUnderline(uiUnderline u)
{
	this->hasUnderline = true;
	this->u = u;
}

void drawingEffectsAttr::setUnderlineColor(double r, double g, double b, double a)
{
	this->hasUnderlineColor = true;
	this->ur = r;
	this->ug = g;
	this->ub = b;
	this->ua = a;
}

HRESULT drawingEffectsAttr::mkColorBrush(ID2D1RenderTarget *rt, ID2D1SolidColorBrush **b)
{
	if (!this->hasColor) {
		*b = NULL;
		return S_OK;
	}
	return mkSolidBrush(rt, this->r, this->g, this->b, this->a, b);
}

HRESULT drawingEffectsAttr::underline(uiUnderline *u)
{
	if (u == NULL)
		return E_POINTER;
	if (!this->hasUnderline)
		return E_UNEXPECTED;
	*u = this->u;
	return S_OK;
}

HRESULT drawingEffectsAttr::mkUnderlineBrush(ID2D1RenderTarget *rt, ID2D1SolidColorBrush **b)
{
	if (!this->hasUnderlineColor) {
		*b = NULL;
		return S_OK;
	}
	return mkSolidBrush(rt, this->ur, this->ug, this->ub, this->ua, b);
}

// this is based on http://www.charlespetzold.com/blog/2014/01/Character-Formatting-Extensions-with-DirectWrite.html
class textRenderer : public IDWriteTextRenderer {
	ULONG refcount;
	ID2D1RenderTarget *rt;
	BOOL snap;
	ID2D1SolidColorBrush *black;
public:
	textRenderer(ID2D1RenderTarget *rt, BOOL snap, ID2D1SolidColorBrush *black)
	{
		this->refcount = 1;
		this->rt = rt;
		this->snap = snap;
		this->black = black;
	}

	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
	{
		if (ppvObject == NULL)
			return E_POINTER;
		if (riid == IID_IUnknown ||
			riid == __uuidof (IDWritePixelSnapping) ||
			riid == __uuidof (IDWriteTextRenderer)) {
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

	// IDWritePixelSnapping
	virtual HRESULT STDMETHODCALLTYPE GetCurrentTransform(void *clientDrawingContext, DWRITE_MATRIX *transform)
	{
		D2D1_MATRIX_3X2_F d2dtf;

		if (transform == NULL)
			return E_POINTER;
		this->rt->GetTransform(&d2dtf);
		transform->m11 = d2dtf._11;
		transform->m12 = d2dtf._12;
		transform->m21 = d2dtf._21;
		transform->m22 = d2dtf._22;
		transform->dx = d2dtf._31;
		transform->dy = d2dtf._32;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE GetPixelsPerDip(void *clientDrawingContext, FLOAT *pixelsPerDip)
	{
		FLOAT dpix, dpiy;

		if (pixelsPerDip == NULL)
			return E_POINTER;
		this->rt->GetDpi(&dpix, &dpiy);
		*pixelsPerDip = dpix / 96;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(void *clientDrawingContext, BOOL *isDisabled)
	{
		if (isDisabled == NULL)
			return E_POINTER;
		*isDisabled = !this->snap;
		return S_OK;
	}

	// IDWriteTextRenderer
	virtual HRESULT STDMETHODCALLTYPE DrawGlyphRun(void *clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, DWRITE_MEASURING_MODE measuringMode, const DWRITE_GLYPH_RUN *glyphRun, const DWRITE_GLYPH_RUN_DESCRIPTION *glyphRunDescription, IUnknown *clientDrawingEffect)
	{
		D2D1_POINT_2F baseline;
		drawingEffectsAttr *dea = (drawingEffectsAttr *) clientDrawingEffect;
		ID2D1SolidColorBrush *brush;

		baseline.x = baselineOriginX;
		baseline.y = baselineOriginY;
		brush = NULL;
		if (dea != NULL) {
			HRESULT hr;

			hr = dea->mkColorBrush(this->rt, &brush);
			if (hr != S_OK)
				return hr;
		}
		if (brush == NULL) {
			brush = this->black;
			brush->AddRef();
		}
		this->rt->DrawGlyphRun(
			baseline,
			glyphRun,
			brush,
			measuringMode);
		brush->Release();
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DrawInlineObject(void *clientDrawingContext, FLOAT originX, FLOAT originY, IDWriteInlineObject *inlineObject, BOOL isSideways, BOOL isRightToLeft, IUnknown *clientDrawingEffect)
	{
		if (inlineObject == NULL)
			return E_POINTER;
		return inlineObject->Draw(clientDrawingContext, this,
			originX, originY,
			isSideways, isRightToLeft,
			clientDrawingEffect);
	}

	virtual HRESULT STDMETHODCALLTYPE DrawStrikethrough(void *clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, const DWRITE_STRIKETHROUGH *strikethrough, IUnknown *clientDrawingEffect)
	{
		// we don't support strikethrough
		return E_UNEXPECTED;
	}

	// TODO clean this function up
	virtual HRESULT STDMETHODCALLTYPE DrawUnderline(void *clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, const DWRITE_UNDERLINE *underline, IUnknown *clientDrawingEffect)
	{
		drawingEffectsAttr *dea = (drawingEffectsAttr *) clientDrawingEffect;
		uiUnderline utype;
		ID2D1SolidColorBrush *brush;
		D2D1_RECT_F rect;
		D2D1::Matrix3x2F pixeltf;
		FLOAT dpix, dpiy;
		D2D1_POINT_2F pt;
		HRESULT hr;

		if (underline == NULL)
			return E_POINTER;
		if (dea == NULL)		// we can only get here through an underline
			return E_UNEXPECTED;
		hr = dea->underline(&utype);
		if (hr != S_OK)			// we *should* only get here through an underline that's actually set...
			return hr;
		hr = dea->mkUnderlineBrush(this->rt, &brush);
		if (hr != S_OK)
			return hr;
		if (brush == NULL) {
			// TODO document this rule if not already done
			hr = dea->mkColorBrush(this->rt, &brush);
			if (hr != S_OK)
				return hr;
		}
		if (brush == NULL) {
			brush = this->black;
			brush->AddRef();
		}
		rect.left = baselineOriginX;
		rect.top = baselineOriginY + underline->offset;
		rect.right = rect.left + underline->width;
		rect.bottom = rect.top + underline->thickness;
		switch (utype) {
		case uiUnderlineSingle:
			this->rt->FillRectangle(&rect, brush);
			break;
		case uiUnderlineDouble:
			// TODO do any of the matrix methods return errors?
			// TODO standardize double-underline shape across platforms? wavy underline shape?
			this->rt->GetTransform(&pixeltf);
			this->rt->GetDpi(&dpix, &dpiy);
			pixeltf = pixeltf * D2D1::Matrix3x2F::Scale(dpix / 96, dpiy / 96);
			pt.x = 0;
			pt.y = rect.top;
			pt = pixeltf.TransformPoint(pt);
			rect.top = (FLOAT) ((int) (pt.y + 0.5));
			pixeltf.Invert();
			pt = pixeltf.TransformPoint(pt);
			rect.top = pt.y;
			// first line
			rect.top -= underline->thickness;
			// and it seems we need to recompute this
			rect.bottom = rect.top + underline->thickness;
			this->rt->FillRectangle(&rect, brush);
			// second line
			rect.top += 2 * underline->thickness;
			rect.bottom = rect.top + underline->thickness;
			this->rt->FillRectangle(&rect, brush);
			break;
		case uiUnderlineSuggestion:
			{		// TODO get rid of the extra block
					// TODO properly clean resources on failure
					// TODO use fully qualified C overloads for all methods
					// TODO ensure all methods properly have errors handled
				ID2D1PathGeometry *path;
				ID2D1GeometrySink *sink;
				double amplitude, period, xOffset, yOffset;
				double t;
				bool first = true;
				HRESULT hr;

				hr = d2dfactory->CreatePathGeometry(&path);
				if (hr != S_OK)
					return hr;
				hr = path->Open(&sink);
				if (hr != S_OK)
					return hr;
				amplitude = underline->thickness;
				period = 5 * underline->thickness;
				xOffset = baselineOriginX;
				yOffset = baselineOriginY + underline->offset;
				for (t = 0; t < underline->width; t++) {
					double x, angle, y;
					D2D1_POINT_2F pt;

					x = t + xOffset;
					angle = 2 * uiPi * fmod(x, period) / period;
					y = amplitude * sin(angle) + yOffset;
					pt.x = x;
					pt.y = y;
					if (first) {
						sink->BeginFigure(pt, D2D1_FIGURE_BEGIN_HOLLOW);
						first = false;
					} else
						sink->AddLine(pt);
				}
				sink->EndFigure(D2D1_FIGURE_END_OPEN);
				hr = sink->Close();
				if (hr != S_OK)
					return hr;
				sink->Release();
				this->rt->DrawGeometry(path, brush, underline->thickness);
				path->Release();
			}
			break;
		}
		brush->Release();
		return S_OK;
	}
};

// TODO this ignores clipping?
void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y)
{
	D2D1_POINT_2F pt;
	ID2D1SolidColorBrush *black;
	textRenderer *renderer;
	HRESULT hr;

	for (auto p : *(tl->backgroundParams)) {
		// TODO
	}

	// TODO document that fully opaque black is the default text color; figure out whether this is upheld in various scenarios on other platforms
	// TODO figure out if this needs to be cleaned out
	black = mustMakeSolidBrush(c->rt, 0.0, 0.0, 0.0, 1.0);

#define renderD2D 0
#define renderOur 1
#if renderD2D
	pt.x = x;
	pt.y = y;
	// TODO D2D1_DRAW_TEXT_OPTIONS_NO_SNAP?
	// TODO D2D1_DRAW_TEXT_OPTIONS_CLIP?
	// TODO LONGTERM when setting 8.1 as minimum (TODO verify), D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT?
	// TODO what is our pixel snapping setting related to the OPTIONS enum values?
	c->rt->DrawTextLayout(pt, tl->layout, black, D2D1_DRAW_TEXT_OPTIONS_NONE);
#endif
#if renderD2D && renderOur
	// draw ours semitransparent so we can check
	// TODO get the actual color Charles Petzold uses and use that
	black->Release();
	black = mustMakeSolidBrush(c->rt, 1.0, 0.0, 0.0, 0.75);
#endif
#if renderOur
	renderer = new textRenderer(c->rt,
		TRUE,			// TODO FALSE for no-snap?
		black);
	hr = tl->layout->Draw(NULL,
		renderer,
		x, y);
	if (hr != S_OK)
		logHRESULT(L"error drawing IDWriteTextLayout", hr);
	renderer->Release();
#endif

	black->Release();
}

// TODO for a single line the height includes the leading; should it? TextEdit on OS X always includes the leading and/or paragraph spacing, otherwise Klee won't work...
// TODO width does not include trailing whitespace
void uiDrawTextLayoutExtents(uiDrawTextLayout *tl, double *width, double *height)
{
	DWRITE_TEXT_METRICS metrics;
	HRESULT hr;

	hr = tl->layout->GetMetrics(&metrics);
	if (hr != S_OK)
		logHRESULT(L"error getting IDWriteTextLayout layout metrics", hr);
	*width = metrics.width;
	// TODO make sure the behavior of this on empty strings is the same on all platforms (ideally should be 0-width, line height-height; TODO note this in the docs too)
	*height = metrics.height;
}
