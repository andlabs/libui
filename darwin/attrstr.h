// 4 march 2018
#import "../common/attrstr.h"

// opentype.m
extern CFArrayRef uiprivOpenTypeFeaturesToCTFeatures(const uiOpenTypeFeatures *otf);

// aat.m
typedef void (^uiprivAATBlock)(uint16_t type, uint16_t selector);
extern void uiprivOpenTypeToAAT(char a, char b, char c, char d, uint32_t value, uiprivAATBlock f);

// fontmatch.m
@interface uiprivFontStyleData : NSObject {
	CTFontRef font;
	CTFontDescriptorRef desc;
	CFDictionaryRef traits;
	CTFontSymbolicTraits symbolic;
	double weight;
	double width;
	BOOL didStyleName;
	CFStringRef styleName;
	BOOL didVariation;
	CFDictionaryRef variation;
	BOOL hasRegistrationScope;
	CTFontManagerScope registrationScope;
	BOOL didPostScriptName;
	CFStringRef postScriptName;
	CTFontFormat fontFormat;
	BOOL didPreferredSubFamilyName;
	CFStringRef preferredSubFamilyName;
	BOOL didSubFamilyName;
	CFStringRef subFamilyName;
	BOOL didFullName;
	CFStringRef fullName;
	BOOL didPreferredFamilyName;
	CFStringRef preferredFamilyName;
	BOOL didFamilyName;
	CFStringRef familyName;
	BOOL didVariationAxes;
	CFArrayRef variationAxes;
}
- (id)initWithFont:(CTFontRef)f;
- (id)initWithDescriptor:(CTFontDescriptorRef)d;
- (BOOL)prepare;
- (void)ensureFont;
- (CTFontSymbolicTraits)symbolicTraits;
- (double)weight;
- (double)width;
- (CFStringRef)styleName;
- (CFDictionaryRef)variation;
- (BOOL)hasRegistrationScope;
- (CTFontManagerScope)registrationScope;
- (CFStringRef)postScriptName;
- (CFDataRef)table:(CTFontTableTag)tag;
- (CTFontFormat)fontFormat;
- (CFStringRef)fontName:(CFStringRef)key;
- (CFStringRef)preferredSubFamilyName;
- (CFStringRef)subFamilyName;
- (CFStringRef)fullName;
- (CFStringRef)preferredFamilyName;
- (CFStringRef)familyName;
- (CFArrayRef)variationAxes;
@end
extern CTFontDescriptorRef uiprivFontDescriptorToCTFontDescriptor(uiFontDescriptor *fd);
extern CTFontDescriptorRef uiprivCTFontDescriptorAppendFeatures(CTFontDescriptorRef desc, const uiOpenTypeFeatures *otf);
extern void uiprivFontDescriptorFromCTFontDescriptor(CTFontDescriptorRef ctdesc, uiFontDescriptor *uidesc);

// fonttraits.m
extern void uiprivProcessFontTraits(uiprivFontStyleData *d, uiFontDescriptor *out);

// fontvariation.m
extern NSDictionary *uiprivMakeVariationAxisDict(CFArrayRef axes, CFDataRef avarTable);
extern void uiprivProcessFontVariation(uiprivFontStyleData *d, NSDictionary *axisDict, uiFontDescriptor *out);

// attrstr.m
extern void uiprivInitUnderlineColors(void);
extern void uiprivUninitUnderlineColors(void);
typedef void (^backgroundBlock)(uiDrawContext *c, uiDrawTextLayout *layout, double x, double y);
extern CFAttributedStringRef uiprivAttributedStringToCFAttributedString(uiDrawTextLayoutParams *p, NSArray **backgroundBlocks);
