// 22 december 2015
#include "uipriv_windows.h"

// notes:
// only available in windows 8 and newer:
// - character spacing
// - kerning control
// - justficiation (how could I possibly be making this up?!)
// - vertical text (SERIOUSLY?! WHAT THE ACTUAL FUCK, MICROSOFT?!?!?!? DID YOU NOT THINK ABOUT THIS THE FIRST TIME, TRYING TO IMPROVE THE INTERNATIONALIZATION OF WINDOWS 7?!?!?! bonus: some parts of MSDN even say 8.1 and up only!)

struct uiDrawFontFamilies {
	fontCollection *fc;
};

uiDrawFontFamilies *uiDrawListFontFamilies(void)
{
	struct uiDrawFontFamilies *ff;

	ff = uiNew(struct uiDrawFontFamilies);
	ff->fc = loadFontCollection();
	return ff;
}

uintmax_t uiDrawFontFamiliesNumFamilies(uiDrawFontFamilies *ff)
{
	return ff->fc->fonts->GetFontFamilyCount();
}

char *uiDrawFontFamiliesFamily(uiDrawFontFamilies *ff, uintmax_t n)
{
	IDWriteFontFamily *family;
	WCHAR *wname;
	char *name;
	HRESULT hr;

	hr = ff->fc->fonts->GetFontFamily(n, &family);
	if (hr != S_OK)
		logHRESULT("error getting font out of collection in uiDrawFontFamiliesFamily()", hr);
	wname = fontCollectionFamilyName(ff->fc, family);
	name = toUTF8(wname);
	uiFree(wname);
	family->Release();
	return name;
}

void uiDrawFreeFontFamilies(uiDrawFontFamilies *ff)
{
	fontCollectionFree(ff->fc);
	uiFree(ff);
}

struct uiDrawTextFont {
	IDWriteFont *f;
	WCHAR *family;		// save for convenience in uiDrawNewTextLayout()
	double size;
};

// We could use a C99-style array initializer like the other backends, but C++ doesn't support those.
// But it turns out we need to look up both by uival and dwval, so this loop method is fine...

static const struct {
	bool lastOne;
	uiDrawTextWeight uival;
	DWRITE_FONT_WEIGHT dwval;
} dwriteWeights[] = {
	{ false, uiDrawTextWeightThin, DWRITE_FONT_WEIGHT_THIN },
	{ false, uiDrawTextWeightUltraLight, DWRITE_FONT_WEIGHT_ULTRA_LIGHT },
	{ false, uiDrawTextWeightLight, DWRITE_FONT_WEIGHT_LIGHT },
	{ false, uiDrawTextWeightBook, DWRITE_FONT_WEIGHT_SEMI_LIGHT },
	{ false, uiDrawTextWeightNormal, DWRITE_FONT_WEIGHT_NORMAL },
	{ false, uiDrawTextWeightMedium, DWRITE_FONT_WEIGHT_MEDIUM },
	{ false, uiDrawTextWeightSemiBold, DWRITE_FONT_WEIGHT_SEMI_BOLD },
	{ false, uiDrawTextWeightBold, DWRITE_FONT_WEIGHT_BOLD },
	{ false, uiDrawTextWeightUtraBold, DWRITE_FONT_WEIGHT_ULTRA_BOLD },
	{ false, uiDrawTextWeightHeavy, DWRITE_FONT_WEIGHT_HEAVY },
	{ true, uiDrawTextWeightUltraHeavy, DWRITE_FONT_WEIGHT_ULTRA_BLACK, },
};

static const struct {
	bool lastOne;
	uiDrawTextItalic uival;
	DWRITE_FONT_STYLE dwval;
} dwriteItalics[] = {
	{ false, uiDrawTextItalicNormal, DWRITE_FONT_STYLE_NORMAL },
	{ false, uiDrawTextItalicOblique, DWRITE_FONT_STYLE_OBLIQUE },
	{ true, uiDrawTextItalicItalic, DWRITE_FONT_STYLE_ITALIC },
};

