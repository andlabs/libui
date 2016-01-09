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
		logHRESULT("error getting list of system fonts in uiDrawListFontFamilies()", hr);
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

double uiDrawTextSizeToPoints(double textSize)
{
	// TODO
	return 0;
}

double uiDrawPointsToTextSize(double points)
{
	// TODO
	return 0;
}

struct uiDrawTextLayout {
	IDWriteTextFormat *format;
	IDWriteTextLayout *layout;
	intmax_t *bytesToCharacters;
};

#define MBTWC(str, n, wstr, bufsiz) MultiByteToWideChar(CP_UTF8, 0, str, n, wstr, bufsiz)

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

		// figure out how many characters to convert and convert them
		found = FALSE;
		for (n = 1; (i + n - 1) < len; n++)
			if (MBTWC(str + i, n, *wstr + outpos, wlen - outpos) == n) {
				// found a full character
				found = TRUE;
				break;
			}
		// if this test passes we reached the end of the string without a successful conversion (invalid string)
		if (!found)
			logLastError("something bad happened when trying to prepare string in uiDrawNewTextLayout()");

		// now save the character offsets for those bytes
		for (j = 0; j < n; j++)
			bytesToCharacters[j] = outpos;

		// and go to the next
		outpos += n;
	}

	return bytesToCharacters;
}


static const DWRITE_FONT_WEIGHT dwriteWeights[] = {
	[uiDrawTextWeightThin] = DWRITE_FONT_WEIGHT_THIN,
	[uiDrawTextWeightUltraLight] = DWRITE_FONT_WEIGHT_ULTRA_LIGHT,
	[uiDrawTextWeightLight] = DWRITE_FONT_WEIGHT_LIGHT,
	[uiDrawTextWeightBook] = DWRITE_FONT_WEIGHT_SEMI_LIGHT,
	[uiDrawTextWeightNormal] = DWRITE_FONT_WEIGHT_NORMAL,
	[uiDrawTextWeightMedium] = DWRITE_FONT_WEIGHT_MEDIUM,
	[uiDrawTextWeightSemiBold] = DWRITE_FONT_WEIGHT_SEMI_BOLD,
	[uiDrawTextWeightBold] = DWRITE_FONT_WEIGHT_BOLD,
	[uiDrawTextWeightUtraBold] = DWRITE_FONT_WEIGHT_ULTRA_BOLD,
	[uiDrawTextWeightHeavy] = DWRITE_FONT_WEIGHT_HEAVY,
	[uiDrawTextWeightUltraHeavy] = DWRITE_FONT_WEIGHT_ULTRA_BLACK,
};

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

uiDrawTextLayout *uiDrawNewTextLayout(const char *text, const uiDrawInitialTextStyle *initialStyle)
{
	uiDrawTextLayout *layout;
	WCHAR *family;
	WCHAR *wtext;
	intmax_t wlen;
	HRESULT hr;

	layout = uiNew(uiDrawTextLayout);

	family = toUTF16(initialStyle->Family);
	hr = dwfactory->CreateTextFormat(family,
		NULL,
		dwriteWeights[initialStyle->Weight],
		dwriteItalics[initialStyle->Italic],
		dwriteStretches[initialStyle->Stretch],
		// typographic points are 1/72 inch; this parameter is 1/96 inch
		// fortunately Microsoft does this too, in https://msdn.microsoft.com/en-us/library/windows/desktop/dd371554%28v=vs.85%29.aspx
		initialStyle->Size * (96.0 / 72.0),
		// see http://stackoverflow.com/questions/28397971/idwritefactorycreatetextformat-failing and https://msdn.microsoft.com/en-us/library/windows/desktop/dd368203.aspx
		// TODO use the current locale again?
		L"",
		&(layout->format));
	if (hr != S_OK)
		logHRESULT("error creating IDWriteTextFormat in uiDrawNewTextLayout()", hr);
	uiFree(family);
	// TODO gravity

	layout->bytesToCharacters = toUTF16Offsets(text, &wtext, &wlen);
	hr = dwfactory->CreateTextLayout(wstr, wlen,
		layout->format,
		// FLOAT is float, not double, so this should work... TODO
		FLT_MAX, FLT_MAX,
		&(layout->layout));
	if (hr != S_OK)
		logHRESULT("error creating IDWriteTextLayout in uiDrawNewTextLayout()", hr);
	uiFree(wtext);

	return layout;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *layout)
{
	layout->layout->Release();
	layout->format->Release();
	uiFree(layout);
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
