// 2 november 2017
#import "uipriv_darwin.h"
#import "attrstr.h"

// This is the part of the font style matching and normalization code
// that handles fonts that use the fvar table.
//
// Matching stupidity: Core Text **doesn't even bother** matching
// these, even if you tell it to do so explicitly. It'll always return
// all variations for a given font.
//
// Normalization stupidity: Core Text doesn't normalize the fvar
// table values for us, so we'll have to do it ourselves. Furthermore,
// Core Text doesn't provide an API for accessing the avar table, if
// any, so we must do so ourselves. (TODO does Core Text even
// follow the avar table if a font has it?)
//
// Thankfully, normalization is well-defined in both TrueType and
// OpenType and seems identical in both, so we can just normalize
// the values and then convert them linearly to libui values for
// matching.
//
// References:
// - https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6fvar.html
// - https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6avar.html
// - https://www.microsoft.com/typography/otspec/fvar.htm
// - https://www.microsoft.com/typography/otspec/otvaroverview.htm#CSN
// - https://www.microsoft.com/typography/otspec/otff.htm
// - https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6.html#Types
// - https://www.microsoft.com/typography/otspec/avar.htm

// TODO Skia doesn't quite map correctly; notice what passes for condensed in the drawtext example
// TODO also investigate Marker Felt not working right in Thin and Wide modes (but that's probably the other file, putting it here just so I don't forget)

#define fvarWeight 0x77676874
#define fvarWidth 0x77647468

// TODO explain why these are signed
typedef int32_t fixed1616;
typedef int16_t fixed214;

// note that Microsoft's data type list implies that *all* fixed-point types have the same format; it only gives specific examples for the 2.14 format, which confused me because I thought 16.16 worked differently, but eh
static fixed1616 doubleToFixed1616(double d)
{
	double ipart, fpart;
	long flong;
	int16_t i16;
	uint32_t ret;

	fpart = modf(d, &ipart);
	// fpart must be unsigned; modf() gives us fpart with the same sign as d (so we have to adjust both ipart and fpart appropriately)
	if (fpart < 0) {
		ipart -= 1;
		fpart = 1 + fpart;
	}
	fpart *= 65536;
	flong = lround(fpart);
	i16 = (int16_t) ipart;
	ret = (uint32_t) ((uint16_t) i16);
	ret <<= 16;
	ret |= (uint16_t) (flong & 0xFFFF);
	return (fixed1616) ret;
}

// see also https://stackoverflow.com/questions/8506317/fixed-point-unsigned-division-in-c and freetype's FT_DivFix()
// TODO figure out the specifics of freetype's more complex implementation that shifts b and juggles signs
static fixed1616 fixed1616Divide(fixed1616 a, fixed1616 b)
{
	uint32_t u;
	int64_t a64;

	u = (uint32_t) a;
	a64 = (int64_t) (((uint64_t) u) << 16);
	return (fixed1616) (a64 / b);
}

static fixed214 fixed1616ToFixed214(fixed1616 f)
{
	uint32_t t;
	uint32_t topbit;

	t = (uint32_t) (f + 0x00000002);
	topbit = t & 0x80000000;
	t >>= 2;
	if (topbit != 0)
		t |= 0xC000000;
	return (fixed214) (t & 0xFFFF);
}

static double fixed214ToDouble(fixed214 f)
{
	uint16_t u;
	double base;
	double frac;

	u = (uint16_t) f;
	switch ((u >> 14) & 0x3) {
	case 0:
		base = 0;
		break;
	case 1:
		base = 1;
		break;
	case 2:
		base = -2;
		break;
	case 3:
		base = -1;
	}
	frac = ((double) (u & 0x3FFF)) / 16384;
	return base + frac;
}

static fixed1616 fixed214ToFixed1616(fixed214 f)
{
	int32_t t;

	t = (int32_t) ((int16_t) f);
	t <<= 2;
	return (fixed1616) (t - 0x00000002);
}

static const fixed1616 fixed1616Negative1 = (int32_t) ((uint32_t) 0xFFFF0000);
static const fixed1616 fixed1616Zero = 0x00000000;
static const fixed1616 fixed1616Positive1 = 0x00010000;

static fixed1616 fixed1616Normalize(fixed1616 val, fixed1616 min, fixed1616 max, fixed1616 def)
{
	if (val < min)
		val = min;
	if (val > max)
		val = max;
	if (val < def)
		return fixed1616Divide(-(def - val), (def - min));
	if (val > def)
		return fixed1616Divide((val - def), (max - def));
	return fixed1616Zero;
}

static fixed214 normalizedTo214(fixed1616 val, const fixed1616 *avarMappings, size_t avarCount)
{
	if (val < fixed1616Negative1)
		val = fixed1616Negative1;
	if (val > fixed1616Positive1)
		val = fixed1616Positive1;
	if (avarCount != 0) {
		size_t start, end;
		fixed1616 startFrom, endFrom;
		fixed1616 startTo, endTo;

		for (end = 0; end < avarCount; end += 2) {
			endFrom = avarMappings[end];
			endTo = avarMappings[end + 1];
			if (endFrom >= val)
				break;
		}
		if (endFrom == val)
			val = endTo;
		else {
			start = end - 2;
			startFrom = avarMappings[start];
			startTo = avarMappings[start + 1];
			val = fixed1616Divide((val - startFrom), (endFrom - startFrom));
			// TODO find a font with an avar table and make sure this works, or if we need to use special code for this too
			val *= (endTo - startTo);
			val += startTo;
		}
	}
	return fixed1616ToFixed214(val);
}