static const struct {
	bool lastOne;
	uiDrawTextStretch uival;
	DWRITE_FONT_STRETCH dwval;
} dwriteStretches[] = {
	{ false, uiDrawTextStretchUltraCondensed, DWRITE_FONT_STRETCH_ULTRA_CONDENSED },
	{ false, uiDrawTextStretchExtraCondensed, DWRITE_FONT_STRETCH_EXTRA_CONDENSED },
	{ false, uiDrawTextStretchCondensed, DWRITE_FONT_STRETCH_CONDENSED },
	{ false, uiDrawTextStretchSemiCondensed, DWRITE_FONT_STRETCH_SEMI_CONDENSED },
	{ false, uiDrawTextStretchNormal, DWRITE_FONT_STRETCH_NORMAL },
	{ false, uiDrawTextStretchSemiExpanded, DWRITE_FONT_STRETCH_SEMI_EXPANDED },
	{ false, uiDrawTextStretchExpanded, DWRITE_FONT_STRETCH_EXPANDED },
	{ false, uiDrawTextStretchExtraExpanded, DWRITE_FONT_STRETCH_EXTRA_EXPANDED },
	{ true, uiDrawTextStretchUltraExpanded, DWRITE_FONT_STRETCH_ULTRA_EXPANDED },
};

void attrToDWriteAttr(struct dwriteAttr *attr)
{
	bool found;
	int i;

	found = false;
	for (i = 0; ; i++) {
		if (dwriteWeights[i].uival == attr->weight) {
			attr->dweight = dwriteWeights[i].dwval;
			found = true;
			break;
		}
		if (dwriteWeights[i].lastOne)
			break;
	}
	if (!found)
		complain("invalid weight %d passed to attrToDWriteAttr()", attr->weight);

	found = false;
	for (i = 0; ; i++) {
		if (dwriteItalics[i].uival == attr->italic) {
			attr->ditalic = dwriteItalics[i].dwval;
			found = true;
			break;
		}
		if (dwriteItalics[i].lastOne)
			break;
	}
	if (!found)
		complain("invalid italic %d passed to attrToDWriteAttr()", attr->italic);

	found = false;
	for (i = 0; ; i++) {
		if (dwriteStretches[i].uival == attr->stretch) {
			attr->dstretch = dwriteStretches[i].dwval;
			found = true;
			break;
		}
		if (dwriteStretches[i].lastOne)
			break;
	}
	if (!found)
		complain("invalid stretch %d passed to attrToDWriteAttr()", attr->stretch);
}

void dwriteAttrToAttr(struct dwriteAttr *attr)
{
	int weight, against, n;
	int curdiff, curindex;
	bool found;
	int i;

	// weight is scaled; we need to test to see what's nearest
	weight = (int) (attr->dweight);
	against = (int) (dwriteWeights[0].dwval);
	curdiff = abs(against - weight);
	curindex = 0;
	for (i = 1; ; i++) {
		against = (int) (dwriteWeights[i].dwval);
		n = abs(against - weight);
		if (n < curdiff) {
			curdiff = n;
			curindex = i;
		}
		if (dwriteWeights[i].lastOne)
			break;
	}
	attr->weight = dwriteWeights[i].uival;

	// italic and stretch are simple values; we can just do a matching search
	found = false;
	for (i = 0; ; i++) {
		if (dwriteItalics[i].dwval == attr->ditalic) {
			attr->italic = dwriteItalics[i].uival;
			found = true;
			break;
		}
		if (dwriteItalics[i].lastOne)
			break;
	}
	if (!found)
		complain("invalid italic %d passed to dwriteAttrToAttr()", attr->ditalic);

	found = false;
	for (i = 0; ; i++) {
		if (dwriteStretches[i].dwval == attr->dstretch) {
			attr->stretch = dwriteStretches[i].uival;
			found = true;
			break;
		}
		if (dwriteStretches[i].lastOne)
			break;
	}
	if (!found)
		complain("invalid stretch %d passed to dwriteAttrToAttr()", attr->dstretch);
}

uiDrawTextFont *uiDrawLoadClosestFont(const uiDrawTextFontDescriptor *desc)
{
	uiDrawTextFont *font;
	IDWriteFontCollection *collection;
	UINT32 index;
	BOOL exists;
	struct dwriteAttr attr;
	IDWriteFontFamily *family;
	HRESULT hr;

	font = uiNew(uiDrawTextFont);

	// always get the latest available font information
	hr = dwfactory->GetSystemFontCollection(&collection, TRUE);
	if (hr != S_OK)
		logHRESULT("error getting system font collection in uiDrawLoadClosestFont()", hr);

	font->family = toUTF16(desc->Family);
	hr = collection->FindFamilyName(font->family, &index, &exists);
	if (hr != S_OK)
		logHRESULT("error finding font family in uiDrawLoadClosestFont()", hr);
	if (!exists)
		complain("TODO family not found in uiDrawLoadClosestFont()", hr);
	hr = collection->GetFontFamily(index, &family);
	if (hr != S_OK)
		logHRESULT("error loading font family in uiDrawLoadClosestFont()", hr);

	attr.weight = desc->Weight;
	attr.italic = desc->Italic;
	attr.stretch = desc->Stretch;
	attrToDWriteAttr(&attr);

	hr = family->GetFirstMatchingFont(
		attr.dweight,
		attr.dstretch,
		attr.ditalic,
		&(font->f));
	if (hr != S_OK)
		logHRESULT("error loading font in uiDrawLoadClosestFont()", hr);

	font->size = desc->Size;

	family->Release();
	collection->Release();

	return font;
}

