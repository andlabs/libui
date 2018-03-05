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
	grammarColors = nil;
	[spellingColor release];
	spellingColor = nil;
}

// unlike the other systems, Core Text rolls family, size, weight, italic, width, AND opentype features into the "font" attribute
// TODO opentype features are lost when using uiDrawFontDescriptor, so a handful of fonts in the font panel ("Titling" variants of some fonts and possibly others but those are the examples I know about) cannot be represented by uiDrawFontDescriptor; what *can* we do about this since this info is NOT part of the font on other platforms?
// TODO see if we could use NSAttributedString?
// TODO consider renaming this struct and the fep variable(s)
// TODO restructure all this so the important details at the top are below with the combined font attributes type?
// TODO in fact I should just write something to explain everything in this file...
struct foreachParams {
	CFMutableAttributedStringRef mas;
	NSMutableDictionary *combinedFontAttrs;		// keys are CFIndex in mas, values are uiprivCombinedFontAttr objects
	uiDrawFontDescriptor *defaultFont;
	NSMutableArray *backgroundBlocks;
};

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
- (void)setAttribute:(uiAttribute *)attr;
- (CTFontRef)toCTFontWithDefaultFont:(uiDrawFontDescriptor *)defaultFont;
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

- (void)setAttribute:(uiAttribute *)attr
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

- (CTFontRef)toCTFontWithDefaultFont:(uiDrawFontDescriptor *)defaultFont
{
	uiDrawFontDescriptor uidesc;
	CTFontDescriptorRef desc;
	CTFontRef font;

	uidesc = *defaultFont;
	if (self->attrs[cFamily] != NULL)
		// TODO const-correct uiDrawFontDescriptor or change this function below
		uidesc.Family = (char *) uiAttributeFamily(self->attrs[cFamily]);
	if (self->attrs[cSize] != NULL)
		uidesc.Size = uiAttributeSize(self->attrs[cSize]);
	if (self->attrs[cWeight] != NULL)
		uidesc.Weight = uiAttributeWeight(self->attrs[cWeight]);
	if (self->attrs[cItalic] != NULL)
		uidesc.Italic = uiAttributeItalic(self->attrs[cItalic]);
	if (self->attrs[cStretch] != NULL)
		uidesc.Stretch = uiAttributeStretch(self->attrs[cStretch]);
	desc = uiprivDrawFontDescriptorToCTFontDescriptor(&uidesc);
	if (self->attrs[cFeatures] != NULL)
		desc = uiprivCTFontDescriptorAppendFeatures(desc, uiAttributeFeatures(self->attrs[cFeatures]));
	font = CTFontCreateWithFontDescriptor(desc, uidesc.Size, NULL);
	CFRelease(desc);			// TODO correct?
	return font;
}

@end

static void adjustFontInRange(struct foreachParams *p, size_t start, size_t end, void (^adj)(uiprivCombinedFontAttr *cfa))
{
	size_t i;
	NSNumber *n;
	uiprivCombinedFontAttr *cfa;

	for (i = start; i < end; i++) {
		n = [NSNumber numberWithInteger:i];
		cfa = (uiprivCombinedFontAttr *) [p->combinedFontAttrs objectForKey:n];
		if (cfa == nil) {
			cfa = [[uiprivCombinedFontAttr alloc] initWithDefaultFont:p->defaultFont];
			[p->combinedFontAttrs setObject:cfa forKey:n];
		}
		adj(cfa);
	}
}

static backgroundBlock mkBackgroundBlock(size_t start, size_t end, double r, double g, double b, double a)
{
	return Block_copy(^(uiDrawContext *c, uiDrawTextLayout *layout, double x, double y) {
		uiDrawBrush brush;

		brush.Type = uiDrawBrushTypeSolid;
		brush.R = r;
		brush.G = g;
		brush.B = b;
		brush.A = a;
		drawTextBackground(c, x, y, layout, start, end, &brush, 0);
	});
}

