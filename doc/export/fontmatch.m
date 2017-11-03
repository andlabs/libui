// 3 january 2017
#import "uipriv_darwin.h"
#import "fontstyle.h"

// Core Text exposes font style info in two forms:
// - Fonts with a QuickDraw GX font variation (fvar) table, a feature
// 	adopted by OpenType, expose variations directly.
// - All other fonts have Core Text normalize the font style info
// 	into a traits dictionary.
// Of course this setup doesn't come without its hiccups and
// glitches. In particular, not only are the exact rules not well
// defined, but also font matching doesn't work as we want it to
// (exactly how varies based on the way the style info is exposed).
// So we'll have to implement style matching ourselves.
// We can use Core Text's matching to get a complete list of
// *possible* matches, and then we can filter out the ones we don't
// want ourselves.
//
// To make things easier for us, we'll match by converting Core
// Text's values back into libui values. This allows us to also use the
// normalization code for filling in uiDrawFontDescriptors from
// Core Text fonts and font descriptors.
//
// Style matching needs to be done early in the font loading process;
// in particular, we have to do this before adding any features,
// because the descriptors returned by Core Text's own font
// matching won't have any.

// TODO explicitly mark these as undocumented
extern const CFStringRef kCTFontPreferredSubFamilyNameKey;
extern const CFStringRef kCTFontPreferredFamilyNameKey;

@implementation fontStyleData

- (id)initWithFont:(CTFontRef)f
{
	self = [super init];
	if (self) {
		self->font = f;
		CFRetain(self->font);
		self->desc = CTFontCopyDescriptor(self->font);
		if (![self prepare]) {
			[self release];
			return nil;
		}
	}
	return self;
}

- (id)initWithDescriptor:(CTFontDescriptorRef)d
{
	self = [super init];
	if (self) {
		self->font = NULL;
		self->desc = d;
		CFRetain(self->desc);
		if (![self prepare]) {
			[self release];
			return nil;
		}
	}
	return self;
}

- (void)dealloc
{
#define REL(x) if (x != NULL) { CFRelease(x); x = NULL; }
	REL(self->variationAxes);
	REL(self->familyName);
	REL(self->preferredFamilyName);
	REL(self->fullName);
	REL(self->subFamilyName);
	REL(self->preferredSubFamilyName);
	REL(self->postScriptName);
	REL(self->variations);
	REL(self->styleName);
	REL(self->traits);
	CFRelease(self->desc);
	REL(self->font);
	[super dealloc];
}

- (BOOL)prepare
{
	CFNumberRef num;
	Boolean success;

	self->traits = NULL;
	self->symbolic = 0;
	self->weight = 0;
	self->width = 0;
	self->didStyleName = NO;
	self->styleName = NULL;
	self->didVariations = NO;
	self->variations = NULL;
	self->hasRegistrationScope = NO;
	self->registrationScope = 0;
	self->didPostScriptName = NO;
	self->postScriptName = NULL;
	self->fontFormat = 0;
	self->didPreferredSubFamilyName = NO;
	self->preferredSubFamilyName = NULL;
	self->didSubFamilyName = NO;
	self->subFamilyName = NULL;
	self->didFullName = NO;
	self->fullName = NULL;
	self->didPreferredFamilyName = NO;
	self->preferredFamilyName = NULL;
	self->didFamilyName = NO;
	self->familyName = NULL;
	self->didVariationAxes = NO;
	self->variationAxes = NULL;

	self->traits = (CFDictionaryRef) CTFontDescriptorCopyAttribute(self->desc, kCTFontTraitsAttribute);
	if (self->traits == NULL)
		return NO;

	num = (CFNumberRef) CFDictionaryGetValue(self->traits, kCTFontSymbolicTrait);
	if (num == NULL)
		return NO;
	if (CFNumberGetValue(num, kCFNumberSInt32Type, &(self->symbolic)) == false)
		return NO;

	num = (CFNumberRef) CFDictionaryGetValue(self->traits, kCTFontWeightTrait);
	if (num == NULL)
		return NO;
	if (CFNumberGetValue(num, kCFNumberDoubleType, &(self->weight)) == false)
		return NO;

	num = (CFNumberRef) CFDictionaryGetValue(self->traits, kCTFontWidthTrait);
	if (num == NULL)
		return NO;
	if (CFNumberGetValue(num, kCFNumberDoubleType, &(self->width)) == false)
		return NO;

	// do these now for the sake of error checking
	num = (CFNumberRef) CTFontDescriptorCopyAttribute(desc, kCTFontRegistrationScopeAttribute);
	self->hasRegistrationScope = num != NULL;
	if (self->hasRegistrationScope) {
		success = CFNumberGetValue(num, kCFNumberSInt32Type, &(self->registrationScope));
		CFRelease(num);
		if (success == false)
			return NO;
	}

	num = (CFNumberRef) CTFontDescriptorCopyAttribute(self->desc, kCTFontFormatAttribute);
	if (num == NULL)
		return NO;
	success = CFNumberGetValue(num, kCFNumberSInt32Type, &(self->fontFormat));
	CFRelease(num);
	if (success == false)
		return NO;

	return YES;
}

