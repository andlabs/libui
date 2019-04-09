// 11 march 2018
#include "../common/attrstr.h"

// See https://developer.gnome.org/pango/1.30/pango-Cairo-Rendering.html#pango-Cairo-Rendering.description
// For the conversion, see https://developer.gnome.org/pango/1.30/pango-Glyph-Storage.html#pango-units-to-double and https://developer.gnome.org/pango/1.30/pango-Glyph-Storage.html#pango-units-from-double
#define pangoToCairo(pango) (pango_units_to_double(pango))
#define cairoToPango(cairo) (pango_units_from_double(cairo))

// opentype.c
extern GString *uiprivOpenTypeFeaturesToPangoCSSFeaturesString(const uiOpenTypeFeatures *otf);

// fontmatch.c
extern PangoWeight uiprivWeightToPangoWeight(uiTextWeight w);
extern PangoStyle uiprivItalicToPangoStyle(uiTextItalic i);
extern PangoStretch uiprivStretchToPangoStretch(uiTextStretch s);
extern PangoFontDescription *uiprivFontDescriptorToPangoFontDescription(const uiFontDescriptor *uidesc);
extern void uiprivFontDescriptorFromPangoFontDescription(PangoFontDescription *pdesc, uiFontDescriptor *uidesc);

// attrstr.c
extern PangoAttrList *uiprivAttributedStringToPangoAttrList(uiDrawTextLayoutParams *p);
