// 6 september 2015
#import "uipriv_darwin.h"

// TODO for all relevant routines, make sure we are freeing memory correctly
// TODO make sure allocation failures throw exceptions?
struct uiDrawFontFamilies {
	CFArrayRef fonts;
};

uiDrawFontFamilies *uiDrawListFontFamilies(void)
{
	uiDrawFontFamilies *ff;

	ff = uiNew(uiDrawFontFamilies);
	// TODO is there a way to get an error reason?
	ff->fonts = CTFontManagerCopyAvailableFontFamilyNames();
	if (ff->fonts == NULL)
		complain("error getting available font names (no reason specified)");
	return ff;
}

uintmax_t uiDrawFontFamiliesNumFamilies(uiDrawFontFamilies *ff)
{
	return CFArrayGetCount(ff->fonts);
}

char *uiDrawFontFamiliesFamily(uiDrawFontFamilies *ff, uintmax_t n)
{
	CFStringRef familystr;
	char *family;

	familystr = (CFStringRef) CFArrayGetValueAtIndex(ff->fonts, n);
	// TODO create a uiDarwinCFStringToText()?
	family = uiDarwinNSStringToText((NSString *) familystr);
	// Get Rule means we do not free familystr
	return family;
}

void uiDrawFreeFontFamilies(uiDrawFontFamilies *ff)
{
	CFRelease(ff->fonts);
	uiFree(ff);
}

// these two are identical:
// - https://developer.apple.com/library/mac/documentation/TextFonts/Conceptual/CocoaTextArchitecture/TypoFeatures/TextSystemFeatures.html#//apple_ref/doc/uid/TP40009459-CH6-51627-BBCCHIFF text points are 72 per inch
// - https://developer.apple.com/library/mac/documentation/Cocoa/Conceptual/CocoaDrawingGuide/Transforms/Transforms.html#//apple_ref/doc/uid/TP40003290-CH204-SW5 user space points are 72 per inch
double uiDrawTextSizeToPoints(double textSize)
{
	return textSize;
}

double uiDrawPointsToTextSize(double points)
{
	return points;
}

struct uiDrawTextLayout {
	CFMutableAttributedStringRef mas;
	intmax_t *bytesToCharacters;
};

// TODO this is *really* iffy, but we need to know character offsets...
// TODO clean up the local variable names and improve documentation
static intmax_t *strToCFStrOffsetList(const char *str, CFMutableStringRef *cfstr)
{
	intmax_t *bytesToCharacters;
	intmax_t i, len;

	len = strlen(str);
	bytesToCharacters = (intmax_t *) uiAlloc(len * sizeof (intmax_t), "intmax_t[]");

	*cfstr = CFStringCreateMutable(NULL, 0);
	if (*cfstr == NULL)
		complain("error creating CFMutableStringRef for storing string in uiDrawNewTextLayout()");

	i = 0;
	while (i < len) {
		CFStringRef substr;
		intmax_t n;
		intmax_t j;
		intmax_t pos;

		// figure out how many characters to convert and convert them
		for (n = 1; (i + n - 1) < len; n++) {
			substr = CFStringCreateWithBytes(NULL, (const UInt8 *) (str + i), n, kCFStringEncodingUTF8, false);
			if (substr != NULL)		// found a full character
				break;
		}
		// if this test passes we either:
		// - reached the end of the string without a successful conversion (invalid string)
		// - ran into allocation issues
		if (substr == NULL)
			complain("something bad happened when trying to prepare string in uiDrawNewTextLayout()");

		// now save the character offsets for those bytes
		pos = CFStringGetLength(*cfstr);
		for (j = 0; j < n; j++)
			bytesToCharacters[j] = pos;

		// and add the characters that we converted
		CFStringAppend(*cfstr, substr);
		CFRelease(substr);			// TODO correct?

		// and go to the next
		i += n;
	}

	return bytesToCharacters;
}

static CFMutableDictionaryRef newAttrList(void)
{
	CFMutableDictionaryRef attr;

	attr = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (attr == NULL)
		complain("error creating attribute dictionary in newAttrList()()");
	return attr;
}