- (void)ensureFont
{
	if (self->font != NULL)
		return;
	self->font = CTFontCreateWithFontDescriptor(self->desc, 0.0, NULL);
}

- (CTFontSymbolicTraits)symbolicTraits
{
	return self->symbolic;
}

- (double)weight
{
	return self->weight;
}

- (double)width
{
	return self->width;
}

- (CFStringRef)styleName
{
	if (!self->didStyleName) {
		self->didStyleName = YES;
		self->styleName = (CFStringRef) CTFontDescriptorCopyAttribute(self->desc, kCTFontStyleNameAttribute);
		// The code we use this for (guessItalicOblique() below) checks if this is NULL or not, so we're good.
	}
	return self->styleName;
}

- (CFDictionaryRef)variations
{
	if (!self->didVariations) {
		self->didVariations = YES;
		self->variations = (CFDictionaryRef) CTFontDescriptorCopyAttribute(self->desc, kCTFontVariationsAttribute);
		// This being NULL is used to determine whether a font uses variations at all, so we don't need to worry now.
	}
	return self->variations;
}

- (BOOL)hasRegistrationScope
{
	return self->hasRegistrationScope;
}

- (CTFontManagerScope)registrationScope
{
	return self->registrationScope;
}

- (CFStringRef)postScriptName
{
	if (!self->didPostScriptName) {
		self->didPostScriptName = YES;
		[self ensureFont];
		self->postScriptName = CTFontCopyPostScriptName(self->font);
	}
	return self->postScriptName;
}

- (CFDataRef)table:(CTFontTableTag)tag
{
	[self ensureFont];
	return CTFontCopyTable(self->font, tag, kCTFontTableOptionNoOptions);
}

- (CTFontFormat)fontFormat
{
	return self->fontFormat;
}

// We don't need to worry if this or any of the functions that use it return NULL, because the code that uses it (libFontRegistry.dylib bug workarounds in fonttraits.m) checks for NULL.
- (CFStringRef)fontName:(CFStringRef)key
{
	[self ensureFont];
	return CTFontCopyName(self->font, key);
}

#define FONTNAME(sel, did, var, key) \
	- (CFString)sel \
	{ \
		if (!did) { \
			did = YES; \
			var = [self fontName:key]; \
		} \
		return var; \
	}
FONTNAME(preferredSubFamilyName,
	self->didPreferredSubFamilyName,
	self->preferredSubFamilyName,
	kCTFontPreferredSubFamilyNameKey)
FONTNAME(subFamilyName,
	self->didSubFamilyName,
	self->subFamilyName,
	kCTFontSubFamilyNameKey)
FONTNAME(fullName,
	self->didFullName,
	self->fullName,
	kCTFontFullNameKey)
FONTNAME(preferredFamilyName,
	self->didPreferredFamilyName,
	self->preferredFamilyName,
	kCTFontPreferredFamilyNameKey)
