// 1 november 2017
#import "uipriv_darwin.h"

static BOOL fontRegistered(CTFontDescriptorRef desc)
{
	CFNumberRef scope;
	CTFontManagerScope val;

	scope = (CFNumberRef) CTFontDescriptorCopyAttribute(desc, kCTFontRegistrationScopeAttribute);
	if (scope == NULL)
		// header says this should be treated as the same as not registered
		return NO;
	if (CFNumberGetValue(scope, kCFNumberSInt32Type, &val) == false) {
		// TODO
		CFRelease(scope);
		return NO;
	}
	CFRelease(scope);
	// examination of Core Text shows this is accurate
	return val != kCTFontManagerScopeNone;
}

static BOOL getTraits(CTFontDescriptorRef desc, CTFontSymbolicTraits *symbolic, double *weight, double *width)
{
	CFDictionaryRef traits = NULL;
	CFNumberRef num = NULL;

	traits = (CFDictionaryRef) CTFontDescriptorCopyAttribute(desc, kCTFontTraitsAttribute);
	if (traits == NULL)
		return NO;

	num = (CFNumberRef) CFDictionaryGetValue(traits, kCTFontSymbolicTrait);
	if (num == NULL)
		goto fail;
	if (CFNumberGetValue(num, kCFNumberSInt32Type, symbolic) == false)
		goto fail;

	num = (CFNumberRef) CFDictionaryGetValue(traits, kCTFontWeightTrait);
	if (num == NULL)
		goto fail;
	if (CFNumberGetValue(num, kCFNumberDoubleType, weight) == false)
		goto fail;

	num = (CFNumberRef) CFDictionaryGetValue(traits, kCTFontWidthTrait);
	if (num == NULL)
		goto fail;
	if (CFNumberGetValue(num, kCFNumberDoubleType, width) == false)
		goto fail;

	CFRelease(traits);
	return YES;

fail:
	CFRelease(traits);
	return NO;
}

// Core Text doesn't seem to differentiate between Italic and Oblique.
// Pango's Core Text code just does a g_strrstr() (backwards case-sensitive search) for "Oblique" in the font's style name (see https://git.gnome.org/browse/pango/tree/pango/pangocoretext-fontmap.c); let's do that too I guess
static uiDrawTextFontItalic guessItalicOblique(CTFontDescriptorRef desc)
{
	CFStringRef styleName;
	BOOL isOblique;

	isOblique = NO;		// default value
	styleName = (CFStringRef) CTFontDescriptorCopyAttribute(desc, kCTFontStyleNameAttribute);
	if (styleName != NULL) {
		CFRange range;

		range = CFStringFind(styleName, CFSTR("Oblique"), kCFCompareBackwards);
		if (range.location != kCFNotFound)
			isOblique = YES;
		CFRelease(styleName);
	}
	if (isOblique)
		return uiDrawFontItalicOblique;
	return uiDrawFontItalicItalic;
}

// Core Text does (usWidthClass - 0.5) x 10
// this roughly maps to our values with increments of 0.1, except for the fact 0 and 10 are allowed by Core Text, despite being banned by TrueType and OpenType themselves
// we'll just treat them as identical to 1 and 9, respectively
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

static void trySecondaryOS2Values(CTFontDescriptorRef desc, uiDrawFontDescriptor *out, BOOL *hasWeight, BOOL *hasWidth)
{
	CTFontRef font;
	CFDataRef os2;
	uint16_t usWeightClass, usWidthClass;
	CFStringRef psname;
	CFStringRef *ex;

	*hasWeight = NO;
	*hasWidth = NO;

	// only applies to unregistered fonts
	if (fontRegistered(desc))
		return;

	font = CTFontCreateWithFontDescriptor(desc, 0.0, NULL);

	os2 = CTFontCopyTable(font, kCTFontTableOS2, kCTFontTableOptionNoOptions);
	if (os2 == NULL) {
		// no OS2 table, so no secondary values
		CFRelease(font);
		return;
	}

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
	psname = CTFontCopyPostScriptName(font);
	for (ex = exceptions; *ex != NULL; ex++)
		if (CFEqual(psname, *ex)) {
			*hasWeight = NO;
			break;
		}
	CFRelease(psname);

	CFRelease(font);
}

static const CFStringRef subfamilyKeys[] = {
	kCTFontSubFamilyNameKey,
	// TODO explicitly mark these as undocumented
	CFSTR("CTFontPreferredSubFamilyName"),
	kCTFontFullNameKey,
	CFSTR("CTFontPreferredFamilyName"),
	kCTFontFamilyNameKey,
	NULL,
};

static BOOL testTTFOTFSubfamilyNames(CTFontDescriptorRef desc, CFStringRef want)
{
	CFNumberRef num;
	CTFontFormat type;
	CGFontRef font;
	CFString *key;

	num = (CFNumberRef) CTFontDescriptorCopyAttribute(desc, kCTFontFormatAttribute);
	if (num == NULL) {
		// TODO
		return NO;
	}
	if (CFNumberGetValue(num, kCFNumberSInt32Type, &type) == false) {
		// TODO
		CFRelease(num);
		return NO;
	}
	CFRelease(num);
	switch (type) {
	case kCTFontFormatOpenTypePostScript:
	case kCTFontFormatOpenTypeTrueType:
	case kCTFontFormatTrueType:
		break;
	default:
		return NO;
	}

	font = CTFontCreateWithFontDescriptor(desc, 0.0, NULL);
	for (key = subfamilyKeys; *key != NULL; key++) {
		CFStringRef val;
		CFRange range;

		val = CTFontCopyName(font, *key);
		if (val == NULL)
			continue;
		range.location = 0;
		range.length = CFStringGetLength(val);
		if (CFStringFindWithOptions(val, want, range,
			(kCFCompareCaseInsensitive | kCFCompareBackwards | kCFCompareNonliteral), NULL) != false) {
			CFRelease(val);
			CFRelease(font);
			return YES;
		}
		CFRelease(val);
	}
	CFRelease(font);
	return NO;
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

void processFontTraits(CTFontDescriptorRef desc, uiDrawFontDescriptor *out)
{
	CTFontSymbolicTraits symbolic;
	double weight;
	double width;
	BOOL hasWeight, hasWidth;
	uint16_t usWeightClasss, usWidthClass;
	CTFontRef font;

	if (!getTraits(desc, &symbolic, &weight, &width)) {
		// TODO
		goto fail;
	}

	out->Italic = uiDrawTextItalicNormal;
	if ((symbolic & kCTFontItalicTrait) != 0)
		out->Italic = guessItalicOblique(desc);

	hasWeight = NO;
	hasWidth = NO;
	trySecondaryOS2Values(desc, out, &hasWeight, &hasWidth);

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
			if (shouldReallyBeThin(desc))
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
			if (shouldReallyBeSemiCondensed(desc))
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
