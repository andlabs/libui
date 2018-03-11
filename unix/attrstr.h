// 11 march 2018
#import "../common/attrstr.h"

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