void uiDrawFreeTextFont(uiDrawTextFont *font)
{
	font->f->Release();
	uiFree(font->family);
	uiFree(font);
}

uintptr_t uiDrawTextFontHandle(uiDrawTextFont *font)
{
	return (uintptr_t) (font->f);
}

void uiDrawTextFontDescribe(uiDrawTextFont *font, uiDrawTextFontDescriptor *desc)
{
	// TODO

	desc->Size = font->size;

	// TODO
}

// text sizes are 1/72 of an inch
// points in Direct2D are 1/96 of an inch (https://msdn.microsoft.com/en-us/library/windows/desktop/ff684173%28v=vs.85%29.aspx, https://msdn.microsoft.com/en-us/library/windows/desktop/hh447022%28v=vs.85%29.aspx)
// As for the actual conversion from design units, see:
// - http://cboard.cprogramming.com/windows-programming/136733-directwrite-font-height-issues.html
// - https://sourceforge.net/p/vstgui/mailman/message/32483143/
// - http://xboxforums.create.msdn.com/forums/t/109445.aspx
// - https://msdn.microsoft.com/en-us/library/dd183564%28v=vs.85%29.aspx
// - http://www.fontbureau.com/blog/the-em/
// TODO make points here about how DIPs in DirectWrite == DIPs in Direct2D; if not, figure out what they really are? for the width and layout functions later
static double scaleUnits(double what, double designUnitsPerEm, double size)
{
	return (what / designUnitsPerEm) * (size * (96.0 / 72.0));
}

void uiDrawTextFontGetMetrics(uiDrawTextFont *font, uiDrawTextFontMetrics *metrics)
{
	DWRITE_FONT_METRICS dm;

	font->f->GetMetrics(&dm);
	metrics->Ascent = scaleUnits(dm.ascent, dm.designUnitsPerEm, font->size);
	metrics->Descent = scaleUnits(dm.descent, dm.designUnitsPerEm, font->size);
	// TODO what happens if dm.xxx is negative?
	metrics->Leading = scaleUnits(dm.lineGap, dm.designUnitsPerEm, font->size);
	metrics->UnderlinePos = scaleUnits(dm.underlinePosition, dm.designUnitsPerEm, font->size);
	metrics->UnderlineThickness = scaleUnits(dm.underlineThickness, dm.designUnitsPerEm, font->size);
}

struct uiDrawTextLayout {
	IDWriteTextFormat *format;
	IDWriteTextLayout *layout;
	intmax_t *bytesToCharacters;
};

#define MBTWC(str, n, wstr, bufsiz) MultiByteToWideChar(CP_UTF8, 0, str, n, wstr, bufsiz)
#define MBTWCErr(str, n, wstr, bufsiz) MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str, n, wstr, bufsiz)

// TODO figure out how ranges are specified in DirectWrite
// TODO clean up the local variable names and improve documentation
static intmax_t *toUTF16Offsets(const char *str, WCHAR **wstr, intmax_t *wlenout)
{
	intmax_t *bytesToCharacters;
	intmax_t i, len;
	int wlen;
	intmax_t outpos;

	len = strlen(str);
	bytesToCharacters = (intmax_t *) uiAlloc(len * sizeof (intmax_t), "intmax_t[]");

	wlen = MBTWC(str, -1, NULL, 0);
	if (wlen == 0)
		logLastError("error figuring out number of characters to convert to in toUTF16Offsets()");
	*wstr = (WCHAR *) uiAlloc(wlen * sizeof (WCHAR), "WCHAR[]");
	*wlenout = wlen;

	i = 0;
	outpos = 0;
	while (i < len) {
		intmax_t n;
		intmax_t j;
		BOOL found;
		int m;

		// figure out how many characters to convert and convert them
		found = FALSE;
		for (n = 1; (i + n - 1) < len; n++) {
			// we need MB_ERR_INVALID_CHARS here for this to work properly
			m = MBTWCErr(str + i, n, *wstr + outpos, wlen - outpos);
			if (m != 0) {			// found a full character
				found = TRUE;
				break;
			}
		}
		// if this test passes we reached the end of the string without a successful conversion (invalid string)
		if (!found)
			logLastError("something bad happened when trying to prepare string in uiDrawNewTextLayout()");

		// now save the character offsets for those bytes
		for (j = 0; j < m; j++)
			bytesToCharacters[j] = outpos;

		// and go to the next
		i += n;
		outpos += m;
	}

	return bytesToCharacters;
}