static CGColorRef mkcolor(uiAttributeSpec *spec)
{
	CGColorSpaceRef colorspace;
	CGColorRef color;
	CGFloat components[4];

	// TODO we should probably just create this once and recycle it throughout program execution...
	colorspace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
	if (colorspace == NULL) {
		// TODO
	}
	components[0] = spec->R;
	components[1] = spec->G;
	components[2] = spec->B;
	components[3] = spec->A;
	color = CGColorCreate(colorspace, components);
	CFRelease(colorspace);
	return color;
}

static uiForEach processAttribute(const uiAttributedString *s, const uiAttribute *attr, size_t start, size_t end, void *data)
{
	struct foreachParams *p = (struct foreachParams *) data;
	CFRange range;
	CGColorRef color;
	size_t ostart, oend;
	backgroundBlock block;
	int32_t us;
	CFNumberRef num;

	ostart = start;
	oend = end;
	start = uiprivAttributedStringUTF8ToUTF16(s, start);
	end = uiprivAttributedStringUTF8ToUTF16(s, end);
	range.location = start;
	range.length = end - start;
	switch (uiAttributeGetType(attr)) {
$$TODO_CONTINUE_HERE
	case uiAttributeFamily:
		adjustFontInRange(p, start, end, ^(uiprivCombinedFontAttr *cfa) {
			cfa.family = spec->Family;
		});
		break;
	case uiAttributeSize:
		adjustFontInRange(p, start, end, ^(uiprivCombinedFontAttr *cfa) {
			cfa.size = spec->Double;
		});
		break;
	case uiAttributeWeight:
		adjustFontInRange(p, start, end, ^(uiprivCombinedFontAttr *cfa) {
			cfa.weight = (uiDrawTextWeight) (spec->Value);
		});
		break;
	case uiAttributeItalic:
		adjustFontInRange(p, start, end, ^(uiprivCombinedFontAttr *cfa) {
			cfa.italic = (uiDrawTextItalic) (spec->Value);
		});
		break;
	case uiAttributeStretch:
		adjustFontInRange(p, start, end, ^(uiprivCombinedFontAttr *cfa) {
			cfa.stretch = (uiDrawTextStretch) (spec->Value);
		});
		break;
	case uiAttributeColor:
		color = mkcolor(spec);
		CFAttributedStringSetAttribute(p->mas, range, kCTForegroundColorAttributeName, color);
		CFRelease(color);
		break;
	case uiAttributeBackground:
		block = mkBackgroundBlock(ostart, oend,
			spec->R, spec->G, spec->B, spec->A);
		[p->backgroundBlocks addObject:block];
		Block_release(block);
		break;
	case uiAttributeUnderline:
		switch (spec->Value) {
		case uiDrawUnderlineStyleNone:
			us = kCTUnderlineStyleNone;
			break;
		case uiDrawUnderlineStyleSingle:
			us = kCTUnderlineStyleSingle;
			break;
		case uiDrawUnderlineStyleDouble:
			us = kCTUnderlineStyleDouble;
			break;
		case uiDrawUnderlineStyleSuggestion:
			// TODO incorrect if a solid color
			us = kCTUnderlineStyleThick;
			break;
		}
		num = CFNumberCreate(NULL, kCFNumberSInt32Type, &us);
		CFAttributedStringSetAttribute(p->mas, range, kCTUnderlineStyleAttributeName, num);
		CFRelease(num);
		break;
	case uiAttributeUnderlineColor:
		switch (spec->Value) {
		case uiDrawUnderlineColorCustom:
			color = mkcolor(spec);
			break;
		case uiDrawUnderlineColorSpelling:
			color = [spellingColor CGColor];
			break;
		case uiDrawUnderlineColorGrammar:
			color = [grammarColor CGColor];
			break;
		case uiDrawUnderlineColorAuxiliary:
			color = [auxiliaryColor CGColor];
			break;
		}
		CFAttributedStringSetAttribute(p->mas, range, kCTUnderlineColorAttributeName, color);
		if (spec->Value == uiDrawUnderlineColorCustom)
			CFRelease(color);
		break;
	case uiAttributeFeatures:
		adjustFontInRange(p, start, end, ^(uiprivCombinedFontAttr *cfa) {
			cfa.features = spec->Features;
		});
		break;
	default:
		// TODO complain
		;
	}
	return uiForEachContinue;
}

