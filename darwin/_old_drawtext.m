// 6 september 2015
#import "uipriv_darwin.h"

// TODO double-check that we are properly handling allocation failures (or just toll free bridge from cocoa)
struct uiDrawFontFamilies {
	CFArrayRef fonts;
};

uiDrawFontFamilies *uiDrawListFontFamilies(void)
{
	uiDrawFontFamilies *ff;

	ff = uiNew(uiDrawFontFamilies);
	ff->fonts = CTFontManagerCopyAvailableFontFamilyNames();
	if (ff->fonts == NULL)
		implbug("error getting available font names (no reason specified) (TODO)");
	return ff;
}

int uiDrawFontFamiliesNumFamilies(uiDrawFontFamilies *ff)
{
	return CFArrayGetCount(ff->fonts);
}

char *uiDrawFontFamiliesFamily(uiDrawFontFamilies *ff, int n)
{
	CFStringRef familystr;
	char *family;

	familystr = (CFStringRef) CFArrayGetValueAtIndex(ff->fonts, n);
	// toll-free bridge
	family = uiDarwinNSStringToText((NSString *) familystr);
	// Get Rule means we do not free familystr
	return family;
}

void uiDrawFreeFontFamilies(uiDrawFontFamilies *ff)
{
	CFRelease(ff->fonts);
	uiFree(ff);
}

struct uiDrawTextFont {
	CTFontRef f;
};

uiDrawTextFont *mkTextFont(CTFontRef f, BOOL retain)
{
	uiDrawTextFont *font;

	font = uiNew(uiDrawTextFont);
	font->f = f;
	if (retain)
		CFRetain(font->f);
	return font;
}

uiDrawTextFont *mkTextFontFromNSFont(NSFont *f)
{
	// toll-free bridging; we do retain, though
	return mkTextFont((CTFontRef) f, YES);
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

#if 0
TODO
// See http://stackoverflow.com/questions/4810409/does-coretext-support-small-caps/4811371#4811371 and https://git.gnome.org/browse/pango/tree/pango/pangocoretext-fontmap.c for what these do
// And fortunately, unlike the traits (see below), unmatched features are simply ignored without affecting the other features :D
static void addFontSmallCapsAttr(CFMutableDictionaryRef attr)
{
	CFMutableArrayRef outerArray;
	CFMutableDictionaryRef innerDict;
	CFNumberRef numType, numSelector;
	int num;

	outerArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	if (outerArray == NULL)
		complain("error creating outer CFArray for adding small caps attributes in addFontSmallCapsAttr()");

	// Apple's headers say these are deprecated, but a few fonts still rely on them
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

	// these are the non-deprecated versions of the above; some fonts have these instead
	num = kLowerCaseType;
	numType = CFNumberCreate(NULL, kCFNumberIntType, &num);
	num = kLowerCaseSmallCapsSelector;
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
#endif

#if 0
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
#endif

// Now remember what I said earlier about having to add the small caps traits after calling the above? This gets a dictionary back so we can do so.
CFMutableDictionaryRef extractAttributes(CTFontDescriptorRef desc)
{
	CFDictionaryRef dict;
	CFMutableDictionaryRef mdict;

	dict = CTFontDescriptorCopyAttributes(desc);
	// this might not be mutable, so make a mutable copy
	mdict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
	CFRelease(dict);
	return mdict;
}

uiDrawTextFont *uiDrawLoadClosestFont(const uiDrawTextFontDescriptor *desc)
{
	CTFontRef f;
	CFMutableDictionaryRef attr;
	CTFontDescriptorRef cfdesc;

	attr = newAttrList();
	addFontFamilyAttr(attr, desc->Family);
	addFontSizeAttr(attr, desc->Size);

	// now we have to do the traits matching, so create a descriptor, match the traits, and then get the attributes back
	cfdesc = CTFontDescriptorCreateWithAttributes(attr);
	// TODO release attr?
	cfdesc = matchTraits(cfdesc, desc->Weight, desc->Italic, desc->Stretch);

	// specify the initial size again just to be safe
	f = CTFontCreateWithFontDescriptor(cfdesc, desc->Size, NULL);
	// TODO release cfdesc?

	return mkTextFont(f, NO);		// we hold the initial reference; no need to retain again
}

void uiDrawFreeTextFont(uiDrawTextFont *font)
{
	CFRelease(font->f);
	uiFree(font);
}

uintptr_t uiDrawTextFontHandle(uiDrawTextFont *font)
{
	return (uintptr_t) (font->f);
}

void uiDrawTextFontDescribe(uiDrawTextFont *font, uiDrawTextFontDescriptor *desc)
{
	// TODO
}

// text sizes and user space points are identical:
// - https://developer.apple.com/library/mac/documentation/TextFonts/Conceptual/CocoaTextArchitecture/TypoFeatures/TextSystemFeatures.html#//apple_ref/doc/uid/TP40009459-CH6-51627-BBCCHIFF text points are 72 per inch
// - https://developer.apple.com/library/mac/documentation/Cocoa/Conceptual/CocoaDrawingGuide/Transforms/Transforms.html#//apple_ref/doc/uid/TP40003290-CH204-SW5 user space points are 72 per inch
void uiDrawTextFontGetMetrics(uiDrawTextFont *font, uiDrawTextFontMetrics *metrics)
{
	metrics->Ascent = CTFontGetAscent(font->f);
	metrics->Descent = CTFontGetDescent(font->f);
	metrics->Leading = CTFontGetLeading(font->f);
	metrics->UnderlinePos = CTFontGetUnderlinePosition(font->f);
	metrics->UnderlineThickness = CTFontGetUnderlineThickness(font->f);
}

// LONGTERM allow line separation and leading to be factored into a wrapping text layout

// TODO reconcile differences in character wrapping on platforms
void uiDrawTextLayoutExtents(uiDrawTextLayout *layout, double *width, double *height)
{
	struct framesetter fs;

	mkFramesetter(layout, &fs);
	*width = fs.extents.width;
	*height = fs.extents.height;
	freeFramesetter(&fs);
}

// LONGTERM provide an equivalent to CTLineGetTypographicBounds() on uiDrawTextLayout?

// LONGTERM keep this for later features and documentation purposes
#if 0

	// LONGTERM provide a way to get the image bounds as a separate function later
	bounds = CTLineGetImageBounds(line, c);
	// though CTLineGetImageBounds() returns CGRectNull on error, it also returns CGRectNull on an empty string, so we can't reasonably check for error

	// CGContextSetTextPosition() positions at the baseline in the case of CTLineDraw(); we need the top-left corner instead
	CTLineGetTypographicBounds(line, &yoff, NULL, NULL);
	// remember that we're flipped, so we subtract
	y -= yoff;
	CGContextSetTextPosition(c, x, y);
#endif

#if 0
void uiDrawTextLayoutSetColor(uiDrawTextLayout *layout, int startChar, int endChar, double r, double g, double b, double a)
{
	CGColorSpaceRef colorspace;
	CGFloat components[4];
	CGColorRef color;

	// for consistency with windows, use sRGB
	colorspace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
	components[0] = r;
	components[1] = g;
	components[2] = b;
	components[3] = a;
	color = CGColorCreate(colorspace, components);
	CGColorSpaceRelease(colorspace);

	CFAttributedStringSetAttribute(layout->mas,
		rangeToCFRange(),
		kCTForegroundColorAttributeName,
		color);
	CGColorRelease(color);		// TODO safe?
}
#endif