uiDrawTextLayout *uiDrawNewTextLayout(const char *text, uiDrawTextFont *defaultFont, double width)
{
	uiDrawTextLayout *layout;
	WCHAR *wtext;
	intmax_t wlen;
	HRESULT hr;

	layout = uiNew(uiDrawTextLayout);

	hr = dwfactory->CreateTextFormat(defaultFont->family,
		NULL,
		defaultFont->f->GetWeight(),
		defaultFont->f->GetStyle(),
		defaultFont->f->GetStretch(),
		// typographic points are 1/72 inch; this parameter is 1/96 inch
		// fortunately Microsoft does this too, in https://msdn.microsoft.com/en-us/library/windows/desktop/dd371554%28v=vs.85%29.aspx
		defaultFont->size * (96.0 / 72.0),
		// see http://stackoverflow.com/questions/28397971/idwritefactorycreatetextformat-failing and https://msdn.microsoft.com/en-us/library/windows/desktop/dd368203.aspx
		// TODO use the current locale again?
		L"",
		&(layout->format));
	if (hr != S_OK)
		logHRESULT("error creating IDWriteTextFormat in uiDrawNewTextLayout()", hr);

	layout->bytesToCharacters = toUTF16Offsets(text, &wtext, &wlen);
	hr = dwfactory->CreateTextLayout(wtext, wlen,
		layout->format,
		// FLOAT is float, not double, so this should work... TODO
		FLT_MAX, FLT_MAX,
		&(layout->layout));
	if (hr != S_OK)
		logHRESULT("error creating IDWriteTextLayout in uiDrawNewTextLayout()", hr);
	uiFree(wtext);

	uiDrawTextLayoutSetWidth(layout, width);

	return layout;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *layout)
{
	layout->layout->Release();
	layout->format->Release();
	uiFree(layout);
}

void uiDrawTextLayoutSetWidth(uiDrawTextLayout *layout, double width)
{
	DWRITE_WORD_WRAPPING wrap;
	FLOAT maxWidth;
	HRESULT hr;

	// this is the only wrapping mode (apart from "no wrap") available prior to Windows 8.1
	wrap = DWRITE_WORD_WRAPPING_WRAP;
	maxWidth = width;
	if (width < 0) {
		wrap = DWRITE_WORD_WRAPPING_NO_WRAP;
		// setting the max width in this case technically isn't needed since the wrap mode will simply ignore the max width, but let's do it just to be safe
		maxWidth = FLT_MAX;		// see TODO above
	}
	hr = layout->layout->SetWordWrapping(wrap);
	if (hr != S_OK)
		logHRESULT("error setting word wrapping mode in uiDrawTextLayoutSetWidth()", hr);
	hr = layout->layout->SetMaxWidth(maxWidth);
	if (hr != S_OK)
		logHRESULT("error setting max layout width in uiDrawTextLayoutSetWidth()", hr);
}

// TODO for a single line the height includes the leading; it should not
void uiDrawTextLayoutExtents(uiDrawTextLayout *layout, double *width, double *height)
{
	DWRITE_TEXT_METRICS metrics;
	HRESULT hr;

	hr = layout->layout->GetMetrics(&metrics);
	if (hr != S_OK)
		logHRESULT("error getting layout metrics in uiDrawTextLayoutExtents()", hr);
	*width = metrics.width;
	// TODO make sure the behavior of this on empty strings is the same on all platforms
	*height = metrics.height;
}

void doDrawText(ID2D1RenderTarget *rt, ID2D1Brush *black, double x, double y, uiDrawTextLayout *layout)
{
	D2D1_POINT_2F pt;
	HRESULT hr;

	pt.x = x;
	pt.y = y;
	// TODO D2D1_DRAW_TEXT_OPTIONS_NO_SNAP?
	// TODO D2D1_DRAW_TEXT_OPTIONS_CLIP?
	// TODO when setting 8.1 as minimum, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT?
	rt->DrawTextLayout(pt, layout->layout, black, D2D1_DRAW_TEXT_OPTIONS_NONE);
}