static void addFontFamilyAttr(CFMutableDictionaryRef attr, const char *family)
{
	CFStringRef cfstr;

	cfstr = CFStringCreateWithCString(NULL, family, kCFStringEncodingUTF8);
	if (cfstr == NULL)
		complain("error creating font family name CFStringRef in addFontFamilyAttr()");
	CFDictionaryAddValue(attr, kCTFontFamilyNameAttribute, cfstr);
	CFRelease(cfstr);			// dictionary holds its own reference
}

static void addFontSizeAttr(CFMutableDictionaryRef attr, double size)
{
	CFNumberRef n;

	n = CFNumberCreate(NULL, kCFNumberDoubleType, &size);
	CFDictionaryAddValue(attr, kCTFontSizeAttribute, n);
	CFRelease(n);
}

// see http://stackoverflow.com/questions/4810409/does-coretext-support-small-caps/4811371#4811371 and https://git.gnome.org/browse/pango/tree/pango/pangocoretext-fontmap.c
static void addFontSmallCapsAttr(CFMutableDictionaryRef attr)
{
	CFMutableArrayRef outerArray;
	CFMutableDictionaryRef innerDict;
	CFNumberRef numType, numSelector;
	int num;

	outerArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	if (outerArray == NULL)
		complain("error creating outer CFArray for adding small caps attributes in addFontSmallCapsAttr()");

	// TODO Apple's headers say these values are deprecated, but I'm not sure what they should be replaced with (or whether they should be deleted outright or used concurrently with their replacements); the other answers of the Stack Overflow question has hints though (and TODO inform Pango of this)
	num = kLetterCaseType;
	numType = CFNumberCreate(NULL, kCFNumberIntType, &num);
	num = kSmallCapsSelector;
	numSelector = CFNumberCreate(NULL, kCFNumberIntType, &num);
	innerDict = newAttrList();
	CFDictionaryAddValue(innerDict, kCTFontFeatureTypeIdentifierKey, numType);
	CFRelease(numType);
	CFDictionaryAddValue(innerDict, kCTFontFeatureSelectorIdentifierKey, numSelector);
	CFRelease(numSelector);
	CFArrayAppendValue(outerArray, innerDict);
	CFRelease(innerDict);		// and likewise for CFArray

	CFDictionaryAddValue(attr, kCTFontFeatureSettingsAttribute, outerArray);
	CFRelease(outerArray);
}

static void addFontGravityAttr(CFMutableDictionaryRef dict, uiDrawTextGravity gravity)
{
	// TODO: matrix setting? kCTFontOrientationAttribute? or is it a kCTVerticalFormsAttributeName of the CFAttributedString attributes and thus not part of the CTFontDescriptor?
}

// Named constants for these were NOT added until 10.11, and even then they were added as external symbols instead of macros, so we can't use them directly :(
// kode54 got these for me before I had access to El Capitan; thanks to him.
#define ourNSFontWeightUltraLight -0.800000
#define ourNSFontWeightThin -0.600000
#define ourNSFontWeightLight -0.400000
#define ourNSFontWeightRegular 0.000000
#define ourNSFontWeightMedium 0.230000
#define ourNSFontWeightSemibold 0.300000
#define ourNSFontWeightBold 0.400000
#define ourNSFontWeightHeavy 0.560000
#define ourNSFontWeightBlack 0.620000
static const CGFloat ctWeights[] = {
	// yeah these two have their names swapped; blame Pango
	// TODO note that these names do not necessarily line up with their OS names
	[uiDrawTextWeightThin] = ourNSFontWeightUltraLight,
	[uiDrawTextWeightUltraLight] = ourNSFontWeightThin,
	[uiDrawTextWeightLight] = ourNSFontWeightLight,
	// for this one let's go between Light and Regular
	// TODO figure out if we can rely on the order for these (and the one below)
	[uiDrawTextWeightBook] = ourNSFontWeightLight + ((ourNSFontWeightRegular - ourNSFontWeightLight) / 2),
	[uiDrawTextWeightNormal] = ourNSFontWeightRegular,
	[uiDrawTextWeightMedium] = ourNSFontWeightMedium,
	[uiDrawTextWeightSemiBold] = ourNSFontWeightSemibold,
	[uiDrawTextWeightBold] = ourNSFontWeightBold,
	// for this one let's go between Bold and Heavy
	[uiDrawTextWeightUtraBold] = ourNSFontWeightBold + ((ourNSFontWeightHeavy - ourNSFontWeightBold) / 2),
	[uiDrawTextWeightHeavy] = ourNSFontWeightHeavy,
	[uiDrawTextWeightUltraHeavy] = ourNSFontWeightBlack,
};

