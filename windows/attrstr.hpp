// 11 march 2018
extern "C" {
#include "../common/attrstr.h"
}

// opentype.cpp
extern IDWriteTypography *uiprivOpenTypeFeaturesToIDWriteTypography(const uiOpenTypeFeatures *otf);

// fontmatch.cpp
extern DWRITE_FONT_WEIGHT uiprivWeightToDWriteWeight(uiTextWeight w);
extern DWRITE_FONT_STYLE uiprivItalicToDWriteStyle(uiTextItalic i);
extern DWRITE_FONT_STRETCH uiprivStretchToDWriteStretch(uiTextStretch s);
extern void uiprivFontDescriptorFromIDWriteFont(IDWriteFont *font, uiDrawFontDescriptor *uidesc);
