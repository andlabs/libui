// 2 january 2017
#import "uipriv_darwin.h"

struct uiDrawTextLayout {
	CFAttributedStringRef attrstr;
	double width;
};

// since uiDrawTextWeight effectively corresponds to OS/2 weights (which roughly correspond to GDI, Pango, and DirectWrite weights, and to a lesser(? TODO) degree, CSS weights), let's just do what Core Text does with OS/2 weights
// TODO this will not be correct for system fonts, which use cached values that have no relation to the OS/2 weights; we need to figure out how to reconcile these
// for more information, see https://bugzilla.gnome.org/show_bug.cgi?id=766148 and TODO_put_blog_post_here_once_I_write_it (TODO keep this line when resolving the above TODO)
static const double weightsToCTWeights[] = {
	-1.0,		// 0..99
	-0.7,		// 100..199
	-0.5,		// 200..299
	-0.23,	// 300..399
	0.0,		// 400..499
	0.2,		// 500..599
	0.3,		// 600..699
	0.4,		// 700..799
	0.6,		// 800..899
	0.8,		// 900..999
	1.0,		// 1000
};

static double weightToCTWeight(uiDrawTextWeight weight)
{
	int weightClass;
	double ctclass;
	double rest, weightFloor, nextFloor;

	if (weight <= 0)
		return -1.0;
	if (weight >= 1000)
		return 1.0;

	weightClass = weight / 100;
	rest = (double) weight;
	weightFloor = (double) (weightClass * 100);
	nextFloor = (double) ((weightClass + 1) * 100);
	rest = (rest - weightFloor) / (nextFloor - weightFloor);

	ctclass = weightsToCTWeights[weightClass];
	return fma(rest,
		weightsToCTWeights[weightClass + 1] - ctclass,
		ctclass);
}

// TODO put italics here

// based on what Core Text says about actual fonts (system fonts, system fonts in another folder to avoid using cached values, Adobe Font Folio 11, Google Fonts archive, fonts in Windows 7/8.1/10)
static const double stretchesToCTWidths[] = {
	[uiDrawTextStretchUltraCondensed] = -0.400000,
	[uiDrawTextStretchExtraCondensed] = -0.300000,
	[uiDrawTextStretchCondensed] = -0.200000,
	[uiDrawTextStretchSemiCondensed] = -0.100000,
	[uiDrawTextStretchNormal] = 0.000000,
	[uiDrawTextStretchSemiExpanded] = 0.100000,
	[uiDrawTextStretchExpanded] = 0.200000,
	[uiDrawTextStretchExtraExpanded] = 0.300000,
	// this one isn't present in any of the fonts I tested, but it follows naturally from the pattern of the rest, so... (TODO verify by checking the font files directly)
	[uiDrawTextStretchUltraExpanded] = 0.400000,
};

static CFDictionaryRef fontdescToTraits(uiDrawFontDescriptor *fd)
{
	CFMutableDictionaryRef traits;
	CFNumberRef num;
	double x;

	traits = CFDictionaryCreateMutable(NULL, 2,
		// TODO are these correct?
		&kCFCopyStringDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	if (traits == NULL) {
		// TODO
	}

	x = weightToCTWeight(fd->Weight);
	num = CFNumberCreate(NULL, kCFNumberDoubleType, &x);
	CFDictionaryAddValue(traits, kCTFontWeightTrait, num);
	CFRelease(num);

	// TODO italics

	x = stretchesToCTWidths[fd->Stretch];
	num = CFNumberCreate(NULL, kCFNumberDoubleType, &x);
	CFDictionaryAddValue(traits, kCTFontWidthTrait, num);
	CFRelease(num);

	return traits;
}

static CTFontRef fontdescToCTFont(uiDrawFontDescriptor *fd)
{
	CFMutableDictionaryRef attrs;
	CFStringRef cffamily;
	CFDictionaryRef traits;
	CTFontDescriptorRef desc;
	CTFontRef font;

	attrs = CFDictionaryCreateMutable(NULL, 2,
		// TODO are these correct?
		&kCFCopyStringDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	if (attrs == NULL) {
		// TODO
	}
	cffamily = CFStringCreateWithCString(NULL, fd.Family, kCFStringEncodingUTF8);
	if (cffamily == NULL) {
		// TODO
	}
	CFDictionaryAddValue(attrs, kCTFontFamilyNameAttribute, cffamily);
	CFRelease(cffamily);
	traits = fontdescToTraits(fd);
	CFDictionaryAddValue(attrs, kCTFontTraitsAttribute, traits);
	CFRelease(traits);

	desc = CTFontDescriptorCreateWithAttributes(attrs);
	CFRelease(attrs);			// TODO correct?
	// This function DOES return a font with the closest traits that are available, so we don't have to do any manual matching.
	// TODO figure out why we had to for other things...
	font = CTFontCreateWithFontDescriptor(desc, fd.Size, NULL);
	CFRelease(desc);			// TODO correct?
	return font;
}

static CFAttributedStringRef attrstrToCoreFoundation(uiAttributedString *s, uiDrawFontDescriptor *defaultFont)
{
	CFStringRef cfstr;
	CFMutableDictionaryRef defaultAttrs;
	CTFontRef defaultCTFont;
	CFAttributedStringRef base;
	CFMutableAttributedStringRef mas;

	cfstr = CFStringCreateWithCharacters(NULL, attrstrUTF16(s), attrstrUTF16Len(s));
	if (cfstr == NULL) {
		// TODO
	}
	defaultAttrs = CFDictionaryCreateMutable(NULL, 1,
		&kCFCopyStringDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	if (defaultAttrs == NULL) {
		// TODO
	}
	defaultCTFont = fontdescToCTFont(defaultFont);
	CFDictionaryAddValue(defaultAttrs, kCTFontAttributeName, defaultCTFont);
	CFRelease(defaultCTFont);

	base = CFAttributedStringCreate(NULL, cfstr, defaultAttrs);
	if (base == NULL) {
		// TODO
	}
	CFRelease(cfstr);
	CFRelease(defaultAttrs);
	mas = CFAttributedStringCreateMutableCopy(NULL, 0, base);
	CFRelease(base);

	CFAttributedStringBeginEditing(mas);
	// TODO copy in the attributes
	CFAttributedStringEndEditing(mas);

	return mas;
}

uiDrawTextLayout *uiDrawNewTextLayout(uiAttributedString *s, uiDrawFontDescriptor *defaultFont, double width)
{
}

void uiDrawFreeTextLayout(uiDrawTextLayout *tl)
{
	CFRelease(tl->attrstr);
	uiFree(tl);
}

void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y)
{
}

void uiDrawTextLayoutExtents(uiDrawTextLayout *tl, double *width, double *height)
{
}

int uiDrawTextLayoutNumLines(uiDrawTextLayout *tl)
{
}

void uiDrawTextLayoutLineByteRange(uiDrawTextLayout *tl, int line, size_t *start, size_t *end)
{
}

void uiDrawTextLayoutLineGetMetrics(uiDrawTextLayout *tl, int line, uiDrawTextLayoutLineMetrics *m)
{
}

void uiDrawTextLayoutByteIndexToGraphemeRect(uiDrawTextLayout *tl, size_t pos, int *line, double *x, double *y, double *width, double *height)
{
}

uiDrawTextLayoutHitTestResult uiDrawTextLayoutHitTest(uiDrawTextLayout *tl, double x, double y, size_t *byteIndex, int *line)
{
}

void uiDrawTextLayoutByteRangeToRectangle(uiDrawTextLayout *tl, size_t start, size_t end, uiDrawTextLayoutByteRangeRectangle *r)
{
}
