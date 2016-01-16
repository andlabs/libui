// 22 december 2015
// Before we begin, you may be wondering why this file is C++.
// Simple: <dwrite.h> is C++ only! Thanks Microsoft!
// And unlike UI Automation which accidentally just forgets the 'struct' and 'enum' tags in places, <dwrite.h> is a full C++ header file, with class definitions and the use of __uuidof. Oh well :/
#include "uipriv_windows.h"

// notes:
// only available in windows 8 and newer:
// - character spacing
// - kerning control
// - justficiation (how could I possibly be making this up?!)
// - vertical text (SERIOUSLY?! WHAT THE ACTUAL FUCK, MICROSOFT?!?!?!? DID YOU NOT THINK ABOUT THIS THE FIRST TIME, TRYING TO IMPROVE THE INTERNATIONALIZATION OF WINDOWS 7?!?!?! bonus: some parts of MSDN even say 8.1 and up only!)

static IDWriteFactory *dwfactory = NULL;

HRESULT initDrawText(void)
{
	// TOOD use DWRITE_FACTORY_TYPE_ISOLATED instead?
	return DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof (IDWriteFactory),
		(IUnknown **) (&dwfactory));
}

void uninitDrawText(void)
{
	dwfactory->Release();
}

struct uiDrawFontFamilies {
	IDWriteFontCollection *fonts;
	WCHAR userLocale[LOCALE_NAME_MAX_LENGTH];
	int userLocaleSuccess;
};

uiDrawFontFamilies *uiDrawListFontFamilies(void)
{
	uiDrawFontFamilies *ff;
	HRESULT hr;

	ff = uiNew(uiDrawFontFamilies);
	// always get the latest available font information
	hr = dwfactory->GetSystemFontCollection(&(ff->fonts), TRUE);
	if (hr != S_OK)
		logHRESULT("error getting system font collection in uiDrawListFontFamilies()", hr);
	ff->userLocaleSuccess = GetUserDefaultLocaleName(ff->userLocale, LOCALE_NAME_MAX_LENGTH);
	return ff;
}

uintmax_t uiDrawFontFamiliesNumFamilies(uiDrawFontFamilies *ff)
{
	return ff->fonts->GetFontFamilyCount();
}

char *uiDrawFontFamiliesFamily(uiDrawFontFamilies *ff, uintmax_t n)
{
	IDWriteFontFamily *family;
	IDWriteLocalizedStrings *names;
	UINT32 index;
	BOOL exists;
	UINT32 length;
	WCHAR *wname;
	char *name;
	HRESULT hr;

	hr = ff->fonts->GetFontFamily(n, &family);
	if (hr != S_OK)
		logHRESULT("error getting font out of collection in uiDrawFontFamiliesFamily()", hr);
	hr = family->GetFamilyNames(&names);
	if (hr != S_OK)
		logHRESULT("error getting names of font out in uiDrawFontFamiliesFamily()", hr);

	// this is complex, but we ignore failure conditions to allow fallbacks
	// 1) If the user locale name was successfully retrieved, try it
	// 2) If the user locale name was not successfully retrieved, or that locale's string does not exist, or an error occurred, try L"en-us", the US English locale
	// 3) And if that fails, assume the first one
	// This algorithm is straight from MSDN: https://msdn.microsoft.com/en-us/library/windows/desktop/dd368214%28v=vs.85%29.aspx
	// For step 2 to work, start by setting hr to S_OK and exists to FALSE.
	// TODO does it skip step 2 entirely if step 1 fails? rewrite it to be a more pure conversion of the MSDN code?
	hr = S_OK;
	exists = FALSE;
	if (ff->userLocaleSuccess != 0)
		hr = names->FindLocaleName(ff->userLocale, &index, &exists);
	if (hr != S_OK || (hr == S_OK && !exists))
		hr = names->FindLocaleName(L"en-us", &index, &exists);
	if (!exists)
		index = 0;

	hr = names->GetStringLength(index, &length);
	if (hr != S_OK)
		logHRESULT("error getting length of font name in uiDrawFontFamiliesFamily()", hr);
	// GetStringLength() does not include the null terminator, but GetString() does
	wname = (WCHAR *) uiAlloc((length + 1) * sizeof (WCHAR), "WCHAR[]");
	hr = names->GetString(index, wname, length + 1);
	if (hr != S_OK)
		logHRESULT("error getting font name in uiDrawFontFamiliesFamily()", hr);

	name = toUTF8(wname);

	uiFree(wname);
	names->Release();
	family->Release();
	return name;
}

void uiDrawFreeFontFamilies(uiDrawFontFamilies *ff)
{
	ff->fonts->Release();
	uiFree(ff);
}

struct uiDrawTextFont {
	IDWriteFont *f;
	WCHAR *family;		// save for convenience in uiDrawNewTextLayout()
	double size;
};

// Not only does C++11 NOT include C99 designated initializers, but the C++ standards committee has REPEATEDLY REJECTING THEM, covering their ears and yelling "CONSTRUCTORS!!!111 PRIVATE DATA!1111 ENCAPSULATION!11one" at the top of their lungs.
// So what could have been a simple array lookup is now a loop. Thanks guys.

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

uiDrawTextFont *uiDrawLoadClosestFont(const uiDrawTextFontDescriptor *desc)
{
	uiDrawTextFont *font;
	IDWriteFontCollection *collection;
	UINT32 index;
	BOOL exists;
	DWRITE_FONT_WEIGHT weight;
	DWRITE_FONT_STYLE italic;
	DWRITE_FONT_STRETCH stretch;
	bool found;
	int i;
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

	found = false;
	for (i = 0; ; i++) {
		if (dwriteWeights[i].uival == desc->Weight) {
			weight = dwriteWeights[i].dwval;
			found = true;
			break;
		}
		if (dwriteWeights[i].lastOne)
			break;
	}
	if (!found)
		complain("invalid initial weight %d passed to uiDrawLoadClosestFont()", desc->Weight);

	found = false;
	for (i = 0; ; i++) {
		if (dwriteItalics[i].uival == desc->Italic) {
			italic = dwriteItalics[i].dwval;
			found = true;
			break;
		}
		if (dwriteItalics[i].lastOne)
			break;
	}
	if (!found)
		complain("invalid initial italic %d passed to uiDrawLoadClosestFont()", desc->Italic);

	found = false;
	for (i = 0; ; i++) {
		if (dwriteStretches[i].uival == desc->Stretch) {
			stretch = dwriteStretches[i].dwval;
			found = true;
			break;
		}
		if (dwriteStretches[i].lastOne)
			break;
	}
	if (!found)
		complain("invalid initial stretch %d passed to uiDrawLoadClosestFont()", desc->Stretch);

	// TODO small caps and gravity

	hr = family->GetFirstMatchingFont(weight,
		stretch,
		italic,
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
	// TODO small caps
	// TODO gravity

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
