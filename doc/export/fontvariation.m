// 2 november 2017
#import "uipriv_darwin.h"

// references:
// - https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6fvar.html
// - https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6avar.html
// - https://www.microsoft.com/typography/otspec/fvar.htm
// - https://www.microsoft.com/typography/otspec/otvaroverview.htm#CSN
// - https://www.microsoft.com/typography/otspec/otff.htm
// - https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6.html#Types
// - https://www.microsoft.com/typography/otspec/avar.htm

typedef uint32_t fixed1616;
typedef uint16_t fixed214;

static fixed1616 doubleToFixed1616(double d)
{
	double ipart, fpart;
	long flong;
	int16_t i16;
	uint32_t ret;

	fpart = fabs(modf(d, &ipart));
	fpart *= 65536;
	flong = lround(fpart);
	i16 = (int16_t) ipart;
	ret = (uint32_t) ((uint16_t) i16);
	ret <<= 16;
	ret |= (uint16_t) (flong & 0xFFFF);
	return (fixed1616) ret;
}

static double fixed1616ToDouble(fixed1616 f)
{
	int16_t base;
	double frac;

	base = (int16_t) ((f >> 16) & 0xFFFF);
	frac = ((double) (f & 0xFFFF)) / 65536;
	return ((double) base) + frac;
}

static fixed214 fixed1616ToFixed214(fixed1616 f)
{
	uint32_t t;
	uint32_t topbit;

	t = f + 0x00000002;
	topbit = t & 0x80000000;
	t >>= 2;
	if (topbit != 0)
		t |= 0xC000000;
	return (fixed214) (t & 0xFFFF);
}

static double fixed214ToDouble(fixed214 f)
{
	double base;
	double frac;

	switch ((f >> 14) & 0x3) {
	case 0:
		base = 0;
		break;
	case 1:
		base = 1;
		break;
	case 2:
		base = -2:
		break;
	case 3:
		base = -1;
	}
	frac = ((double) (f & 0x3FFF)) / 16384;
	return base + frac;
}

static fixed1616 fixed214ToFixed1616(fixed214 f)
{
	int32_t t;
	uint32_t x;

	t = (int32_t) ((int16_t) f);
	t <<= 2;
	x = (uint32_t) t;
	return (float1616) (x - 0x00000002);
}

static const fixed1616Negative1 = 0xFFFF0000;
static const fixed1616Zero = 0x00000000;
static const fixed1616Positive1 = 0x00010000;

static fixed1616 normalize1616(fixed1616 val, fixed1616 min, fixed1616 max, fixed1616 def)
{
	if (val < min)
		val = min;
	if (val > max)
		val = max;
	if (val < def)
		return -(def - val) / (def - min);
	if (val > def)
		return (val - def) / (max - def);
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
		float1616 startFrom, endFrom;
		float1616 startTo, endTo;

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
			val = (val - startFrom) / (endFrom - startFrom);
			val *= (endTo - startTo);
			val += startTo;
		}
	}
	return fixed1616ToFixed214(val);
}

fixed1616 *avarExtract(CFDataRef table, size_t index, size_t *n)
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
	entries = (fixed1616 *) uiAlloc(*n * sizeof (fixed1616), "fixed1616[]");
	for (i = 0; i < *n; i++) {
		*p++ = fixed214ToFixed1616((fixed214) nextuint16be());
		off += 2;
	}
	return entries;
}
