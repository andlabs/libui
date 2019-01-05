// 12 february 2017
#import "uipriv_darwin.h"
#import "attrstr.h"

// this is what AppKit does internally
// WebKit does this too; see https://github.com/adobe/webkit/blob/master/Source/WebCore/platform/graphics/mac/GraphicsContextMac.mm
static NSColor *spellingColor = nil;
static NSColor *grammarColor = nil;
static NSColor *auxiliaryColor = nil;

static NSColor *tryColorNamed(NSString *name)
{
	NSImage *img;

	img = [NSImage imageNamed:name];
	if (img == nil)
		return nil;
	return [NSColor colorWithPatternImage:img];
}

void uiprivInitUnderlineColors(void)
{
	spellingColor = tryColorNamed(@"NSSpellingDot");
	if (spellingColor == nil) {
		// WebKit says this is needed for "older systems"; not sure how old, but 10.11 AppKit doesn't look for this
		spellingColor = tryColorNamed(@"SpellingDot");
		if (spellingColor == nil)
			spellingColor = [NSColor redColor];
	}
	[spellingColor retain];		// override autoreleasing

	grammarColor = tryColorNamed(@"NSGrammarDot");
	if (grammarColor == nil) {
		// WebKit says this is needed for "older systems"; not sure how old, but 10.11 AppKit doesn't look for this
		grammarColor = tryColorNamed(@"GrammarDot");
		if (grammarColor == nil)
			grammarColor = [NSColor greenColor];
	}
	[grammarColor retain];		// override autoreleasing

	auxiliaryColor = tryColorNamed(@"NSCorrectionDot");
	if (auxiliaryColor == nil) {
		// WebKit says this is needed for "older systems"; not sure how old, but 10.11 AppKit doesn't look for this
		auxiliaryColor = tryColorNamed(@"CorrectionDot");
		if (auxiliaryColor == nil)
			auxiliaryColor = [NSColor blueColor];
	}
	[auxiliaryColor retain];		// override autoreleasing
}

void uiprivUninitUnderlineColors(void)
{
	[auxiliaryColor release];
	auxiliaryColor = nil;
	[grammarColor release];
	grammarColor = nil;
	[spellingColor release];
	spellingColor = nil;
}

// TODO opentype features are lost when using uiFontDescriptor, so a handful of fonts in the font panel ("Titling" variants of some fonts and possibly others but those are the examples I know about) cannot be represented by uiFontDescriptor; what *can* we do about this since this info is NOT part of the font on other platforms?
// TODO see if we could use NSAttributedString?
// TODO consider renaming this struct and the fep variable(s)
// TODO restructure all this so the important details at the top are below with the combined font attributes type?
// TODO in fact I should just write something to explain everything in this file...
struct foreachParams {
	CFMutableAttributedStringRef mas;
	NSMutableArray *backgroundParams;
};

// unlike the other systems, Core Text rolls family, size, weight, italic, width, AND opentype features into the "font" attribute
// instead of incrementally adjusting CTFontRefs (which, judging from NSFontManager, seems finicky and UI-centric), we use a custom class to incrementally store attributes that go into a CTFontRef, and then convert everything to CTFonts en masse later
// https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/AttributedStrings/Tasks/ChangingAttrStrings.html#//apple_ref/doc/uid/20000162-BBCBGCDG says we must have -hash and -isEqual: workign properly for this to work, so we must do that too, using a basic xor-based hash and leveraging Cocoa -hash implementations where useful and feasible (if not necessary)
// TODO structure and rewrite this part
// TODO re-find sources proving support of custom attributes
// TODO what if this is NULL?
static const CFStringRef combinedFontAttrName = CFSTR("libuiCombinedFontAttribute");

enum {
	cFamily,
	cSize,
	cWeight,
	cItalic,
	cStretch,
	cFeatures,
	nc,
};

