// 17 january 2017
#include "uipriv_windows.hpp"
#include "draw.hpp"

// TODO
// - consider the warnings about antialiasing in the PadWrite sample
// - if that's not a problem, do we have overlapping rects in the hittest sample? I can't tell...
// - empty string: nLines == 1 and all checks out except extents has x == 0 when not left aligned
// - paragraph alignment is subject to RTL mirroring; see if it is on other platforms
// - add overhang info to metrics?

// TODO verify our renderer is correct, especially with regards to snapping

struct uiDrawTextLayout {
	IDWriteTextFormat *format;
	IDWriteTextLayout *layout;
	std::vector<backgroundFunc> *backgroundFuncs;
	UINT32 nLines;
	struct lineInfo *lineInfo;
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

struct lineInfo {
	size_t startPos;			// in UTF-16 points
	size_t endPos;
	size_t newlineCount;
	double x;
	double y;
	double width;
	double height;
	double baseline;
};

// this function is deeply indebted to the PadWrite sample: https://github.com/Microsoft/Windows-classic-samples/blob/master/Samples/Win7Samples/multimedia/DirectWrite/PadWrite/TextEditor.cpp
static void computeLineInfo(uiDrawTextLayout *tl)
{
	DWRITE_LINE_METRICS *dlm;
	size_t nextStart;
	UINT32 i, j;
	DWRITE_HIT_TEST_METRICS *htm;
	UINT32 nFragments, unused;
	HRESULT hr;

	// TODO make sure this is legal; if not, switch to GetMetrics() and use its line count field instead
	hr = tl->layout->GetLineMetrics(NULL, 0, &(tl->nLines));
	// ugh, HRESULT_TO_WIN32() is an inline function and is not constexpr so we can't use switch here
	if (hr == S_OK) {
		// TODO what do we do here
	} else if (hr != E_NOT_SUFFICIENT_BUFFER)
		logHRESULT(L"error getting number of lines in IDWriteTextLayout", hr);
	tl->lineInfo = (struct lineInfo *) uiAlloc(tl->nLines * sizeof (struct lineInfo), "struct lineInfo[] (text layout)");

	dlm = new DWRITE_LINE_METRICS[tl->nLines];
	// we can't pass NULL here; it outright crashes if we do
	// TODO verify the numbers haven't changed
	hr = tl->layout->GetLineMetrics(dlm, tl->nLines, &unused);
	if (hr != S_OK)
		logHRESULT(L"error getting IDWriteTextLayout line metrics", hr);

	// assume the first line starts at position 0 and the string flow is incremental
	nextStart = 0;
	for (i = 0; i < tl->nLines; i++) {
		tl->lineInfo[i].startPos = nextStart;
		tl->lineInfo[i].endPos = nextStart + dlm[i].length;
		tl->lineInfo[i].newlineCount = dlm[i].newlineLength;
		nextStart = tl->lineInfo[i].endPos;

		// a line can have multiple fragments; for example, if there's a bidirectional override in the middle of a line
		hr = tl->layout->HitTestTextRange(tl->lineInfo[i].startPos, (tl->lineInfo[i].endPos - tl->lineInfo[i].newlineCount) - tl->lineInfo[i].startPos,
			0, 0,
			NULL, 0, &nFragments);
		if (hr != S_OK && hr != E_NOT_SUFFICIENT_BUFFER)
			logHRESULT(L"error getting IDWriteTextLayout line fragment count", hr);
		htm = new DWRITE_HIT_TEST_METRICS[nFragments];
		// TODO verify unused == nFragments?
		hr = tl->layout->HitTestTextRange(tl->lineInfo[i].startPos, (tl->lineInfo[i].endPos - tl->lineInfo[i].newlineCount) - tl->lineInfo[i].startPos,
			0, 0,
			htm, nFragments, &unused);
		// TODO can this return E_NOT_SUFFICIENT_BUFFER again?
		if (hr != S_OK)
			logHRESULT(L"error getting IDWriteTextLayout line fragment metrics", hr);
		// TODO verify htm.textPosition and htm.length against dtm[i]/tl->lineInfo[i]?
		tl->lineInfo[i].x = htm[0].left;
		tl->lineInfo[i].y = htm[0].top;
		// TODO does this not include trailing whitespace? I forget
		tl->lineInfo[i].width = htm[0].width;
		tl->lineInfo[i].height = htm[0].height;
		for (j = 1; j < nFragments; j++) {
			// this is correct even if the leftmost fragment on the line is RTL
			if (tl->lineInfo[i].x > htm[j].left)
				tl->lineInfo[i].x = htm[j].left;
			tl->lineInfo[i].width += htm[j].width;
			// TODO verify y and height haven't changed?
		}
		// TODO verify dlm[i].height == htm.height?
		delete[] htm;

		// TODO on Windows 8.1 and/or 10 we can use DWRITE_LINE_METRICS1 to get specific info about the ascent and descent; do we have an alternative?
		// TODO and even on those platforms can we somehow split tyographic leading from spacing?
		// TODO and on that note, can we have both line spacing proportionally above and uniformly below?
		tl->lineInfo[i].baseline = dlm[i].baseline;
	}

	delete[] dlm;
}

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