FONTNAME(familyName,
	self->didFamilyName,
	self->familyName,
	kCTFontFamilyNameKey)

- (CFArrayRef)variationAxes
{
	if (!self->didVariationAxes) {
		self->didVariationAxes = YES;
		[self ensureFont];
		self->variationAxes = CTFontCopyVariationAxes(self->font);
		// We don't care about the return value because we call this only on fonts that we know have variations anyway.
	}
	return self->variationAxes;
}

@end

struct closeness {
	CFIndex index;
	double weight;
	double italic;
	double stretch;
	double distance;
};

static double doubleAttr(CFDictionaryRef traits, CFStringRef attr)
{
	CFNumberRef cfnum;
	double val;

	cfnum = (CFNumberRef) CFDictionaryGetValue(traits, attr);
	if (cfnum == NULL) {
		// TODO
	}
	if (CFNumberGetValue(cfnum, kCFNumberDoubleType, &val) == false) {
		// TODO
	}
	// Get Rule; do not release cfnum
	return val;
}

struct italicCloseness {
	double normal;
	double oblique;
	double italic;
};

// remember that in closeness, 0 means exact
// in this case, since we define the range, we use 0.5 to mean "close enough" (oblique for italic and italic for oblique) and 1 to mean "not a match"
static const struct italicCloseness italicClosenesses[] = {
	[uiDrawTextItalicNormal] = { 0, 1, 1 },
	[uiDrawTextItalicOblique] = { 1, 0, 0.5 },
	[uiDrawTextItalicItalic] = { 1, 0.5, 0 },
};

// Italics are hard because Core Text does NOT distinguish between italic and oblique.
// All Core Text provides is a slant value and the italic bit of the symbolic traits mask.
// However, Core Text does seem to guarantee (from experimentation; see below) that the slant will be nonzero if and only if the italic bit is set, so we don't need to use the slant value.
// Core Text also seems to guarantee that if a font lists itself as Italic or Oblique by name (font subfamily name, font style name, whatever), it will also have that bit set, so testing this bit does cover all fonts that name themselves as Italic and Oblique. (Again, this is from the below experimentation.)
// TODO there is still one catch that might matter from a user's POV: the reverse is not true — the italic bit can be set even if the style of the font face/subfamily/style isn't named as Italic (for example, script typefaces like Adobe's Palace Script MT Std); I don't know what to do about this... I know how to start: find a script font that has an italic form (Adobe's Palace Script MT Std does not; only Regular and Semibold)
static double italicCloseness(CTFontDescriptorRef desc, CFDictionaryRef traits, uiDrawTextItalic italic)
{
	const struct italicCloseness *ic = &(italicClosenesses[italic]);
	CFNumberRef cfnum;
	CTFontSymbolicTraits symbolic;
	// there is no kCFNumberUInt32Type, but CTFontSymbolicTraits is uint32_t, so SInt32 should work
	SInt32 s;
	CFStringRef styleName;
	BOOL isOblique;

	cfnum = CFDictionaryGetValue(traits, kCTFontSymbolicTrait);
	if (cfnum == NULL) {
		// TODO
	}
	if (CFNumberGetValue(cfnum, kCFNumberSInt32Type, &s) == false) {
		// TODO
	}
	symbolic = (CTFontSymbolicTraits) s;
	// Get Rule; do not release cfnum
	if ((symbolic & kCTFontItalicTrait) == 0)
		return ic->normal;

	// Okay, now we know it's either Italic or Oblique
	// Pango's Core Text code just does a g_strrstr() (backwards case-sensitive search) for "Oblique" in the font's style name (see https://git.gnome.org/browse/pango/tree/pango/pangocoretext-fontmap.c); let's do that too I guess
	isOblique = NO;		// default value
	styleName = (CFStringRef) CTFontDescriptorCopyAttribute(desc, kCTFontStyleNameAttribute);
	// TODO is styleName guaranteed?
	if (styleName != NULL) {
		CFRange range;

		// note the use of the toll-free bridge for the string literal, since CFSTR() *can* return NULL
		// TODO is this really the case? or is that just a copy-paste error from the other CFStringCreateXxx() functions? and what's this about -fconstant-cfstring?
		range = CFStringFind(styleName, (CFStringRef) @"Oblique", kCFCompareBackwards);
		if (range.location != kCFNotFound)
			isOblique = YES;
		CFRelease(styleName);
	}
	if (isOblique)
		return ic->oblique;
	return ic->italic;
}