// Unfortunately there are still no named constants for these.
// Let's just use normalized widths.
// As far as I can tell (OS X only has condensed fonts, not expanded fonts; TODO), regardless of condensed or expanded, negative means condensed and positive means expanded.
// TODO verify this is correct
static const CGFloat ctStretches[] = {
	[uiDrawTextStretchUltraCondensed] = -1.0,
	[uiDrawTextStretchExtraCondensed] = -0.75,
	[uiDrawTextStretchCondensed] = -0.5,
	[uiDrawTextStretchSemiCondensed] = -0.25,
	[uiDrawTextStretchNormal] = 0.0,
	[uiDrawTextStretchSemiExpanded] = 0.25,
	[uiDrawTextStretchExpanded] = 0.5,
	[uiDrawTextStretchExtraExpanded] = 0.75,
	[uiDrawTextStretchUltraExpanded] = 1.0,
};

struct closeness {
	CFIndex index;
	CGFloat weight;
	CGFloat italic;
	CGFloat stretch;
	CGFloat distance;
};

// see below for details
// TODO document that font matching is closest match but the search method is OS defined
CTFontDescriptorRef matchTraits(CTFontDescriptorRef against, uiDrawTextWeight weight, uiDrawTextItalic italic, uiDrawTextStretch stretch)
{
	CGFloat targetWeight;
	CGFloat italicCloseness, obliqueCloseness, normalCloseness;
	CGFloat targetStretch;
	CFArrayRef matching;
	CFIndex i, n;
	struct closeness *closeness;
	CTFontDescriptorRef current;
	CTFontDescriptorRef out;

	targetWeight = ctWeights[weight];
	switch (italic) {
	case uiDrawTextItalicNormal:
		italicCloseness = 1;
		obliqueCloseness = 1;
		normalCloseness = 0;
		break;
	case uiDrawTextItalicOblique:
		italicCloseness = 0.5;
		obliqueCloseness = 0;
		normalCloseness = 1;
		break;
	case uiDrawTextItalicItalic:
		italicCloseness = 0;
		obliqueCloseness = 0.5;
		normalCloseness = 1;
		break;
	}
	targetStretch = ctStretches[stretch];

	matching = CTFontDescriptorCreateMatchingFontDescriptors(against, NULL);
	if (matching == NULL)
		// no matches; give the original back and hope for the best
		return against;
	n = CFArrayGetCount(matching);
	if (n == 0) {
		// likewise
		CFRelease(matching);
		return against;
	}

	closeness = (struct closeness *) uiAlloc(n * sizeof (struct closeness), "struct closeness[]");
	for (i = 0; i < n; i++) {
		CFDictionaryRef traits;
		CFNumberRef cfnum;
		CTFontSymbolicTraits symbolic;

		closeness[i].index = i;

		current = CFArrayGetValueAtIndex(matching, i);
		traits = CTFontDescriptorCopyAttribute(current, kCTFontTraitsAttribute);
		if (traits == NULL) {
			// couldn't get traits; be safe by ranking it lowest
			// TODO figure out what the longest possible distances are
			closeness[i].weight = 3;
			closeness[i].italic = 2;
			closeness[i].stretch = 3;
			continue;
		}

		symbolic = 0;			// assume no symbolic traits if none are listed
		cfnum = CFDictionaryGetValue(traits, kCTFontSymbolicTrait);
		if (cfnum != NULL) {
			SInt32 s;

			if (CFNumberGetValue(cfnum, kCFNumberSInt32Type, &s) == false)
				complain("error getting symbolic traits in matchTraits()");
			symbolic = (CTFontSymbolicTraits) s;
			// Get rule; do not release cfnum
		}

		// now try weight
		cfnum = CFDictionaryGetValue(traits, kCTFontWeightTrait);
		if (cfnum != NULL) {
			CGFloat val;

			// TODO instead of complaining for this and width, should we just fall through to the default?
			if (CFNumberGetValue(cfnum, kCFNumberCGFloatType, &val) == false)
				complain("error getting weight value in matchTraits()");
			closeness[i].weight = val - targetWeight;
		} else
			// okay there's no weight key; let's try the literal meaning of the symbolic constant
			// TODO is the weight key guaranteed?
			if ((symbolic & kCTFontBoldTrait) != 0)
				closeness[i].weight = ourNSFontWeightBold - targetWeight;
			else
				closeness[i].weight = ourNSFontWeightRegular - targetWeight;

		// italics is a bit harder because Core Text doesn't expose a concept of obliqueness
		// Pango just does a g_strrstr() (backwards case-sensitive search) for "Oblique" in the font's style name (see https://git.gnome.org/browse/pango/tree/pango/pangocoretext-fontmap.c); let's do that too I guess
		if ((symbolic & kCTFontItalicTrait) != 0)
			closeness[i].italic = italicCloseness;
		else {
			CFStringRef styleName;
			BOOL isOblique;

			isOblique = NO;		// default value
			styleName = CTFontDescriptorCopyAttribute(current, kCTFontStyleNameAttribute);
			if (styleName != NULL) {
				CFRange range;

				// note the use of the toll-free bridge for the string literal, since CFSTR() *can* return NULL
				range = CFStringFind(styleName, (CFStringRef) @"Oblique", kCFCompareBackwards);
				if (range.location != kCFNotFound)
					isOblique = YES;
				CFRelease(styleName);
			}
			if (isOblique)
				closeness[i].italic = obliqueCloseness;
			else
				closeness[i].italic = normalCloseness;
		}

		// now try width
		// TODO this does not seem to be enough for Skia's extended variants; the width trait is 0 but the Expanded flag is on
		// TODO verify the rest of this matrix
		cfnum = CFDictionaryGetValue(traits, kCTFontWidthTrait);
		if (cfnum != NULL) {
			CGFloat val;

			if (CFNumberGetValue(cfnum, kCFNumberCGFloatType, &val) == false)
				complain("error getting width value in matchTraits()");
			closeness[i].stretch = val - targetStretch;
		} else
			// okay there's no width key; let's try the literal meaning of the symbolic constant
			// TODO is the width key guaranteed?
			if ((symbolic & kCTFontExpandedTrait) != 0)
				closeness[i].stretch = 1.0 - targetStretch;
			else if ((symbolic & kCTFontCondensedTrait) != 0)
				closeness[i].stretch = -1.0 - targetStretch;
			else
				closeness[i].stretch = 0.0 - targetStretch;

		CFRelease(traits);
	}

	// now figure out the 3-space difference between the three and sort by that
	for (i = 0; i < n; i++) {
		CGFloat weight, italic, stretch;

		weight = closeness[i].weight;
		weight *= weight;
		italic = closeness[i].italic;
		italic *= italic;
		stretch = closeness[i].stretch;
		stretch *= stretch;
		closeness[i].distance = sqrt(weight + italic + stretch);
	}
	qsort_b(closeness, n, sizeof (struct closeness), ^(const void *aa, const void *bb) {
		const struct closeness *a = (const struct closeness *) aa;
		const struct closeness *b = (const struct closeness *) bb;

		// via http://www.gnu.org/software/libc/manual/html_node/Comparison-Functions.html#Comparison-Functions
		// TODO is this really the best way? isn't it the same as if (*a < *b) return -1; if (*a > *b) return 1; return 0; ?
		return (a->distance > b->distance) - (a->distance < b->distance);
	});
	// and the first element of the sorted array is what we want
	out = CFArrayGetValueAtIndex(matching, closeness[0].index);
	CFRetain(out);			// get rule

	// release everything
	uiFree(closeness);
	CFRelease(matching);
	// and release the original descriptor since we no longer need it
	CFRelease(against);

	return out;
}

