// 12 february 2017
#import "uipriv_darwin.h"

// LONGTERM FUTURE for typographic features, on 10.10 we can use OpenType tags directly!

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

void initUnderlineColors(void)
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

void uninitUnderlineColors(void)
{
	[auxiliaryColor release];
	[grammarColor release];
	[spellingColor release];
}

// unlike the other systems, Core Text rolls family, size, weight, italic, width, AND opentype features into the "font" attribute
// TODO opentype features and AAT fvar table info is lost, so a handful of fonts in the font panel ("Titling" variants of some fonts and Skia and possibly others but those are the examples I know about) cannot be represented by uiDrawFontDescriptor; what *can* we do about this since this info is NOT part of the font on other platforms?
// TODO see if we could use NSAttributedString?
// TODO consider renaming this struct and the fep variable(s)
// TODO restructure all this so the important details at the top are below with the combined font attributes type?
struct foreachParams {
	CFMutableAttributedStringRef mas;
	NSMutableDictionary *combinedFontAttrs;		// keys are CFIndex in mas, values are combinedFontAttr objects
	uiDrawFontDescriptor *defaultFont;
	NSMutableArray *backgroundBlocks;
};

@interface combinedFontAttr : NSObject
@property const char *family;
@property double size;
@property uiDrawTextWeight weight;
@property uiDrawTextItalic italic;
@property uiDrawTextStretch stretch;
@property const uiOpenTypeFeatures *features;
- (id)initWithDefaultFont:(uiDrawFontDescriptor *)defaultFont;
- (BOOL)same:(combinedFontAttr *)b;
@end

@implementation combinedFontAttr

- (id)initWithDefaultFont:(uiDrawFontDescriptor *)defaultFont
{
	self = [super init];
	if (self) {
		self.family = defaultFont->Family;
		self.size = defaultFont->Size;
		self.weight = defaultFont->Weight;
		self.italic = defaultFont->Italic;
		self.stretch = defaultFont->Stretch;
		self.features = NULL;
	}
	return self;
}

// TODO deduplicate this with common/attrlist.c
- (BOOL)same:(combinedFontAttr *)b
{
	// TODO should this be case-insensitive?
	if (strcmp(self.family, b.family) != 0)
		return NO;
	// TODO use a closest match?
	if (self.size != b.size)
		return NO;
	if (self.weight != b.weight)
		return NO;
	if (self.italic != b.italic)
		return NO;
	if (self.stretch != b.stretch)
		return NO;
	// TODO make this part of uiOpenTypeFeaturesEqual() on all platforms
	if (self.features == NULL && b.features == NULL)
		return YES;
	if (self.features != NULL && b.features == NULL)
		return NO;
	if (self.features == NULL && b.features != NULL)
		return NO;
	if (!uiOpenTypeFeaturesEqual(self.features, b.features))
		return NO;
	return YES;
}

@end

static void ensureFontInRange(struct foreachParams *p, size_t start, size_t end)
{
	size_t i;
	NSNumber *n;
	combinedFontAttr *new;

	for (i = start; i < end; i++) {
		n = [NSNumber numberWithInteger:i];
		if ([p->combinedFontAttrs objectForKey:n] != nil)
			continue;
		new = [[combinedFontAttr alloc] initWithDefaultFont:p->defaultFont];
		[p->combinedFontAttrs setObject:new forKey:n];
	}
}