static const int toc[] = {
	[uiAttributeTypeFamily] = cFamily,
	[uiAttributeTypeSize] = cSize,
	[uiAttributeTypeWeight] = cWeight,
	[uiAttributeTypeItalic] = cItalic,
	[uiAttributeTypeStretch] = cStretch,
	[uiAttributeTypeFeatures] = cFeatures,
};

static uiForEach featuresHash(const uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value, void *data)
{
	NSUInteger *hash = (NSUInteger *) data;
	uint32_t tag;

	tag = (((uint32_t) a) & 0xFF) << 24;
	tag |= (((uint32_t) b) & 0xFF) << 16;
	tag |= (((uint32_t) c) & 0xFF) << 8;
	tag |= ((uint32_t) d) & 0xFF;
	*hash ^= tag;
	*hash ^= value;
	return uiForEachContinue;
}

@interface uiprivCombinedFontAttr : NSObject<NSCopying> {
	uiAttribute *attrs[nc];
	BOOL hasHash;
	NSUInteger hash;
}
- (void)addAttribute:(uiAttribute *)attr;
- (CTFontRef)toCTFontWithDefaultFont:(uiFontDescriptor *)defaultFont;
@end

@implementation uiprivCombinedFontAttr

- (id)init
{
	self = [super init];
	if (self) {
		memset(self->attrs, 0, nc * sizeof (uiAttribute *));
		self->hasHash = NO;
	}
	return self;
}