	tl = uiNew(uiDrawTextLayout);

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
	if (hr != S_OK)
		logHRESULT(L"error creating IDWriteTextFormat", hr);
	hr = tl->format->SetTextAlignment(dwriteAligns[p->Align]);
	if (hr != S_OK)
		logHRESULT(L"error applying text layout alignment", hr);

	hr = dwfactory->CreateTextLayout(
		(const WCHAR *) attrstrUTF16(p->String), attrstrUTF16Len(p->String),
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

	attrstrToIDWriteTextLayoutAttrs(p, tl->layout, &(tl->backgroundFuncs));

	computeLineInfo(tl);

	// and finally copy the UTF-8/UTF-16 index conversion tables
	tl->u8tou16 = attrstrCopyUTF8ToUTF16(p->String, &(tl->nUTF8));
	tl->u16tou8 = attrstrCopyUTF16ToUTF8(p->String, &(tl->nUTF16));

	// TODO can/should this be moved elsewhere?
	uiFree(wDefaultFamily);
	return tl;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *tl)
{
	uiFree(tl->u16tou8);
	uiFree(tl->u8tou16);
	uiFree(tl->lineInfo);
	delete tl->backgroundFuncs;
	tl->layout->Release();
	tl->format->Release();
	uiFree(tl);
}

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
		textDrawingEffect *t = (textDrawingEffect *) clientDrawingEffect;
		ID2D1SolidColorBrush *brush;

		baseline.x = baselineOriginX;
		baseline.y = baselineOriginY;
		brush = this->black;
		if (t != NULL && t->hasColor) {
			HRESULT hr;

			hr = mkSolidBrush(this->rt, t->r, t->g, t->b, t->a, &brush);
			if (hr != S_OK)
				return hr;
		}
		this->rt->DrawGlyphRun(
			baseline,
			glyphRun,
			brush,
			measuringMode);
		if (t != NULL && t->hasColor)
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