static void adjustFontInRange(struct foreachParams *p, size_t start, size_t end, void (^adj)(combinedFontAttr *cfa))
{
	size_t i;
	NSNumber *n;
	combinedFontAttr *cfa;

	for (i = start; i < end; i++) {
		n = [NSNumber numberWithInteger:i];
		v = (combinedFontAttr *) [p->combinedFontAttrs objectForKey:n];
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

struct aatParam {
	struct foreachParams *p;
	size_t start;
	size_t end;
};

static void doAAT(uint16_t type, uint16_t selector, void *data)
{
#if 0 /* TODO */
	struct aatParam *p = (struct aatParam *) data;

	ensureFontInRange(p->p, p->start, p->end);
	adjustFontInRange(p->p, p->start, p->end, ^(struct fontParams *fp) {
		fp->featureTypes[fp->nFeatures] = type;
		fp->featureSelectors[fp->nFeatures] = selector;
		fp->nFeatures++;
		if (fp->nFeatures == maxFeatures) {
			// TODO
			// TODO move this check to the top like in the drawtext example? and all the other instances of this?
		}
	});
#endif
}

static int processAttribute(uiAttributedString *s, uiAttributeSpec *spec, size_t start, size_t end, void *data)
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
	start = attrstrUTF8ToUTF16(s, start);
	end = attrstrUTF8ToUTF16(s, end);
	range.location = start;
	range.length = end - start;
	switch (spec->Type) {
	case uiAttributeFamily:
		ensureFontInRange(p, start, end);
		adjustFontInRange(p, start, end, ^(combinedFontAttr *cfa) {
			cfa.family = spec->Family;
		});
		break;
	case uiAttributeSize:
		ensureFontInRange(p, start, end);
		adjustFontInRange(p, start, end, ^(combinedFontAttr *cfa) {
			cfa.size = spec->Double;
		});
		break;
	case uiAttributeWeight:
		ensureFontInRange(p, start, end);
		adjustFontInRange(p, start, end, ^(combinedFontAttr *cfa) {
			cfa.weight = (uiDrawTextWeight) (spec->Value);
		});
		break;
	case uiAttributeItalic:
		ensureFontInRange(p, start, end);
		adjustFontInRange(p, start, end, ^(combinedFontAttr *cfa) {
			cfa.italic = (uiDrawTextItalic) (spec->Value);
		});
		break;
	case uiAttributeStretch:
		ensureFontInRange(p, start, end);
		adjustFontInRange(p, start, end, ^(combinedFontAttr *cfa) {
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
		ensureFontInRange(p, start, end);
		adjustFontInRange(p, start, end, ^(combinedFontAttr *cfa) {
			cfa.features = spec->Features;
		});
		break;
	default:
		// TODO complain
		;
	}
	return 0;
}

static CTFontRef fontdescToCTFont(struct fontParams *fp)
{
	CTFontDescriptorRef desc;
	CTFontRef font;

	desc = fontdescToCTFontDescriptor(&(fp->desc));
	desc = fontdescAppendFeatures(desc, fp->featureTypes, fp->featureSelectors, fp->nFeatures);
	font = CTFontCreateWithFontDescriptor(desc, fp->desc.Size, NULL);
	CFRelease(desc);			// TODO correct?
	return font;
}

static void applyAndFreeFontAttributes(struct foreachParams *p)
{
	[p->converted enumerateKeysAndObjectsUsingBlock:^(NSNumber *key, NSValue *val, BOOL *stop) {
		struct fontParams *fp;
		CTFontRef font;
		CFRange range;

		fp = (struct fontParams *) [val pointerValue];
		font = fontdescToCTFont(fp);
		range.location = [key integerValue];
		// TODO this is wrong for surrogate pairs
		range.length = 1;
		CFAttributedStringSetAttribute(p->mas, range, kCTFontAttributeName, font);
		CFRelease(font);
		uiFree(fp);
	}];
}

static const CTTextAlignment ctaligns[] = {
	[uiDrawTextLayoutAlignLeft] = kCTTextAlignmentLeft,
	[uiDrawTextLayoutAlignCenter] = kCTTextAlignmentCenter,
	[uiDrawTextLayoutAlignRight] = kCTTextAlignmentRight,
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

CFAttributedStringRef attrstrToCoreFoundation(uiDrawTextLayoutParams *p, NSArray **backgroundBlocks)
{
	CFStringRef cfstr;
	CFMutableDictionaryRef defaultAttrs;
	CTFontRef defaultCTFont;
	CTParagraphStyleRef ps;
	CFAttributedStringRef base;
	CFMutableAttributedStringRef mas;
	struct foreachParams fep;
	struct fontParams ffp;

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
	memset(&ffp, 0, sizeof (struct fontParams));
	ffp.desc = *(p->DefaultFont);
	defaultCTFont = fontdescToCTFont(&ffp);
	CFDictionaryAddValue(defaultAttrs, kCTFontAttributeName, defaultCTFont);
	CFRelease(defaultCTFont);
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
	fep.converted = [NSMutableDictionary new];
	fep.defaultFont = p->DefaultFont;
	fep.backgroundBlocks = [NSMutableArray new];
	uiAttributedStringForEachAttribute(p->String, processAttribute, &fep);
	applyAndFreeFontAttributes(&fep);
	[fep.converted release];
	CFAttributedStringEndEditing(mas);

	*backgroundBlocks = fep.backgroundBlocks;
	return mas;
}
