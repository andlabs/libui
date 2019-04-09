// 3 january 2017
#import "uipriv_darwin.h"
#import "attrstr.h"

// TODOs:
// - switching from Skia to a non-fvar-based font crashes because the CTFontDescriptorRef we get has an empty variation dictionary for some reason...
// - Futura causes the Courier New in the drawtext example to be bold for some reason...

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
// normalization code for filling in uiFontDescriptors from
// Core Text fonts and font descriptors.
//
// Style matching needs to be done early in the font loading process;
// in particular, we have to do this before adding any features,
// because the descriptors returned by Core Text's own font
// matching won't have any.

@implementation uiprivFontStyleData

- (id)initWithFont:(CTFontRef)f
{
	self = [super init];
	if (self) {
		self->font = f;
		CFRetain(self->font);
		self->desc = CTFontCopyFontDescriptor(self->font);
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
	REL(self->variation);
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
	self->didVariation = NO;
	self->variation = NULL;
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

- (CFDictionaryRef)variation
{
	if (!self->didVariation) {
		self->didVariation = YES;
		self->variation = (CFDictionaryRef) CTFontDescriptorCopyAttribute(self->desc, kCTFontVariationAttribute);
		// This being NULL is used to determine whether a font uses variations at all, so we don't need to worry now.
	}
	return self->variation;
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
	- (CFStringRef)sel \
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
	uiprivUNDOC_kCTFontPreferredSubFamilyNameKey)
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
	uiprivUNDOC_kCTFontPreferredFamilyNameKey)
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
	uiTextWeight weight;
	double italic;
	uiTextStretch stretch;
	double distance;
};

// remember that in closeness, 0 means exact
// in this case, since we define the range, we use 0.5 to mean "close enough" (oblique for italic and italic for oblique) and 1 to mean "not a match"
static const double italicClosenessNormal[] = { 0, 1, 1 };
static const double italicClosenessOblique[] = { 1, 0, 0.5 };
static const double italicClosenessItalic[] = { 1, 0.5, 0 };
static const double *italicClosenesses[] = {
	[uiTextItalicNormal] = italicClosenessNormal,
	[uiTextItalicOblique] = italicClosenessOblique,
	[uiTextItalicItalic] = italicClosenessItalic,
};

// Core Text doesn't seem to differentiate between Italic and Oblique.
// Pango's Core Text code just does a g_strrstr() (backwards case-sensitive search) for "Oblique" in the font's style name (see https://git.gnome.org/browse/pango/tree/pango/pangocoretext-fontmap.c); let's do that too I guess
static uiTextItalic guessItalicOblique(uiprivFontStyleData *d)
{
	CFStringRef styleName;
	BOOL isOblique;

	isOblique = NO;		// default value
	styleName = [d styleName];
	if (styleName != NULL) {
		CFRange range;

		range = CFStringFind(styleName, CFSTR("Oblique"), kCFCompareBackwards);
		if (range.location != kCFNotFound)
			isOblique = YES;
	}
	if (isOblique)
		return uiTextItalicOblique;
	return uiTextItalicItalic;
}

// Italics are hard because Core Text does NOT distinguish between italic and oblique.
// All Core Text provides is a slant value and the italic bit of the symbolic traits mask.
// However, Core Text does seem to guarantee (from experimentation; see below) that the slant will be nonzero if and only if the italic bit is set, so we don't need to use the slant value.
// Core Text also seems to guarantee that if a font lists itself as Italic or Oblique by name (font subfamily name, font style name, whatever), it will also have that bit set, so testing this bit does cover all fonts that name themselves as Italic and Oblique. (Again, this is from the below experimentation.)
// TODO there is still one catch that might matter from a user's POV: the reverse is not true — the italic bit can be set even if the style of the font face/subfamily/style isn't named as Italic (for example, script typefaces like Adobe's Palace Script MT Std); I don't know what to do about this... I know how to start: find a script font that has an italic form (Adobe's Palace Script MT Std does not; only Regular and Semibold)
static void setItalic(uiprivFontStyleData *d, uiFontDescriptor *out)
{
	out->Italic = uiTextItalicNormal;
	if (([d symbolicTraits] & kCTFontItalicTrait) != 0)
		out->Italic = guessItalicOblique(d);
}

