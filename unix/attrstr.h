// 11 march 2018
#import "../common/attrstr.h"

// See https://developer.gnome.org/pango/1.30/pango-Cairo-Rendering.html#pango-Cairo-Rendering.description
// For the conversion, see https://developer.gnome.org/pango/1.30/pango-Glyph-Storage.html#pango-units-to-double and https://developer.gnome.org/pango/1.30/pango-Glyph-Storage.html#pango-units-from-double
#define pangoToCairo(pango) (pango_units_to_double(pango))
#define cairoToPango(cairo) (pango_units_from_double(cairo))

// opentype.c
extern GString *uiprivOpenTypeFeaturesToPangoCSSFeaturesString(const uiOpenTypeFeatures *otf);

// attrstr.c
extern PangoAttrList *uiprivAttributedStringToPangoAttrList(uiDrawTextLayoutParams *p, GPtrArray **backgroundParams);

// drawtext.c
// TODO figure out where this type should *really* go in all the headers...
typedef struct uiprivDrawTextBackgroundParams uiprivDrawTextBackgroundParams;
struct uiprivDrawTextBackgroundParams {
	size_t start;
	size_t end;
	double r;
	double g;
	double b;
	double a;
};
// TODO move this to a fontmatch.c
extern const PangoStyle uiprivPangoItalics[];
extern const PangoStretch uiprivPangoStretches[];