- (void)dealloc
{
	int i;

	for (i = 0; i < nc; i++)
		if (self->attrs[i] != NULL) {
			uiprivAttributeRelease(self->attrs[i]);
			self->attrs[i] = NULL;
		}
	[super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
	uiprivCombinedFontAttr *ret;
	int i;

	ret = [[uiprivCombinedFontAttr allocWithZone:zone] init];
	for (i = 0; i < nc; i++)
		if (self->attrs[i] != NULL)
			ret->attrs[i] = uiprivAttributeRetain(self->attrs[i]);
	ret->hasHash = self->hasHash;
	ret->hash = self->hash;
	return ret;
}

- (void)addAttribute:(uiAttribute *)attr
{
	int index;

	index = toc[uiAttributeGetType(attr)];
	if (self->attrs[index] != NULL)
		uiprivAttributeRelease(self->attrs[index]);
	self->attrs[index] = uiprivAttributeRetain(attr);
	self->hasHash = NO;
}

- (BOOL)isEqual:(id)bb
{
	uiprivCombinedFontAttr *b = (uiprivCombinedFontAttr *) bb;
	int i;

	if (b == nil)
		return NO;
	for (i = 0; i < nc; i++) {
		if (self->attrs[i] == NULL && b->attrs[i] == NULL)
			continue;
		if (self->attrs[i] == NULL || b->attrs[i] == NULL)
			return NO;
		if (!uiprivAttributeEqual(self->attrs[i], b->attrs[i]))
			return NO;
	}
	return YES;
}

- (NSUInteger)hash
{
	if (self->hasHash)
		return self->hash;
	@autoreleasepool {
		NSString *family;
		NSNumber *size;

		self->hash = 0;
		if (self->attrs[cFamily] != NULL) {
			family = [NSString stringWithUTF8String:uiAttributeFamily(self->attrs[cFamily])];
			// TODO make sure this aligns with case-insensitive compares when those are done in common/attribute.c
			self->hash ^= [[family uppercaseString] hash];
		}
		if (self->attrs[cSize] != NULL) {
			size = [NSNumber numberWithDouble:uiAttributeSize(self->attrs[cSize])];
			self->hash ^= [size hash];
		}
		if (self->attrs[cWeight] != NULL)
			self->hash ^= (NSUInteger) uiAttributeWeight(self->attrs[cWeight]);
		if (self->attrs[cItalic] != NULL)
			self->hash ^= (NSUInteger) uiAttributeItalic(self->attrs[cItalic]);
		if (self->attrs[cStretch] != NULL)
			self->hash ^= (NSUInteger) uiAttributeStretch(self->attrs[cStretch]);
		if (self->attrs[cFeatures] != NULL)
			uiOpenTypeFeaturesForEach(uiAttributeFeatures(self->attrs[cFeatures]), featuresHash, &(self->hash));
		self->hasHash = YES;
	}
	return self->hash;
}

- (CTFontRef)toCTFontWithDefaultFont:(uiFontDescriptor *)defaultFont
{
	uiFontDescriptor uidesc;
	CTFontDescriptorRef desc;
	CTFontRef font;

	uidesc = *defaultFont;
	if (self->attrs[cFamily] != NULL)
		// TODO const-correct uiFontDescriptor or change this function below
		uidesc.Family = (char *) uiAttributeFamily(self->attrs[cFamily]);
	if (self->attrs[cSize] != NULL)
		uidesc.Size = uiAttributeSize(self->attrs[cSize]);
	if (self->attrs[cWeight] != NULL)
		uidesc.Weight = uiAttributeWeight(self->attrs[cWeight]);
	if (self->attrs[cItalic] != NULL)
		uidesc.Italic = uiAttributeItalic(self->attrs[cItalic]);
	if (self->attrs[cStretch] != NULL)
		uidesc.Stretch = uiAttributeStretch(self->attrs[cStretch]);
	desc = uiprivFontDescriptorToCTFontDescriptor(&uidesc);
	if (self->attrs[cFeatures] != NULL)
		desc = uiprivCTFontDescriptorAppendFeatures(desc, uiAttributeFeatures(self->attrs[cFeatures]));
	font = CTFontCreateWithFontDescriptor(desc, uidesc.Size, NULL);
	CFRelease(desc);			// TODO correct?
	return font;
}

@end

static void addFontAttributeToRange(struct foreachParams *p, size_t start, size_t end, uiAttribute *attr)
{
	uiprivCombinedFontAttr *cfa;
	CFRange range;
	size_t diff;

	while (start < end) {
		cfa = (uiprivCombinedFontAttr *) CFAttributedStringGetAttribute(p->mas, start, combinedFontAttrName, &range);
		if (cfa == nil)
			cfa = [uiprivCombinedFontAttr new];
		else
			cfa = [cfa copy];
		[cfa addAttribute:attr];
		// clamp range within [start, end)
		if (range.location < start) {
			diff = start - range.location;
			range.location = start;
			range.length -= diff;
		}
		if ((range.location + range.length) > end)
			range.length = end - range.location;
		CFAttributedStringSetAttribute(p->mas, range, combinedFontAttrName, cfa);
		[cfa release];
		start += range.length;
	}
}

static CGColorRef mkcolor(double r, double g, double b, double a)
{
	CGColorSpaceRef colorspace;
	CGColorRef color;
	CGFloat components[4];

	// TODO we should probably just create this once and recycle it throughout program execution...
	colorspace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
	if (colorspace == NULL) {
		// TODO
	}
	components[0] = r;
	components[1] = g;
	components[2] = b;
	components[3] = a;
	color = CGColorCreate(colorspace, components);
	CFRelease(colorspace);
	return color;
}

static void addBackgroundAttribute(struct foreachParams *p, size_t start, size_t end, double r, double g, double b, double a)
{
	uiprivDrawTextBackgroundParams *dtb;

	// TODO make sure this works properly with line paragraph spacings (after figuring out what that means, of course)
	if (uiprivFUTURE_kCTBackgroundColorAttributeName != NULL) {
		CGColorRef color;
		CFRange range;

		color = mkcolor(r, g, b, a);
		range.location = start;
		range.length = end - start;
		CFAttributedStringSetAttribute(p->mas, range, *uiprivFUTURE_kCTBackgroundColorAttributeName, color);
		CFRelease(color);
		return;
	}

	dtb = [[uiprivDrawTextBackgroundParams alloc] initWithStart:start end:end r:r g:g b:b a:a];
	[p->backgroundParams addObject:dtb];
	[dtb release];
}

static uiForEach processAttribute(const uiAttributedString *s, const uiAttribute *attr, size_t start, size_t end, void *data)
{
	struct foreachParams *p = (struct foreachParams *) data;
	CFRange range;
	CGColorRef color;
	int32_t us;
	CFNumberRef num;
	double r, g, b, a;
	uiUnderlineColor colorType;

	start = uiprivAttributedStringUTF8ToUTF16(s, start);
	end = uiprivAttributedStringUTF8ToUTF16(s, end);
	range.location = start;
	range.length = end - start;
	switch (uiAttributeGetType(attr)) {
	case uiAttributeTypeFamily:
	case uiAttributeTypeSize:
	case uiAttributeTypeWeight:
	case uiAttributeTypeItalic:
	case uiAttributeTypeStretch:
	case uiAttributeTypeFeatures:
		addFontAttributeToRange(p, start, end, attr);
		break;
	case uiAttributeTypeColor:
		uiAttributeColor(attr, &r, &g, &b, &a);
		color = mkcolor(r, g, b, a);
		CFAttributedStringSetAttribute(p->mas, range, kCTForegroundColorAttributeName, color);
		CFRelease(color);
		break;
	case uiAttributeTypeBackground:
		uiAttributeColor(attr, &r, &g, &b, &a);
		addBackgroundAttribute(p, start, end, r, g, b, a);
		break;
	// TODO turn into a class, like we did with the font attributes, or even integrate *into* the font attributes
	case uiAttributeTypeUnderline:
		switch (uiAttributeUnderline(attr)) {
		case uiUnderlineNone:
			us = kCTUnderlineStyleNone;
			break;
		case uiUnderlineSingle:
			us = kCTUnderlineStyleSingle;
			break;
		case uiUnderlineDouble:
			us = kCTUnderlineStyleDouble;
			break;
		case uiUnderlineSuggestion:
			// TODO incorrect if a solid color
			us = kCTUnderlineStyleThick;
			break;
		}
		num = CFNumberCreate(NULL, kCFNumberSInt32Type, &us);
		CFAttributedStringSetAttribute(p->mas, range, kCTUnderlineStyleAttributeName, num);
		CFRelease(num);
		break;
	case uiAttributeTypeUnderlineColor:
		uiAttributeUnderlineColor(attr, &colorType, &r, &g, &b, &a);
		switch (colorType) {
		case uiUnderlineColorCustom:
			color = mkcolor(r, g, b, a);
			break;
		case uiUnderlineColorSpelling:
			color = [spellingColor CGColor];
			break;
		case uiUnderlineColorGrammar:
			color = [grammarColor CGColor];
			break;
		case uiUnderlineColorAuxiliary:
			color = [auxiliaryColor CGColor];
			break;
		}
		CFAttributedStringSetAttribute(p->mas, range, kCTUnderlineColorAttributeName, color);
		if (colorType == uiUnderlineColorCustom)
			CFRelease(color);
		break;
	}
	return uiForEachContinue;
}

static void applyFontAttributes(CFMutableAttributedStringRef mas, uiFontDescriptor *defaultFont)
{
	uiprivCombinedFontAttr *cfa;
	CTFontRef font;
	CFRange range;
	CFIndex n;

	n = CFAttributedStringGetLength(mas);

	// first apply the default font to the entire string
	// TODO is this necessary given the #if 0'd code in uiprivAttributedStringToCFAttributedString()?
	cfa = [uiprivCombinedFontAttr new];
	font = [cfa toCTFontWithDefaultFont:defaultFont];
	[cfa release];
	range.location = 0;
	range.length = n;
	CFAttributedStringSetAttribute(mas, range, kCTFontAttributeName, font);
	CFRelease(font);

	// now go through, replacing every uiprivCombinedFontAttr with the proper CTFontRef
	// we are best off treating series of identical fonts as single ranges ourselves for parity across platforms, even if OS X does something similar itself
	range.location = 0;
	while (range.location < n) {
		// TODO consider seeing if CFAttributedStringGetAttributeAndLongestEffectiveRange() can make things faster by reducing the number of potential iterations, either here or above
		cfa = (uiprivCombinedFontAttr *) CFAttributedStringGetAttribute(mas, range.location, combinedFontAttrName, &range);
		if (cfa != nil) {
			font = [cfa toCTFontWithDefaultFont:defaultFont];
			CFAttributedStringSetAttribute(mas, range, kCTFontAttributeName, font);
			CFRelease(font);
		}
		range.location += range.length;
	}

	// and finally, get rid of all the uiprivCombinedFontAttrs as we won't need them anymore
	range.location = 0;
	range.length = 0;
	CFAttributedStringRemoveAttribute(mas, range, combinedFontAttrName);
}

static const CTTextAlignment ctaligns[] = {
	[uiDrawTextAlignLeft] = kCTTextAlignmentLeft,
	[uiDrawTextAlignCenter] = kCTTextAlignmentCenter,
	[uiDrawTextAlignRight] = kCTTextAlignmentRight,
};

static CTParagraphStyleRef mkParagraphStyle(uiDrawTextLayoutParams *p)
{
	CTParagraphStyleRef ps;
	CTParagraphStyleSetting settings[16];
	size_t nSettings = 0;

	settings[nSettings].spec = kCTParagraphStyleSpecifierAlignment;
	settings[nSettings].valueSize = sizeof (CTTextAlignment);
	settings[nSettings].value = ctaligns + p->Align;
	nSettings++;

	ps = CTParagraphStyleCreate(settings, nSettings);
	if (ps == NULL) {
		// TODO
	}
	return ps;
}

// TODO either rename this (on all platforms) to uiprivDrawTextLayoutParams... or rename this file or both or split the struct or something else...
CFAttributedStringRef uiprivAttributedStringToCFAttributedString(uiDrawTextLayoutParams *p, NSArray **backgroundParams)
{
	CFStringRef cfstr;
	CFMutableDictionaryRef defaultAttrs;
	CTParagraphStyleRef ps;
	CFAttributedStringRef base;
	CFMutableAttributedStringRef mas;
	struct foreachParams fep;

	cfstr = CFStringCreateWithCharacters(NULL, uiprivAttributedStringUTF16String(p->String), uiprivAttributedStringUTF16Len(p->String));
	if (cfstr == NULL) {
		// TODO
	}
	defaultAttrs = CFDictionaryCreateMutable(NULL, 0,
		&kCFCopyStringDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	if (defaultAttrs == NULL) {
		// TODO
	}
#if 0 /* TODO */
	ffp.desc = *(p->DefaultFont);
	defaultCTFont = fontdescToCTFont(&ffp);
	CFDictionaryAddValue(defaultAttrs, kCTFontAttributeName, defaultCTFont);
	CFRelease(defaultCTFont);
#endif
	ps = mkParagraphStyle(p);
	CFDictionaryAddValue(defaultAttrs, kCTParagraphStyleAttributeName, ps);
	CFRelease(ps);

	base = CFAttributedStringCreate(NULL, cfstr, defaultAttrs);
	if (base == NULL) {
		// TODO
	}
	CFRelease(cfstr);
	CFRelease(defaultAttrs);
	mas = CFAttributedStringCreateMutableCopy(NULL, 0, base);
	CFRelease(base);

	CFAttributedStringBeginEditing(mas);
	fep.mas = mas;
	fep.backgroundParams = [NSMutableArray new];
	uiAttributedStringForEachAttribute(p->String, processAttribute, &fep);
	applyFontAttributes(mas, p->DefaultFont);
	CFAttributedStringEndEditing(mas);

	*backgroundParams = fep.backgroundParams;
	return mas;
}
