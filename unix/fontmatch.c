// 11 march 2018
#include "uipriv_unix.h"
#include "attrstr.h"

static const PangoStyle pangoItalics[] = {
	[uiTextItalicNormal] = PANGO_STYLE_NORMAL,
	[uiTextItalicOblique] = PANGO_STYLE_OBLIQUE,
	[uiTextItalicItalic] = PANGO_STYLE_ITALIC,
};

static const PangoStretch pangoStretches[] = {
	[uiTextStretchUltraCondensed] = PANGO_STRETCH_ULTRA_CONDENSED,
	[uiTextStretchExtraCondensed] = PANGO_STRETCH_EXTRA_CONDENSED,
	[uiTextStretchCondensed] = PANGO_STRETCH_CONDENSED,
	[uiTextStretchSemiCondensed] = PANGO_STRETCH_SEMI_CONDENSED,
	[uiTextStretchNormal] = PANGO_STRETCH_NORMAL,
	[uiTextStretchSemiExpanded] = PANGO_STRETCH_SEMI_EXPANDED,
	[uiTextStretchExpanded] = PANGO_STRETCH_EXPANDED,
	[uiTextStretchExtraExpanded] = PANGO_STRETCH_EXTRA_EXPANDED,
	[uiTextStretchUltraExpanded] = PANGO_STRETCH_ULTRA_EXPANDED,
};

// for the most part, pango weights correlate to ours
// the differences:
// - Book — libui: 350, Pango: 380
// - Ultra Heavy — libui: 950, Pango: 1000
// TODO figure out what to do about this misalignment
PangoWeight uiprivWeightToPangoWeight(uiTextWeight w)
{
	return (PangoWeight) w;
}

PangoStyle uiprivItalicToPangoStyle(uiTextItalic i)
{
	return pangoItalics[i];
}

PangoStretch uiprivStretchToPangoStretch(uiTextStretch s)
{
	return pangoStretches[s];
}

PangoFontDescription *uiprivFontDescriptorToPangoFontDescription(const uiFontDescriptor *uidesc)
{
	PangoFontDescriptor *desc;

	desc = pango_font_description_new();
	pango_font_description_set_family(desc, uidesc->Family);
	// see https://developer.gnome.org/pango/1.30/pango-Fonts.html#pango-font-description-set-size and https://developer.gnome.org/pango/1.30/pango-Glyph-Storage.html#pango-units-from-double
	pango_font_description_set_size(desc, pango_units_from_double(uidesc->Size));
	pango_font_description_set_weight(desc, uiprivWeightToPangoWeight(uidesc->Weight));
	pango_font_description_set_style(desc, uiprivItalicToPangoStyle(uidesc->Italic));
	pango_font_description_set_stretch(desc, uiprivStretchToPangoStretch(uidesc->Stretch));
	return desc;
}