uiDrawTextLayout *uiDrawNewTextLayout(const char *str, const uiDrawInitialTextStyle *initialStyle)
{
	uiDrawTextLayout *layout;
	CFMutableStringRef cfstr;
	CFMutableDictionaryRef attr;
	CTFontDescriptorRef desc;
	CTFontRef font;
	CFAttributedStringRef immutable;

	layout = uiNew(uiDrawTextLayout);

	layout->bytesToCharacters = strToCFStrOffsetList(str, &cfstr);

	attr = newAttrList();
	addFontFamilyAttr(attr, initialStyle->Family);
	addFontSizeAttr(attr, initialStyle->Size);
	if (initialStyle->SmallCaps)
		addFontSmallCapsAttr(attr);
	addFontGravityAttr(attr, initialStyle->Gravity);

	desc = CTFontDescriptorCreateWithAttributes(attr);
	// TODO release attr?

	// unfortunately OS X requires an EXACT MATCH for the traits, otherwise it will *drop all the traits*
	// we want a nearest match, so we have to do it ourselves
	// TODO this does not preserve small caps
	desc = matchTraits(desc, initialStyle->Weight, initialStyle->Italic, initialStyle->Stretch);

	// specify the initial size again just to be safe
	font = CTFontCreateWithFontDescriptor(desc, initialStyle->Size, NULL);
	// TODO release desc?

	attr = newAttrList();
	CFDictionaryAddValue(attr, kCTFontAttributeName, font);
	CFRelease(font);

	immutable = CFAttributedStringCreate(NULL, cfstr, attr);
	if (immutable == NULL)
		complain("error creating immutable attributed string in uiDrawNewTextLayout()");
	CFRelease(cfstr);
	CFRelease(attr);

	layout->mas = CFAttributedStringCreateMutableCopy(NULL, 0, immutable);
	if (layout->mas == NULL)
		complain("error creating attributed string in uiDrawNewTextLayout()");
	CFRelease(immutable);

	return layout;
}