static void fillDescStyleFields(uiprivFontStyleData *d, NSDictionary *axisDict, uiFontDescriptor *out)
{
	setItalic(d, out);
	if (axisDict != nil)
		uiprivProcessFontVariation(d, axisDict, out);
	else
		uiprivProcessFontTraits(d, out);
}

static CTFontDescriptorRef matchStyle(CTFontDescriptorRef against, uiFontDescriptor *styles)
{
	CFArrayRef matching;
	CFIndex i, n;
	struct closeness *closeness;
	CTFontDescriptorRef current;
	CTFontDescriptorRef out;
	uiprivFontStyleData *d;
	NSDictionary *axisDict;

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

	current = (CTFontDescriptorRef) CFArrayGetValueAtIndex(matching, 0);
	d = [[uiprivFontStyleData alloc] initWithDescriptor:current];
	axisDict = nil;
	if ([d variation] != NULL)
		axisDict = uiprivMakeVariationAxisDict([d variationAxes], [d table:kCTFontTableAvar]);

	closeness = (struct closeness *) uiprivAlloc(n * sizeof (struct closeness), "struct closeness[]");
	for (i = 0; i < n; i++) {
		uiFontDescriptor fields;

		closeness[i].index = i;
		if (i != 0) {
			current = (CTFontDescriptorRef) CFArrayGetValueAtIndex(matching, i);
			d = [[uiprivFontStyleData alloc] initWithDescriptor:current];
		}
		fillDescStyleFields(d, axisDict, &fields);
		closeness[i].weight = fields.Weight - styles->Weight;
		closeness[i].italic = italicClosenesses[styles->Italic][fields.Italic];
		closeness[i].stretch = fields.Stretch - styles->Stretch;
		[d release];
	}

	// now figure out the 3-space difference between the three and sort by that
	// TODO merge this loop with the previous loop?
	for (i = 0; i < n; i++) {
		double weight, italic, stretch;

		weight = (double) (closeness[i].weight);
		weight *= weight;
		italic = closeness[i].italic;
		italic *= italic;
		stretch = (double) (closeness[i].stretch);
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
	if (axisDict != nil)
		[axisDict release];
	uiprivFree(closeness);
	CFRelease(matching);
	// and release the original descriptor since we no longer need it
	CFRelease(against);

	return out;
}

CTFontDescriptorRef uiprivFontDescriptorToCTFontDescriptor(uiFontDescriptor *fd)
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
	return matchStyle(basedesc, fd);
}

// fortunately features that aren't supported are simply ignored, so we can copy them all in
CTFontDescriptorRef uiprivCTFontDescriptorAppendFeatures(CTFontDescriptorRef desc, const uiOpenTypeFeatures *otf)
{
	CTFontDescriptorRef new;
	CFArrayRef featuresArray;
	CFDictionaryRef attrs;
	const void *keys[1], *values[1];

	featuresArray = uiprivOpenTypeFeaturesToCTFeatures(otf);
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

void uiprivFontDescriptorFromCTFontDescriptor(CTFontDescriptorRef ctdesc, uiFontDescriptor *uidesc)
{
	CFStringRef cffamily;
	uiprivFontStyleData *d;
	NSDictionary *axisDict;

	cffamily = (CFStringRef) CTFontDescriptorCopyAttribute(ctdesc, kCTFontFamilyNameAttribute);
	if (cffamily == NULL) {
		// TODO
	}
	// TODO normalize this by adding a uiDarwinCFStringToText()
	uidesc->Family = uiDarwinNSStringToText((NSString *) cffamily);
	CFRelease(cffamily);

	d = [[uiprivFontStyleData alloc] initWithDescriptor:ctdesc];
	axisDict = nil;
	if ([d variation] != NULL)
		axisDict = uiprivMakeVariationAxisDict([d variationAxes], [d table:kCTFontTableAvar]);
	fillDescStyleFields(d, axisDict, uidesc);
	if (axisDict != nil)
		[axisDict release];
	[d release];
}