static fixed1616 *avarExtract(CFDataRef table, CFIndex index, size_t *n)
{
	const UInt8 *b;
	size_t off;
	size_t i, nEntries;
	fixed1616 *entries;
	fixed1616 *p;

	b = CFDataGetBytePtr(table);
	off = 8;
#define nextuint16be() ((((uint16_t) (b[off])) << 8) | ((uint16_t) (b[off + 1])))
	for (; index > 0; index--) {
		nEntries = (size_t) nextuint16be();
		off += 2;
		off += 4 * nEntries;
	}
	nEntries = nextuint16be();
	*n = nEntries * 2;
	entries = (fixed1616 *) uiprivAlloc(*n * sizeof (fixed1616), "fixed1616[]");
	p = entries;
	for (i = 0; i < *n; i++) {
		*p++ = fixed214ToFixed1616((fixed214) nextuint16be());
		off += 2;
	}
	return entries;
}

static BOOL extractAxisDictValue(CFDictionaryRef dict, CFStringRef key, fixed1616 *out)
{
	CFNumberRef num;
	double v;

	num = (CFNumberRef) CFDictionaryGetValue(dict, key);
	if (CFNumberGetValue(num, kCFNumberDoubleType, &v) == false)
		return NO;
	*out = doubleToFixed1616(v);
	return YES;
}

// TODO here and elsewhere: make sure all Objective-C classes and possibly also custom method names have uipriv prefixes
@interface fvarAxis : NSObject {
	fixed1616 min;
	fixed1616 max;
	fixed1616 def;
	fixed1616 *avarMappings;
	size_t avarCount;
}
- (id)initWithIndex:(CFIndex)i dict:(CFDictionaryRef)dict avarTable:(CFDataRef)table;
- (double)normalize:(double)v;
@end

@implementation fvarAxis

- (id)initWithIndex:(CFIndex)i dict:(CFDictionaryRef)dict avarTable:(CFDataRef)table
{
	self = [super init];
	if (self) {
		self->avarMappings = NULL;
		self->avarCount = 0;
		if (!extractAxisDictValue(dict, kCTFontVariationAxisMinimumValueKey, &(self->min)))
			goto fail;
		if (!extractAxisDictValue(dict, kCTFontVariationAxisMaximumValueKey, &(self->max)))
			goto fail;
		if (!extractAxisDictValue(dict, kCTFontVariationAxisDefaultValueKey, &(self->def)))
			goto fail;
		if (table != NULL)
			self->avarMappings = avarExtract(table, i, &(self->avarCount));
	}
	return self;

fail:
	[self release];
	return nil;
}

- (void)dealloc
{
	if (self->avarMappings != NULL) {
		uiprivFree(self->avarMappings);
		self->avarMappings = NULL;
	}
	[super dealloc];
}

- (double)normalize:(double)d
{
	fixed1616 n;
	fixed214 n2;

	n = doubleToFixed1616(d);
	n = fixed1616Normalize(n, self->min, self->max, self->def);
	n2 = normalizedTo214(n, self->avarMappings, self->avarCount);
	return fixed214ToDouble(n2);
}

@end

NSDictionary *uiprivMakeVariationAxisDict(CFArrayRef axes, CFDataRef avarTable)
{
	CFDictionaryRef axis;
	CFIndex i, n;
	NSMutableDictionary *out;

	n = CFArrayGetCount(axes);
	out = [NSMutableDictionary new];
	for (i = 0; i < n; i++) {
		CFNumberRef key;

		axis = (CFDictionaryRef) CFArrayGetValueAtIndex(axes, i);
		key = (CFNumberRef) CFDictionaryGetValue(axis, kCTFontVariationAxisIdentifierKey);
		[out setObject:[[fvarAxis alloc] initWithIndex:i dict:axis avarTable:avarTable]
			forKey:((NSNumber *) key)];
	}
	if (avarTable != NULL)
		CFRelease(avarTable);
	return out;
}

#define fvarAxisKey(n) [NSNumber numberWithUnsignedInteger:n]

static BOOL tryAxis(NSDictionary *axisDict, CFDictionaryRef var, NSNumber *key, double *out)
{
	fvarAxis *axis;
	CFNumberRef num;

	axis = (fvarAxis *) [axisDict objectForKey:key];
	if (axis == nil)
		return NO;
	num = (CFNumberRef) CFDictionaryGetValue(var, (CFNumberRef) key);
	if (num == nil)
		return NO;
	if (CFNumberGetValue(num, kCFNumberDoubleType, out) == false) {
		// TODO
		return NO;
	}
	*out = [axis normalize:*out];
	return YES;
}

void uiprivProcessFontVariation(uiprivFontStyleData *d, NSDictionary *axisDict, uiFontDescriptor *out)
{
	CFDictionaryRef var;
	double v;

	out->Weight = uiTextWeightNormal;
	out->Stretch = uiTextStretchNormal;

	var = [d variation];

	if (tryAxis(axisDict, var, fvarAxisKey(fvarWeight), &v)) {
		// v is now a value between -1 and 1 scaled linearly between discrete points
		// we want a linear value between 0 and 1000 with 400 being normal
		if (v < 0) {
			v += 1;
			out->Weight = (uiTextWeight) (v * 400);
		} else if (v > 0)
			out->Weight += (uiTextWeight) (v * 600);
	}

	if (tryAxis(axisDict, var, fvarAxisKey(fvarWidth), &v)) {
		// likewise, but with stretches, we go from 0 to 8 with 4 being directly between the two, so this is sufficient
		v += 1;
		out->Stretch = (uiTextStretch) (v * 4);
	}
}
