// 1 november 2017
#import "uipriv_darwin.h"
#import "fontstyle.h"

// This is the part of the font style matching and normalization code
// that handles fonts that use a traits dictionary.
//
// Matching stupidity: Core Text requires an **exact match for the
// entire traits dictionary**, otherwise it will **drop ALL the traits**.
//
// Normalization stupidity: Core Text uses its own scaled values for
// weight and width, but the values are different if the font is not
// registered and if said font is TrueType or OpenType. The values
// for all other cases do have some named constants starting with
// OS X 10.11, but even these aren't very consistent in practice.
//
// Of course, none of this is documented anywhere, so I had to do
// both trial-and-error AND reverse engineering to figure out what's
// what. We'll just convert Core Text's values into libui constants
// and use those for matching.

static BOOL fontRegistered(fontStyleData *d)
{
	if (![d hasRegistrationScope])
		// header says this should be treated as the same as not registered
		return NO;
	// examination of Core Text shows this is accurate
	return [d registrationScope] != kCTFontManagerScopeNone;
}

// Core Text doesn't seem to differentiate between Italic and Oblique.
// Pango's Core Text code just does a g_strrstr() (backwards case-sensitive search) for "Oblique" in the font's style name (see https://git.gnome.org/browse/pango/tree/pango/pangocoretext-fontmap.c); let's do that too I guess
static uiDrawTextFontItalic guessItalicOblique(fontStyleData *d)
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
		return uiDrawFontItalicOblique;
	return uiDrawFontItalicItalic;
}

// Core Text does (usWidthClass / 10) - 0.5 here.
// This roughly maps to our values with increments of 0.1, except for the fact 0 and 10 are allowed by Core Text, despite being banned by TrueType and OpenType themselves.
// We'll just treat them as identical to 1 and 9, respectively.
static const uiDrawFontStretch os2WidthsToStretches[] = {
	uiDrawTextStretchUltraCondensed,
	uiDrawTextStretchUltraCondensed,
	uiDrawTextStretchExtraCondensed,
	uiDrawTextStretchCondensed,
	uiDrawTextStretchSemiCondensed,
	uiDrawTextStretchNormal,
	uiDrawTextStretchSemiExpanded,
	uiDrawTextStretchExpanded,
	uiDrawTextStretchExtraExpanded,
	uiDrawTextStretchUltraExpanded,
	uiDrawTextStretchUltraExpanded,
};

static const CFStringRef exceptions[] = {
	CFSTR("LucidaGrande"),
	CFSTR(".LucidaGrandeUI"),
	CFSTR("STHeiti"),
	CFSTR("STXihei"),
	CFSTR("TimesNewRomanPSMT"),
	NULL,
};

static void trySecondaryOS2Values(fontStyleData *d, uiDrawFontDescriptor *out, BOOL *hasWeight, BOOL *hasWidth)
{
	CFDataRef os2;
	uint16_t usWeightClass, usWidthClass;
	CFStringRef psname;
	CFStringRef *ex;

	*hasWeight = NO;
	*hasWidth = NO;

	// only applies to unregistered fonts
	if (fontRegistered(d))
		return;

	os2 = [d table:kCTFontTableOS2];
	if (os2 == NULL)
		// no OS2 table, so no secondary values
		return;

	if (CFDataGetLength(os2) > 77) {
		const UInt8 *b;

		b = CFDataGetBytePtr(os2);

		usWeightClass = ((uint16_t) (b[4])) << 8;
		usWeightClass |= (uint16_t) (b[5]);
		if (usWeightClass <= 1000) {
			if (usWeightClass < 11)
				usWeigthClass *= 100;
			*hasWeight = YES;
		}

		usWidthClass = ((uint16_t) (b[6])) << 8;
		usWidthClass |= (uint16_t) (b[7]);
		if (usWidthClass <= 10)
			*hasWidth = YES;
	} else {
		usWeightClass = 0;
		*hasWeight = YES;

		usWidthClass = 0;
		*hasWidth = YES;
	}
	if (*hasWeight)
		// we can just use this directly
		out->Weight = usWeightClass;
	if (*hasWidth)
		out->Stretch = os2WidthsToStretches[usWidthClass];
	CFRelease(os2);

	// don't use secondary weights in the event of special predefined names
	psname = [d postScriptName];
	for (ex = exceptions; *ex != NULL; ex++)
		if (CFEqual(psname, *ex)) {
			*hasWeight = NO;
			break;
		}
}

