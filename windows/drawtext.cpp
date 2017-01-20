// 17 january 2017
#include "uipriv_windows.hpp"

struct uiDrawTextLayout {
	IDWriteTextFormat *format;
	IDWriteTextLayout *layout;
	UINT32 *nLines;
	struct lineInfo *lineInfo;
	// for converting DirectWrite indices to byte offsets
	size_t *u16tou8;
	size_t nu16tou8;		// TODO I don't like the casing of this name; is it even necessary?
};

// typographic points are 1/72 inch; this parameter is 1/96 inch
// fortunately Microsoft does this too, in https://msdn.microsoft.com/en-us/library/windows/desktop/dd371554%28v=vs.85%29.aspx
#define pointSizeToDWriteSize(size) (size * (96.0 / 72.0))

static const DWRITE_FONT_STYLE dwriteItalics[] = {
	[uiDrawTextItalicNormal] = DWRITE_FONT_STYLE_NORMAL,
	[uiDrawTextItalicOblique] = DWRITE_FONT_STYLE_OBLIQUE,
	[uiDrawTextItalicItalic] = DWRITE_FONT_STYLE_ITALIC,
};

static const DWRITE_FONT_STRETCH dwriteStretches[] = {
	[uiDrawTextStretchUltraCondensed] = DWRITE_FONT_STRETCH_ULTRA_CONDENSED,
	[uiDrawTextStretchExtraCondensed] = DWRITE_FONT_STRETCH_EXTRA_CONDENSED,
	[uiDrawTextStretchCondensed] = DWRITE_FONT_STRETCH_CONDENSED,
	[uiDrawTextStretchSemiCondensed] = DWRITE_FONT_STRETCH_SEMI_CONDENSED,
	[uiDrawTextStretchNormal] = DWRITE_FONT_STRETCH_NORMAL,
	[uiDrawTextStretchSemiExpanded] = DWRITE_FONT_STRETCH_SEMI_EXPANDED,
	[uiDrawTextStretchExpanded] = DWRITE_FONT_STRETCH_EXPANDED,
	[uiDrawTextStretchExtraExpanded] = DWRITE_FONT_STRETCH_EXTRA_EXPANDED,
	[uiDrawTextStretchUltraExpanded] = DWRITE_FONT_STRETCH_ULTRA_EXPANDED,
};

struct lineInfo {
	size_t startPos;
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
	UINT32 i;
	DWRITE_HIT_TEST_METRICS htm;
	UINT32 unused;
	HRESULT hr;

	// TODO make sure this is legal; if not, switch to GetMetrics() and use its line count field instead
	hr = tl->layout->GetLineMetrics(NULL, 0, &(tl->nLines));
	switch (hr) {
	case S_OK:
		// TODO what do we do here
	case E_NOT_SUFFICIENT_BUFFER:
		break;
	default:
		logHRESULT(L"error getting number of lines in IDWriteTextLayout", hr);
	}
	tl->lineInfo = (struct lineInfo *) uiAlloc(tl->nLines * sizeof (struct lineInfo), "struct lineInfo[] (text layout)");

	dlm = new DWRITE_LINE_METRICS[tl->nLines];
	// TODO make sure pasisng NULL here is legal
	hr = tl->layout->GetLineMetrics(dlm, tl->nLines, NULL);
	if (hr != S_OK)
		logHRESULT(L"error getting IDWriteTextLayout line metrics", hr);

	// assume the first line starts at position 0 and the string flow is incremental
	nextStart = 0;
	for (i = 0; i < tl->nLines; i++) {
		tl->lineinfo[i].startPos = nextStart;
		tl->lineinfo[i].endPos = nextStart + dlm[i].length;
		tl->lineinfo[i].newlineCount = dlm[i].newlineLength;
		nextStart = tl->lineinfo[i].endpos;

		hr = layout->HitTestTextRange(line->startPos, (line->endPos - line->newlineCount) - line->startPos,
			0, 0,
			&htm, 1, &unused);
		if (hr == E_NOT_SUFFICIENT_BUFFER)
			logHRESULT(L"TODO CONTACT ANDLABS — IDWriteTextLayout::HitTestTextRange() can return multiple ranges for a single line", hr);
		if (hr != S_OK)
			logHRESULT(L"error getting IDWriteTextLayout line rect", hr);
		// TODO verify htm.textPosition and htm.length?
		tl->lineInfo[i].x = htm.left;
		tl->lineInfo[i].y = htm.top;
		tl->lineInfo[i].width = htm.width;
		tl->lineInfo[i].height = htm.height;
		// TODO verify dlm[i].height == htm.height

		// TODO on Windows 8.1 and/or 10 we can use DWRITE_LINE_METRICS1 to get specific info about the ascent and descent; do we have an alternative?
		// TODO and even on those platforms can we somehow split tyographic leading from spacing?
		// TODO and on that note, can we have both line spacing proportionally above and uniformly below?
		tl->lineinfo[i].baseline = dlm[i].baseline;
	}