	virtual HRESULT STDMETHODCALLTYPE DrawUnderline(void *clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, const DWRITE_UNDERLINE *underline, IUnknown *clientDrawingEffect)
	{
		textDrawingEffect *t = (textDrawingEffect *) clientDrawingEffect;
		ID2D1SolidColorBrush *brush;
		D2D1_RECT_F rect;
		D2D1::Matrix3x2F pixeltf;
		FLOAT dpix, dpiy;
		D2D1_POINT_2F pt;

		if (underline == NULL)
			return E_POINTER;
		if (t == NULL)		// we can only get here through an underline
			return E_UNEXPECTED;
		brush = this->black;
		if (t->hasUnderlineColor) {
			HRESULT hr;

			hr = mkSolidBrush(this->rt, t->ur, t->ug, t->ub, t->ua, &brush);
			if (hr != S_OK)
				return hr;
		} else if (t->hasColor) {
			// TODO formalize this rule
			HRESULT hr;

			hr = mkSolidBrush(this->rt, t->r, t->g, t->b, t->a, &brush);
			if (hr != S_OK)
				return hr;
		}
		rect.left = baselineOriginX;
		rect.top = baselineOriginY + underline->offset;
		rect.right = rect.left + underline->width;
		rect.bottom = rect.top + underline->thickness;
		switch (t->u) {
		case uiDrawUnderlineStyleSingle:
			this->rt->FillRectangle(&rect, brush);
			break;
		case uiDrawUnderlineStyleDouble:
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
		case uiDrawUnderlineStyleSuggestion:
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
		if (t->hasUnderlineColor || t->hasColor)
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

	for (const auto &f : *(tl->backgroundFuncs))
		f(c, tl, x, y);

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

int uiDrawTextLayoutNumLines(uiDrawTextLayout *tl)
{
	return tl->nLines;
}

// DirectWrite doesn't provide a direct way to do this, so we have to do this manually
// TODO does that comment still apply here or to the code at the top of this file?
void uiDrawTextLayoutLineByteRange(uiDrawTextLayout *tl, int line, size_t *start, size_t *end)
{
	*start = tl->lineInfo[line].startPos;
	*start = tl->u16tou8[*start];
	*end = tl->lineInfo[line].endPos - tl->lineInfo[line].newlineCount;
	*end = tl->u16tou8[*end];
}

void uiDrawTextLayoutLineGetMetrics(uiDrawTextLayout *tl, int line, uiDrawTextLayoutLineMetrics *m)
{
	m->X = tl->lineInfo[line].x;
	m->Y = tl->lineInfo[line].y;
	m->Width = tl->lineInfo[line].width;
	m->Height = tl->lineInfo[line].height;

	// TODO rename tl->lineInfo[line].baseline to .baselineOffset or something of the sort to make its meaning more clear
	m->BaselineY = tl->lineInfo[line].y + tl->lineInfo[line].baseline;
	m->Ascent = tl->lineInfo[line].baseline;
	m->Descent = tl->lineInfo[line].height - tl->lineInfo[line].baseline;
	m->Leading = 0;		// TODO

	m->ParagraphSpacingBefore = 0;		// TODO
	m->LineHeightSpace = 0;				// TODO
	m->LineSpacing = 0;				// TODO
	m->ParagraphSpacing = 0;			// TODO
}

// this algorithm comes from Microsoft's PadWrite sample, following TextEditor::SetSelectionFromPoint()
// TODO go back through all of these and make sure we convert coordinates properly
// TODO same for OS X
void uiDrawTextLayoutHitTest(uiDrawTextLayout *tl, double x, double y, size_t *pos, int *line)
{
	DWRITE_HIT_TEST_METRICS m;
	BOOL trailing, inside;
	size_t p;
	UINT32 i;
	HRESULT hr;

	hr = tl->layout->HitTestPoint(x, y,
		&trailing, &inside,
		&m);
	if (hr != S_OK)
		logHRESULT(L"error hit-testing IDWriteTextLayout", hr);
	p = m.textPosition;
	// on a trailing hit, align to the nearest cluster
	if (trailing) {
		DWRITE_HIT_TEST_METRICS m2;
		FLOAT x, y;				// crashes if I skip these :/

		hr = tl->layout->HitTestTextPosition(m.textPosition, trailing,
			&x, &y, &m2);
		if (hr != S_OK)
			logHRESULT(L"error aligning trailing hit to nearest cluster", hr);
		p = m2.textPosition + m2.length;
	}
	*pos = tl->u16tou8[p];

	for (i = 0; i < tl->nLines; i++) {
		double ltop, lbottom;

		ltop = tl->lineInfo[i].y;
		lbottom = ltop + tl->lineInfo[i].height;
		// y will already >= ltop at this point since the past lbottom should == ltop
		if (y < lbottom)
			break;
	}
	if (i == tl->nLines)
		i--;
	*line = i;
}

double uiDrawTextLayoutByteLocationInLine(uiDrawTextLayout *tl, size_t pos, int line)
{
	BOOL trailing;
	DWRITE_HIT_TEST_METRICS m;
	FLOAT x, y;
	HRESULT hr;

	if (line < 0 || line >= tl->nLines)
		return -1;
	pos = tl->u8tou16[pos];
	// note: >, not >=, because the position at endPos is valid!
	if (pos < tl->lineInfo[line].startPos || pos > tl->lineInfo[line].endPos)
		return -1;
	// this behavior seems correct
	// there's also PadWrite's TextEditor::GetCaretRect() but that requires state...
	// TODO where does this fail?
	trailing = FALSE;
	if (pos != 0 && pos != tl->nUTF16 && pos == tl->lineInfo[line].endPos) {
		pos--;
		trailing = TRUE;
	}
	hr = tl->layout->HitTestTextPosition(pos, trailing,
		&x, &y, &m);
	if (hr != S_OK)
		logHRESULT(L"error calling IDWriteTextLayout::HitTestTextPosition()", hr);
	return x;
}

void caretDrawParams(uiDrawContext *c, double height, struct caretDrawParams *p)
{
	DWORD caretWidth;

	// there seems to be no defined caret color
	// the best I can come up with is "inverts colors underneath" (according to https://msdn.microsoft.com/en-us/library/windows/desktop/ms648397(v=vs.85).aspx) which I have no idea how to do (TODO)
	// just return black for now
	p->r = 0.0;
	p->g = 0.0;
	p->b = 0.0;
	p->a = 1.0;

	if (SystemParametersInfoW(SPI_GETCARETWIDTH, 0, &caretWidth, 0) == 0)
		// don't log the failure, fall back gracefully
		// the instruction to use this comes from https://msdn.microsoft.com/en-us/library/windows/desktop/ms648399(v=vs.85).aspx
		// and we have to assume GetSystemMetrics() always succeeds, so
		caretWidth = GetSystemMetrics(SM_CXBORDER);
	// TODO make this a function and split it out of areautil.cpp
	{
		FLOAT dpix, dpiy;

		// TODO can we pass NULL for dpiy?
		c->rt->GetDpi(&dpix, &dpiy);
		// see https://msdn.microsoft.com/en-us/library/windows/desktop/dd756649%28v=vs.85%29.aspx (and others; search "direct2d mouse")
		p->width = ((double) (caretWidth * 96)) / dpix;
	}
	// and there doesn't seem to be this either... (TODO check what PadWrite does?)
	p->xoff = 0;
}