static BOOL testTTFOTFSubfamilyName(CFStringRef name, CFStringRef want)
{
	CFRange range;

	if (name == NULL)
		return NO;
	range.location = 0;
	range.length = CFStringGetLength(name);
	return CFStringFindWithOptions(name, want, range,
		(kCFCompareCaseInsensitive | kCFCompareBackwards | kCFCompareNonliteral), NULL) != false;
}

static BOOL testTTFOTFSubfamilyNames(fontStyleData *d, CFStringRef want)
{
	CGFontRef font;
	CFString *key;

	switch ([d fontFormat]) {
	case kCTFontFormatOpenTypePostScript:
	case kCTFontFormatOpenTypeTrueType:
	case kCTFontFormatTrueType:
		break;
	default:
		return NO;
	}

	if (testTTFOTFSubfamilyName([d preferredSubFamilyName], want))
		return YES;
	if (testTTFOTFSubfamilyName([d subFamilyName], want))
		return YES;
	if (testTTFOTFSubfamilyName([d fullName], want))
		return YES;
	if (testTTFOTFSubfamilyName([d preferredFamilyName], want))
		return YES;
	return testTTFOTFSubfamilyName([d familyName], want);
}

// work around a bug in libFontRegistry.dylib
static BOOL shouldReallyBeThin(CTFontDescriptorRef desc)
{
	return testTTFOTFSubfamilyNames(desc, CFSTR("W1"));
}

// work around a bug in libFontRegistry.dylib
static BOOL shouldReallyBeSemiCondensed(CTFontDescriptorRef desc)
{
	return testTTFOTFSubfamilyNames(desc, CFSTR("Semi Condensed"));
}

void processFontTraits(fontStyleData *d, uiDrawFontDescriptor *out)
{
	double weight, width;
	BOOL hasWeight, hasWidth;

	out->Italic = uiDrawTextItalicNormal;
	if (([d symbolicTraits] & kCTFontItalicTrait) != 0)
		out->Italic = guessItalicOblique(d);

	hasWeight = NO;
	hasWidth = NO;
	trySecondaryOS2Values(d, out, &hasWeight, &hasWidth);

	weight = [d weight];
	width = [d width];

	if (!hasWeight)
		// TODO this scale is a bit lopsided
		if (weight <= -0.7)
			out->Weight = uiDrawTextWeightThin;
		else if (weight <= -0.5)
			out->Weight = uiDrawTextWeightUltraLight;
		else if (weight <= -0.3)
			out->Weight = uiDrawTextWeightLight;
		else if (weight <= -0.23) {
			out->Weight = uiDrawTextWeightBook;
			if (shouldReallyBeThin(d))
				out->Weight = uiDrawTextWeightThin;
		} else if (weight <= 0.0)
			out->Weight = uiDrawTextWeightNormal;
		else if (weight <= 0.23)
			out->Weight = uiDrawTextWeightMedium;
		else if (weight <= 0.3)
			out->Weight = uiDrawTextWeightSemiBold;
		else if (weight <= 0.4)
			out->Weight = uiDrawTextWeightBold;
		else if (weight <= 0.5)
			out->Weight = uiDrawTextWeightUltraBold;
		else if (weight <= 0.7)
			out->Weight = uiDrawTextWeightHeavy;
		else
			out->Weight = uiDrawTextWeightUltraHeavy;

	if (!hasWidth)
		// TODO this scale is a bit lopsided
		if (width <= -0.7) {
			out->Stretch = uiDrawTextStretchUltraCondensed;
			if (shouldReallyBeSemiCondensed(d))
				out->Stretch = uiDrawTextStretchSemiCondensed;
		} else if (width <= -0.5)
			out->Stretch = uiDrawTextStretchExtraCondensed;
		else if (width <= -0.2)
			out->Stretch = uiDrawTextStretchCondensed;
		else if (width <= -0.1)
			out->Stretch = uiDrawTextStretchSemiCondensed;
		else if (width <= 0.0)
			out->Stretch = uiDrawTextStretchNormal;
		else if (width <= 0.1)
			out->Stretch = uiDrawTextStretchSemiExpanded;
		else if (width <= 0.2)
			out->Stretch = uiDrawTextStretchExpanded;
		else if (width <= 0.6)
			out->Stretch = uiDrawTextStretchExtraExpanded;
		else
			out->Stretch = uiDrawTextStretchUltraExpanded;
}