static CTFontDescriptorRef matchTraits(CTFontDescriptorRef against, double targetWeight, uiDrawTextItalic targetItalic, double targetStretch)
{
	CFArrayRef matching;
	CFIndex i, n;
	struct closeness *closeness;
	CTFontDescriptorRef current;
	CTFontDescriptorRef out;

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

		closeness[i].index = i;
		current = (CTFontDescriptorRef) CFArrayGetValueAtIndex(matching, i);
		traits = (CFDictionaryRef) CTFontDescriptorCopyAttribute(current, kCTFontTraitsAttribute);
		if (traits == NULL) {
			// couldn't get traits; be safe by ranking it lowest
			// LONGTERM figure out what the longest possible distances are
			closeness[i].weight = 3;
			closeness[i].italic = 2;
			closeness[i].stretch = 3;
			continue;
		}
		closeness[i].weight = doubleAttr(traits, kCTFontWeightTrait) - targetWeight;
		closeness[i].italic = italicCloseness(current, traits, targetItalic);
		closeness[i].stretch = doubleAttr(traits, kCTFontWidthTrait) - targetStretch;
		CFRelease(traits);
	}

	// now figure out the 3-space difference between the three and sort by that
	// TODO merge this loop with the previous loop?
	for (i = 0; i < n; i++) {
		double weight, italic, stretch;

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
		// LONGTERM is this really the best way? isn't it the same as if (*a < *b) return -1; if (*a > *b) return 1; return 0; ?
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

CTFontDescriptorRef fontdescToCTFontDescriptor(uiDrawFontDescriptor *fd)
{
	CFMutableDictionaryRef attrs;
	CFStringRef cffamily;
	CFNumberRef cfsize;
	CTFontDescriptorRef basedesc;

	attrs = CFDictionaryCreateMutable(NULL, 2,
		// TODO are these correct?
		&kCFCopyStringDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	if (attrs == NULL) {
		// TODO
	}
	cffamily = CFStringCreateWithCString(NULL, fd->Family, kCFStringEncodingUTF8);
	if (cffamily == NULL) {
		// TODO
	}
	CFDictionaryAddValue(attrs, kCTFontFamilyNameAttribute, cffamily);
	CFRelease(cffamily);
	cfsize = CFNumberCreate(NULL, kCFNumberDoubleType, &(fd->Size));
	CFDictionaryAddValue(attrs, kCTFontSizeAttribute, cfsize);
	CFRelease(cfsize);

	basedesc = CTFontDescriptorCreateWithAttributes(attrs);
	CFRelease(attrs);			// TODO correct?
	return matchTraits(basedesc,
		weightToCTWeight(fd->Weight),
		fd->Italic,
		stretchesToCTWidths[fd->Stretch]);
}

// fortunately features that aren't supported are simply ignored, so we can copy them all in
CTFontDescriptorRef fontdescAppendFeatures(CTFontDescriptorRef desc, const uiOpenTypeFeatures *otf)
{
	CTFontDescriptorRef new;
	CFArrayRef featuresArray;
	CFDictionaryRef attrs;
	const void *keys[1], *values[1];

	featuresArray = otfToFeaturesArray(otf);
	keys[0] = kCTFontFeatureSettingsAttribute;
	values[0] = featuresArray;
	attrs = CFDictionaryCreate(NULL,
		keys, values, 1,
		// TODO are these correct?
		&kCFCopyStringDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	CFRelease(featuresArray);
	new = CTFontDescriptorCreateCopyWithAttributes(desc, attrs);
	CFRelease(attrs);
	CFRelease(desc);
	return new;
}

// TODO deduplicate this from italicCloseness()
static uiDrawTextItalic italicFromCTItalic(CTFontDescriptorRef desc, CFDictionaryRef traits)
{
	CFNumberRef cfnum;
	CTFontSymbolicTraits symbolic;
	// there is no kCFNumberUInt32Type, but CTFontSymbolicTraits is uint32_t, so SInt32 should work
	SInt32 s;
	CFStringRef styleName;
	BOOL isOblique;

	cfnum = CFDictionaryGetValue(traits, kCTFontSymbolicTrait);
	if (cfnum == NULL) {
		// TODO
	}
	if (CFNumberGetValue(cfnum, kCFNumberSInt32Type, &s) == false) {
		// TODO
	}
	symbolic = (CTFontSymbolicTraits) s;
	// Get Rule; do not release cfnum
	if ((symbolic & kCTFontItalicTrait) == 0)
		return uiDrawTextItalicNormal;

	// Okay, now we know it's either Italic or Oblique
	// Pango's Core Text code just does a g_strrstr() (backwards case-sensitive search) for "Oblique" in the font's style name (see https://git.gnome.org/browse/pango/tree/pango/pangocoretext-fontmap.c); let's do that too I guess
	isOblique = NO;		// default value
	styleName = (CFStringRef) CTFontDescriptorCopyAttribute(desc, kCTFontStyleNameAttribute);
	// TODO is styleName guaranteed?
	if (styleName != NULL) {
		CFRange range;

		// note the use of the toll-free bridge for the string literal, since CFSTR() *can* return NULL
		// TODO is this really the case? or is that just a copy-paste error from the other CFStringCreateXxx() functions? and what's this about -fconstant-cfstring?
		range = CFStringFind(styleName, (CFStringRef) @"Oblique", kCFCompareBackwards);
		if (range.location != kCFNotFound)
			isOblique = YES;
		CFRelease(styleName);
	}
	if (isOblique)
		return uiDrawTextItalicOblique;
	return uiDrawTextItalicItalic;
}

void fontdescFromCTFontDescriptor(CTFontDescriptorRef ctdesc, uiDrawFontDescriptor *uidesc)
{
	CFStringRef cffamily;
	CFDictionaryRef traits;
	double ctweight, ctstretch;
	int wc;
	uiDrawTextStretch stretch;

	cffamily = (CFStringRef) CTFontDescriptorCopyAttribute(ctdesc, kCTFontFamilyNameAttribute);
	if (cffamily == NULL) {
		// TODO
	}
	// TODO normalize this by adding a uiDarwinCFStringToText()
	uidesc->Family = uiDarwinNSStringToText((NSString *) cffamily);
	CFRelease(cffamily);

	traits = (CFDictionaryRef) CTFontDescriptorCopyAttribute(ctdesc, kCTFontTraitsAttribute);
	if (traits == NULL) {
		// TODO
	}
	ctweight = doubleAttr(traits, kCTFontWeightTrait);
	uidesc->Italic = italicFromCTItalic(ctdesc, traits);
	ctstretch = doubleAttr(traits, kCTFontWidthTrait);
	CFRelease(traits);

	// TODO make sure this is correct
	for (wc = 0; wc < 10; wc++)
		if (ctweight >= weightsToCTWeights[wc])
			if (ctweight < weightsToCTWeights[wc + 1])
				break;
	uidesc->Weight = ((ctweight - weightsToCTWeights[wc]) / (weightsToCTWeights[wc + 1] - weightsToCTWeights[wc])) * 100;
	uidesc->Weight += wc * 100;

	// TODO is this correct?
	for (stretch = uiDrawTextStretchUltraCondensed; stretch < uiDrawTextStretchUltraExpanded; stretch++)
		if (ctstretch <= stretchesToCTWidths[stretch])
			break;
	uidesc->Stretch = stretch;
}