void uiDrawFreeTextLayout(uiDrawTextLayout *layout)
{
	CFRelease(layout->mas);
	uiFree(layout->bytesToCharacters);
	uiFree(layout);
}

// Core Text doesn't draw onto a flipped view correctly; we have to do this
// see the iOS bits of the first example at https://developer.apple.com/library/mac/documentation/StringsTextFonts/Conceptual/CoreText_Programming/LayoutOperations/LayoutOperations.html#//apple_ref/doc/uid/TP40005533-CH12-SW1 (iOS is naturally flipped)
// TODO how is this affected by the CTM?
static void prepareContextForText(CGContextRef c, CGFloat cheight, double *y)
{
	CGContextSaveGState(c);
	CGContextTranslateCTM(c, 0, cheight);
	CGContextScaleCTM(c, 1.0, -1.0);
	CGContextSetTextMatrix(c, CGAffineTransformIdentity);

	// wait, that's not enough; we need to offset y values to account for our new flipping
	*y = cheight - *y;
}

// TODO how does this behave with multiple lines? on other platforms? is there a generic way to draw this unbounded?
void doDrawText(CGContextRef c, CGFloat cheight, double x, double y, uiDrawTextLayout *layout)
{
	CTLineRef line;
	CGRect bounds;

	prepareContextForText(c, cheight, &y);

	line = CTLineCreateWithAttributedString(layout->mas);
	if (line == NULL)
		complain("error creating CTLine object in uiDrawText()");

	// oh, and (x, y) is the bottom-left corner; we need the top-left
	// remember that we're flipped, so we subtract
	bounds = CTLineGetImageBounds(line, c);
	// though CTLineGetImageBounds() returns CGRectNull on error, it also returns CGRectNull on an empty string, so we can't reasonably check for error
	y -= bounds.size.height;
	CGContextSetTextPosition(c, x, y);

	// and now we can FINALLY draw the line
	CTLineDraw(line, c);
	CFRelease(line);

	CGContextRestoreGState(c);
}
