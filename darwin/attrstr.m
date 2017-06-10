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
// TODO in fact I should just write something to explain everything in this file...
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
- (CTFontRef)toCTFont;
@end

@implementation combinedFontAttr

- (id)initWithDefaultFont:(uiDrawFontDescriptor *)defaultFont
{
	self = [super init];
	if (self) {
		// TODO define behaviors if defaultFont->Family or any attribute Family is NULL, same with other invalid values
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
	// this also handles NULL cases
	if (!uiOpenTypeFeaturesEqual(self.features, b.features))
		return NO;
	return YES;
}

- (CTFontRef)toCTFont
{
	uiDrawFontDescriptor uidesc;
	CTFontDescriptorRef desc;
	CTFontRef font;

	// TODO const-correct uiDrawFontDescriptor or change this function below
	uidesc.Family = (char *) (self.family);
	uidesc.Size = self.size;
	uidesc.Weight = self.weight;
	uidesc.Italic = self.italic;
	uidesc.Stretch = self.stretch;
	desc = fontdescToCTFontDescriptor(&uidesc);
	if (self.features != NULL)
		desc = fontdescAppendFeatures(desc, self.features);
	font = CTFontCreateWithFontDescriptor(desc, self.size, NULL);
	CFRelease(desc);			// TODO correct?
	return font;
}

@end

// TODO merge this with adjustFontInRange() (and TODO figure out why they were separate in the first place; probably Windows?)
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
		cfa = (combinedFontAttr *) [p->combinedFontAttrs objectForKey:n];
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

static uiForEach processAttribute(uiAttributedString *s, uiAttributeSpec *spec, size_t start, size_t end, void *data)
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
	return uiForEachContinue;
}

static BOOL cfaIsEqual(combinedFontAttr *a, combinedFontAttr *b)
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
	combinedFontAttr *cfa, *cfab;
	CTFontRef defaultFont;
	CTFontRef font;
	CFRange range;

	// first get the default font as a CTFontRef
	cfa = [[combinedFontAttr alloc] initWithDefaultFont:p->defaultFont];
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
		cfab = (combinedFontAttr *) [p->combinedFontAttrs objectForKey:nn];
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

CFAttributedStringRef attrstrToCoreFoundation(uiDrawTextLayoutParams *p, NSArray **backgroundBlocks)
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
