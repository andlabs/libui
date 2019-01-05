// 12 february 2017
#include "uipriv_unix.h"
#include "attrstr.h"

// TODO pango alpha attributes turn 0 into 65535 :|

// TODO make this name less generic?
struct foreachParams {
	PangoAttrList *attrs;
};

static void addattr(struct foreachParams *p, size_t start, size_t end, PangoAttribute *attr)
{
	if (attr == NULL)		// in case of a future attribute
		return;
	attr->start_index = start;
	attr->end_index = end;
	pango_attr_list_insert(p->attrs, attr);
}

static uiForEach processAttribute(const uiAttributedString *s, const uiAttribute *attr, size_t start, size_t end, void *data)
{
	struct foreachParams *p = (struct foreachParams *) data;
	double r, g, b, a;
	PangoUnderline underline;
	uiUnderlineColor colorType;
	const uiOpenTypeFeatures *features;
	GString *featurestr;

	switch (uiAttributeGetType(attr)) {
	case uiAttributeTypeFamily:
		addattr(p, start, end,
			pango_attr_family_new(uiAttributeFamily(attr)));
		break;
	case uiAttributeTypeSize:
		addattr(p, start, end,
			pango_attr_size_new(cairoToPango(uiAttributeSize(attr))));
		break;
	case uiAttributeTypeWeight:
		// TODO reverse the misalignment from drawtext.c if it is corrected 
		addattr(p, start, end,
			pango_attr_weight_new(uiprivWeightToPangoWeight(uiAttributeWeight(attr))));
		break;
	case uiAttributeTypeItalic:
		addattr(p, start, end,
			pango_attr_style_new(uiprivItalicToPangoStyle(uiAttributeItalic(attr))));
		break;
	case uiAttributeTypeStretch:
		addattr(p, start, end,
			pango_attr_stretch_new(uiprivStretchToPangoStretch(uiAttributeStretch(attr))));
		break;
	case uiAttributeTypeColor:
		uiAttributeColor(attr, &r, &g, &b, &a);
		addattr(p, start, end,
			pango_attr_foreground_new(
				(guint16) (r * 65535.0),
				(guint16) (g * 65535.0),
				(guint16) (b * 65535.0)));
		addattr(p, start, end,
			uiprivFUTURE_pango_attr_foreground_alpha_new(
				(guint16) (a * 65535.0)));
		break;
	case uiAttributeTypeBackground:
		// TODO make sure this works properly with line paragraph spacings (after figuring out what that means, of course)
		uiAttributeColor(attr, &r, &g, &b, &a);
		addattr(p, start, end,
			pango_attr_background_new(
				(guint16) (r * 65535.0),
				(guint16) (g * 65535.0),
				(guint16) (b * 65535.0)));
		addattr(p, start, end,
			uiprivFUTURE_pango_attr_background_alpha_new(
				(guint16) (a * 65535.0)));
		break;
	case uiAttributeTypeUnderline:
		switch (uiAttributeUnderline(attr)) {
		case uiUnderlineNone:
			underline = PANGO_UNDERLINE_NONE;
			break;
		case uiUnderlineSingle:
			underline = PANGO_UNDERLINE_SINGLE;
			break;
		case uiUnderlineDouble:
			underline = PANGO_UNDERLINE_DOUBLE;
			break;
		case uiUnderlineSuggestion:
			underline = PANGO_UNDERLINE_ERROR;
			break;
		}
		addattr(p, start, end,
			pango_attr_underline_new(underline));
		break;
	case uiAttributeTypeUnderlineColor:
		uiAttributeUnderlineColor(attr, &colorType, &r, &g, &b, &a);
		switch (colorType) {
		case uiUnderlineColorCustom:
			addattr(p, start, end,
				pango_attr_underline_color_new(
					(guint16) (r * 65535.0),
					(guint16) (g * 65535.0),
					(guint16) (b * 65535.0)));
			break;
		case uiUnderlineColorSpelling:
			// TODO GtkTextView style property error-underline-color
			addattr(p, start, end,
				pango_attr_underline_color_new(65535, 0, 0));
			break;
		case uiUnderlineColorGrammar:
			// TODO find a more appropriate color
			addattr(p, start, end,
				pango_attr_underline_color_new(0, 65535, 0));
			break;
		case uiUnderlineColorAuxiliary:
			// TODO find a more appropriate color
			addattr(p, start, end,
				pango_attr_underline_color_new(0, 0, 65535));
			break;
		}
		break;
	case uiAttributeTypeFeatures:
		// only generate an attribute if the features object is not NULL
		// TODO state that this is allowed
		features = uiAttributeFeatures(attr);
		if (features == NULL)
			break;
		featurestr = uiprivOpenTypeFeaturesToPangoCSSFeaturesString(features);
		addattr(p, start, end,
			uiprivFUTURE_pango_attr_font_features_new(featurestr->str));
		g_string_free(featurestr, TRUE);
		break;
	default:
		// TODO complain
		;
	}
	return uiForEachContinue;
}

PangoAttrList *uiprivAttributedStringToPangoAttrList(uiDrawTextLayoutParams *p)
{
	struct foreachParams fep;

	fep.attrs = pango_attr_list_new();
	uiAttributedStringForEachAttribute(p->String, processAttribute, &fep);
	return fep.attrs;
}
