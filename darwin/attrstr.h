// 4 march 2018
#import "../common/attrstr.h"

// opentype.m
extern CFArrayRef uiprivOpenTypeFeaturesToCTFeatures(const uiOpenTypeFeatures *otf);

// aat.m
typedef void (^uiprivAATBlock)(uint16_t type, uint16_t selector);
extern void uiprivOpenTypeToAAT(char a, char b, char c, char d, uint32_t value, uiprivAATBlock f);