	delete[] dlm;
}

uiDrawTextLayout *uiDrawNewTextLayout(uiAttributedString *s, uiDrawFontDescriptor *defaultFont, double width)
{
	uiDrawTextLayout *tl;
	WCHAR *wDefaultFamily;
	DWRITE_WORD_WRAPPING wrap;
	FLOAT maxWidth;
	HRESULT hr;

	tl = uiNew(uiDrawTextLayout);

	wDefaultFamily = toUTF16(defaultFont->Family);
	hr = dwfactory->CreateTextFormat(
		wDefaultFamily, NULL,
		// for the most part, DirectWrite weights correlate to ours
		// the differences:
		// - Minimum — libui: 0, DirectWrite: 1
		// - Maximum — libui: 1000, DirectWrite: 999
		// TODO figure out what to do about this shorter range (the actual major values are the same (but with different names), so it's just a range issue)
		(DWRITE_FONT_WEIGHT) (defaultFont->Weight),
		dwriteItalics[defaultFont->Italic],
		dwriteStrecthes[defaultFont->Stretch],
		pointSizeToDWriteSize(defaultFont->Size),
		// see http://stackoverflow.com/questions/28397971/idwritefactorycreatetextformat-failing and https://msdn.microsoft.com/en-us/library/windows/desktop/dd368203.aspx
		// TODO use the current locale?
		L"",
		&(tl->format));
	if (hr != S_OK)
		logHRESULT(L"error creating IDWriteTextFormat", hr);

	hr = dwfactory->CreateTextLayout(
		attrstrUTF16(s), attrstrUTF16Len(s),
		tl->format,
		// FLOAT is float, not double, so this should work... TODO
		FLT_MAX, FLT_MAX,
		&(tl->layout));
	if (hr != S_OK)
		logHRESULT(L"error creating IDWriteTextLayout", hr);

	// and set the width
	// this is the only wrapping mode (apart from "no wrap") available prior to Windows 8.1 (TODO verify this fact) (TODO this should be the default anyway)
	wrap = DWRITE_WORD_WRAPPING_WRAP;
	maxWidth = (FLOAT) width;
	if (width < 0) {
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

	computeLineInfo(tl);

	// and finally copy the UTF-16 to UTF-8 index conversion table
	tl->u16tou8 = attrstrCopyUTF16ToUTF8(s, &(tl->nu16tou8));

	// TODO can/should this be moved elsewhere?
	uiFree(wDefaultFamily);
	return tl;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *tl)
{
	uiFree(tl->u16tou8);
	uiFree(tl->lineInfo);
	tl->layout->Release();
	tl->format->Release();
	uiFree(tl);
}

void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y)
{
	D2D1_POINT_2F pt;
	ID2D1Brush *black;

	// TODO document that fully opaque black is the default text color; figure out whether this is upheld in various scenarios on other platforms
	// TODO figure out if this needs to be cleaned out
	black = mkSolidBrush(c->rt, 0.0, 0.0, 0.0, 1.0);

	pt.x = x;
	pt.y = y;
	// TODO D2D1_DRAW_TEXT_OPTIONS_NO_SNAP?
	// TODO D2D1_DRAW_TEXT_OPTIONS_CLIP?
	// TODO when setting 8.1 as minimum (TODO verify), D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT?
	// TODO what is our pixel snapping setting related to the OPTIONS enum values?
	c->rt->DrawTextLayout(pt, tl->layout, black, D2D1_DRAW_TEXT_OPTIONS_NONE);

	black->Release();
}

// TODO for a single line the height includes the leading; should it? TextEdit on OS X always includes the leading and/or paragraph spacing, otherwise Klee won't work...
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
void uiDrawTextLayoutLineByteRange(uiDrawTextLayout *tl, int line, size_t *start, size_t *end)
{
	*start = tl->lineInfo[line].startPos;
	*start = tl->u16tou8[*start];
	*end = tl->lineInfo[line].endPos - tl->lineInfo[line].newlineCount;
	*end = tl->u16tou8[*end];
}

void uiDrawTextLayoutLineGetMetrics(uiDrawTextLayout *tl, int line, uiDrawTextLayoutLineMetrics *m)
{
	// TODO
}

void uiDrawTextLayoutByteIndexToGraphemeRect(uiDrawTextLayout *tl, size_t pos, int *line, double *x, double *y, double *width, double *height)
{
}

void uiDrawTextLayoutHitTest(uiDrawTextLayout *tl, double x, double y, uiDrawTextLayoutHitTestResult *result)
{
	// TODO
}

void uiDrawTextLayoutByteRangeToRectangle(uiDrawTextLayout *tl, size_t start, size_t end, uiDrawTextLayoutByteRangeRectangle *r)
{
}
