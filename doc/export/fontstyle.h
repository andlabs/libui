// 3 november 2017

// fontmatch.m
@interface fontStyleData : NSObject {
	CTFontRef font;
	CTFontDescriptorRef desc;
	CFDictionaryRef traits;
	CTFontSymbolicTraits symbolic;
	double weight;
	double width;
	BOOL didStyleName;
	CFStringRef styleName;
	BOOL didVariations;
	CFDictionaryRef variations;
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
- (CFDictionaryRef)variations;
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

// fonttraits.m
extern void processFontTraits(fontStyleData *d, uiDrawFontDescriptor *out);

// fontvariation.m
extern NSDictionary *mkVariationAxisDict(CFArrayRef axes, CFDataRef avarTable);
extern void processFontVariation(fontStyleData *d, NSDictionary *axisDict, uiDrawFontDescriptor *out);