static BOOL cfaIsEqual(uiprivCombinedFontAttr *a, uiprivCombinedFontAttr *b)
{
	if (a == nil && b == nil)
		return YES;
	if (a == nil || b == nil)
		return NO;
	return [a same:b];
}

static void applyAndFreeFontAttributes(struct foreachParams *p)
{
	CFIndex i, n;
	uiprivCombinedFontAttr *cfa, *cfab;
	CTFontRef defaultFont;
	CTFontRef font;
	CFRange range;

	// first get the default font as a CTFontRef
	cfa = [[uiprivCombinedFontAttr alloc] initWithDefaultFont:p->defaultFont];
	defaultFont = [cfa toCTFont];
	[cfa release];

	// now go through, fililng in the font attribute for successive ranges of identical combinedFontAttrs
	// we are best off treating series of identical fonts as single ranges ourselves for parity across platforms, even if OS X does something similar itself
	// this also avoids breaking apart surrogate pairs (though IIRC OS X doing the something similar itself might make this a non-issue here)
	cfa = nil;
	n = CFAttributedStringGetLength(p->mas);
	range.location = 0;
	for (i = 0; i < n; i++) {
		NSNumber *nn;

		// TODO use NSValue or some other method of NSNumber
		nn = [NSNumber numberWithInteger:i];
		cfab = (uiprivCombinedFontAttr *) [p->combinedFontAttrs objectForKey:nn];
		if (cfaIsEqual(cfa, cfab))
			continue;

		// the font has changed; write out the old one
		range.length = i - range.location;
		if (cfa == nil) {
			// TODO this isn't necessary because of default font shenanigans below
			font = defaultFont;
			CFRetain(font);
		} else
			font = [cfa toCTFont];
		CFAttributedStringSetAttribute(p->mas, range, kCTFontAttributeName, font);
		CFRelease(font);
		// and start this run
		cfa = cfab;
		range.location = i;
	}

	// and finally, write out the last range
	range.length = i - range.location;
	if (cfa == nil) {
		// TODO likewise
		font = defaultFont;
		CFRetain(font);
	} else
		// note that this handles the difference between NULL and empty uiOpenTypeFeatures properly as far as conversion to native data formats is concerned (NULL does not generate a features dictionary; empty just produces an empty one)
		// TODO but what about from the OS's perspective on all OSs?
		font = [cfa toCTFont];
	CFAttributedStringSetAttribute(p->mas, range, kCTFontAttributeName, font);
	CFRelease(font);

	CFRelease(defaultFont);
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

CFAttributedStringRef uiprivAttributedStringToCFAttributedString(uiDrawTextLayoutParams *p, NSArray **backgroundBlocks)
{
	CFStringRef cfstr;
	CFMutableDictionaryRef defaultAttrs;
	CTFontRef defaultCTFont;
	CTParagraphStyleRef ps;
	CFAttributedStringRef base;
	CFMutableAttributedStringRef mas;
	struct foreachParams fep;

	cfstr = CFStringCreateWithCharacters(NULL, attrstrUTF16(p->String), attrstrUTF16Len(p->String));
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
	fep.combinedFontAttrs = [NSMutableDictionary new];
	fep.defaultFont = p->DefaultFont;
	fep.backgroundBlocks = [NSMutableArray new];
	uiAttributedStringForEachAttribute(p->String, processAttribute, &fep);
	applyAndFreeFontAttributes(&fep);
	[fep.combinedFontAttrs release];
	CFAttributedStringEndEditing(mas);

	*backgroundBlocks = fep.backgroundBlocks;
	return mas;
}
