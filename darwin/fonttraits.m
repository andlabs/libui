// 1 november 2017
#import "uipriv_darwin.h"
#import "attrstr.h"

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

static BOOL fontRegistered(uiprivFontStyleData *d)
{
	if (![d hasRegistrationScope])
		// header says this should be treated as the same as not registered
		return NO;
	// examination of Core Text shows this is accurate
	return [d registrationScope] != kCTFontManagerScopeNone;
}

// Core Text does (usWidthClass / 10) - 0.5 here.
// This roughly maps to our values with increments of 0.1, except for the fact 0 and 10 are allowed by Core Text, despite being banned by TrueType and OpenType themselves.
// We'll just treat them as identical to 1 and 9, respectively.
static const uiTextStretch os2WidthsToStretches[] = {
	uiTextStretchUltraCondensed,
	uiTextStretchUltraCondensed,
	uiTextStretchExtraCondensed,
	uiTextStretchCondensed,
	uiTextStretchSemiCondensed,
	uiTextStretchNormal,
	uiTextStretchSemiExpanded,
	uiTextStretchExpanded,
	uiTextStretchExtraExpanded,
	uiTextStretchUltraExpanded,
	uiTextStretchUltraExpanded,
};

static const CFStringRef exceptions[] = {
	CFSTR("LucidaGrande"),
	CFSTR(".LucidaGrandeUI"),
	CFSTR("STHeiti"),
	CFSTR("STXihei"),
	CFSTR("TimesNewRomanPSMT"),
	NULL,
};

static void trySecondaryOS2Values(uiprivFontStyleData *d, uiFontDescriptor *out, BOOL *hasWeight, BOOL *hasWidth)
{
	CFDataRef os2;
	uint16_t usWeightClass, usWidthClass;
	CFStringRef psname;
	const CFStringRef *ex;

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
				usWeightClass *= 100;
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

static BOOL testTTFOTFSubfamilyNames(uiprivFontStyleData *d, CFStringRef want)
{
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
static BOOL shouldReallyBeThin(uiprivFontStyleData *d)
{
	return testTTFOTFSubfamilyNames(d, CFSTR("W1"));
}

// work around a bug in libFontRegistry.dylib
static BOOL shouldReallyBeSemiCondensed(uiprivFontStyleData *d)
{
	return testTTFOTFSubfamilyNames(d, CFSTR("Semi Condensed"));
}

void uiprivProcessFontTraits(uiprivFontStyleData *d, uiFontDescriptor *out)
{
	double weight, width;
	BOOL hasWeight, hasWidth;

	hasWeight = NO;
	hasWidth = NO;
	trySecondaryOS2Values(d, out, &hasWeight, &hasWidth);

	weight = [d weight];
	width = [d width];

	if (!hasWeight)
		// TODO this scale is a bit lopsided
		if (weight <= -0.7)
			out->Weight = uiTextWeightThin;
		else if (weight <= -0.5)
			out->Weight = uiTextWeightUltraLight;
		else if (weight <= -0.3)
			out->Weight = uiTextWeightLight;
		else if (weight <= -0.23) {
			out->Weight = uiTextWeightBook;
			if (shouldReallyBeThin(d))
				out->Weight = uiTextWeightThin;
		} else if (weight <= 0.0)
			out->Weight = uiTextWeightNormal;
		else if (weight <= 0.23)
			out->Weight = uiTextWeightMedium;
		else if (weight <= 0.3)
			out->Weight = uiTextWeightSemiBold;
		else if (weight <= 0.4)
			out->Weight = uiTextWeightBold;
		else if (weight <= 0.5)
			out->Weight = uiTextWeightUltraBold;
		else if (weight <= 0.7)
			out->Weight = uiTextWeightHeavy;
		else
			out->Weight = uiTextWeightUltraHeavy;

	if (!hasWidth)
		// TODO this scale is a bit lopsided
		if (width <= -0.7) {
			out->Stretch = uiTextStretchUltraCondensed;
			if (shouldReallyBeSemiCondensed(d))
				out->Stretch = uiTextStretchSemiCondensed;
		} else if (width <= -0.5)
			out->Stretch = uiTextStretchExtraCondensed;
		else if (width <= -0.2)
			out->Stretch = uiTextStretchCondensed;
		else if (width <= -0.1)
			out->Stretch = uiTextStretchSemiCondensed;
		else if (width <= 0.0)
			out->Stretch = uiTextStretchNormal;
		else if (width <= 0.1)
			out->Stretch = uiTextStretchSemiExpanded;
		else if (width <= 0.2)
			out->Stretch = uiTextStretchExpanded;
		else if (width <= 0.6)
			out->Stretch = uiTextStretchExtraExpanded;
		else
			out->Stretch = uiTextStretchUltraExpanded;
}
