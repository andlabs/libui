// 11 march 2018
#include "uipriv_windows.hpp"
#include "attrstr.hpp"

// TODO should be const but then I can't operator[] on it; the real solution is to find a way to do designated array initializers in C++11 but I do not know enough C++ voodoo to make it work (it is possible but no one else has actually done it before)
static std::map<uiTextItalic, DWRITE_FONT_STYLE> dwriteItalics = {
	{ uiTextItalicNormal, DWRITE_FONT_STYLE_NORMAL },
	{ uiTextItalicOblique, DWRITE_FONT_STYLE_OBLIQUE },
	{ uiTextItalicItalic, DWRITE_FONT_STYLE_ITALIC },
};

// TODO should be const but then I can't operator[] on it; the real solution is to find a way to do designated array initializers in C++11 but I do not know enough C++ voodoo to make it work (it is possible but no one else has actually done it before)
static std::map<uiTextStretch, DWRITE_FONT_STRETCH> dwriteStretches = {
	{ uiTextStretchUltraCondensed, DWRITE_FONT_STRETCH_ULTRA_CONDENSED },
	{ uiTextStretchExtraCondensed, DWRITE_FONT_STRETCH_EXTRA_CONDENSED },
	{ uiTextStretchCondensed, DWRITE_FONT_STRETCH_CONDENSED },
	{ uiTextStretchSemiCondensed, DWRITE_FONT_STRETCH_SEMI_CONDENSED },
	{ uiTextStretchNormal, DWRITE_FONT_STRETCH_NORMAL },
	{ uiTextStretchSemiExpanded, DWRITE_FONT_STRETCH_SEMI_EXPANDED },
	{ uiTextStretchExpanded, DWRITE_FONT_STRETCH_EXPANDED },
	{ uiTextStretchExtraExpanded, DWRITE_FONT_STRETCH_EXTRA_EXPANDED },
	{ uiTextStretchUltraExpanded, DWRITE_FONT_STRETCH_ULTRA_EXPANDED },
};

// for the most part, DirectWrite weights correlate to ours
// the differences:
// - Minimum — libui: 0, DirectWrite: 1
// - Maximum — libui: 1000, DirectWrite: 999
// TODO figure out what to do about this shorter range (the actual major values are the same (but with different names), so it's just a range issue)
DWRITE_FONT_WEIGHT uiprivWeightToDWriteWeight(uiTextWeight w)
{
	return (DWRITE_FONT_WEIGHT) w;
}

DWRITE_FONT_STYLE uiprivItalicToDWriteStyle(uiTextItalic i)
{
	return dwriteItalics[i];
}

DWRITE_FONT_STRETCH uiprivStretchToDWriteStretch(uiTextStretch s)
{
	return dwriteStretches[s];
}

void uiprivFontDescriptorFromIDWriteFont(IDWriteFont *font, uiFontDescriptor *uidesc)
{
	DWRITE_FONT_STYLE dwitalic;
	DWRITE_FONT_STRETCH dwstretch;

	dwitalic = font->GetStyle();
	// TODO reverse the above misalignment if it is corrected
	uidesc->Weight = (uiTextWeight) (font->GetWeight());
	dwstretch = font->GetStretch();

	for (uidesc->Italic = uiTextItalicNormal; uidesc->Italic < uiTextItalicItalic; uidesc->Italic++)
		if (dwriteItalics[uidesc->Italic] == dwitalic)
			break;
	for (uidesc->Stretch = uiTextStretchUltraCondensed; uidesc->Stretch < uiTextStretchUltraExpanded; uidesc->Stretch++)
		if (dwriteStretches[uidesc->Stretch] == dwstretch)
			break;
}
