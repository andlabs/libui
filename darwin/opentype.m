// 11 may 2017
#import "uipriv_darwin.h"

struct uiOpenTypeFeatures {
	NSMutableDictionary *tags;
};

uiOpenTypeFeatures *uiNewOpenTypeFeatures(void)
{
	uiOpenTypeFeatures *otf;

	otf = uiNew(uiOpenTypeFeatures);
	otf->tags = [NSMutableDictionary new];
	return otf;
}

void uiFreeOpenTypeFeatures(uiOpenTypeFeatures *otf)
{
	[otf->tags release];
	uiFree(otf);
}

uiOpenTypeFeatures *uiOpenTypeFeaturesClone(const uiOpenTypeFeatures *otf)
{
	uiOpenTypeFeatures *out;

	out = uiNew(uiOpenTypeFeatures);
	out->tags = [otf->tags mutableCopy];
	return out;
}

// why are there no NSNumber methods for stdint.h or the equivalent core foundation types?...
#define mkMapObject(tag) [NSNumber numberWithUnsignedLongLong:((unsigned long long) tag)]
#define mapObjectValue(num) ((uint32_t) [num unsignedLongLongValue])

void uiOpenTypeFeaturesAdd(uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value)
{
	NSNumber *tn, *vn;

	tn = mkMapObject(mkTag(a, b, c, d));
	vn = mkMapObject(value);
	[otf->tags setObject:vn forKey:tn];
}

void uiOpenTypeFeaturesRemove(uiOpenTypeFeatures *otf, char a, char b, char c, char d)
{
	NSNumber *tn;

	tn = mkMapObject(mkTag(a, b, c, d));
	// documented as doing nothing if tn is not in otf->tags
	[otf->tags removeObjectForKey:tn];
}

// TODO will the const wreck stuff up?
int uiOpenTypeFeaturesGet(const uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t *value)
{
	NSNumber *tn, *vn;

	tn = mkMapObject(mkTag(a, b, c, d));
	vn = (NSNumber *) [otf->tags objectForKey:tn];
	if (vn == nil)
		return 0;
	*value = mapObjectValue(vn);
	// TODO release vn?
	return 1;
}

void uiOpenTypeFeaturesForEach(const uiOpenTypeFeatures *otf, uiOpenTypeFeaturesForEachFunc f, void *data)
{
	[otf->tags enumerateKeysAndObjectsUsingBlock:^(id key, id value, BOOL *stop) {
		NSNumber *tn = (NSNumber *) key;
		NSNumber *vn = (NSNumber *) value;
		uint32_t tag;
		uint8_t a, b, c, d;
		uiForEach ret;

		tag = mapObjectValue(tn);
		a = (uint8_t) ((tag >> 24) & 0xFF);
		b = (uint8_t) ((tag >> 16) & 0xFF);
		c = (uint8_t) ((tag >> 8) & 0xFF);
		d = (uint8_t) (tag & 0xFF);
		ret = (*f)(otf, (char) a, (char) b, (char) c, (char) d,
			mapObjectValue(vn), data);
		// TODO for all: require exact match?
		if (ret == uiForEachStop)
			*stop = YES;
	}];
}

int uiOpenTypeFeaturesEqual(const uiOpenTypeFeatures *a, const uiOpenTypeFeatures *b)
{
	if (a == NULL && b == NULL)
		return 1;
	if (a == NULL || b == NULL)
		return 0;
	return [a->tags isEqualToDictionary:b->tags];
}

// TODO explain all this
// TODO rename outerArray and innerDict (the names made sense when this was part of fontdescAppendFeatures(), but not here)
// TODO make all this use enumerateKeysAndObjects (which requires duplicating code)?
static uiForEach otfArrayForEachAAT(const uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value, void *data)
{
	CFMutableArrayRef outerArray = (CFMutableArrayRef) data;

	openTypeToAAT(a, b, c, d, value, ^(uint16_t type, uint16_t selector) {
		CFDictionaryRef innerDict;
		CFNumberRef numType, numSelector;
		// not well documented, but fixed-size arrays don't support __block either (VLAs are documented as being unsupported)
		const void *keys[2], *values[2];

		keys[0] = kCTFontFeatureTypeIdentifierKey;
		keys[1] = kCTFontFeatureSelectorIdentifierKey;
		numType = CFNumberCreate(NULL, kCFNumberSInt16Type,
			(const SInt16 *) (&type));
		numSelector = CFNumberCreate(NULL, kCFNumberSInt16Type,
			(const SInt16 *) (&selector));
		values[0] = numType;
		values[1] = numSelector;
		innerDict = CFDictionaryCreate(NULL,
			keys, values, 2,
			// TODO are these correct?
			&kCFCopyStringDictionaryKeyCallBacks,
			&kCFTypeDictionaryValueCallBacks);
		if (innerDict == NULL) {
			// TODO
		}
		CFArrayAppendValue(outerArray, innerDict);
		CFRelease(innerDict);
		CFRelease(numSelector);
		CFRelease(numType);
	});
	return uiForEachContinue;
}

// TODO find out which fonts differ in AAT small caps and test them with this
static uiForEach otfArrayForEachOT(const uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value, void *data)
{
	CFMutableArrayRef outerArray = (CFMutableArrayRef) data;
	CFDictionaryRef innerDict;
	// TODO rename this to tagstr (and all the other variables likewise...)
	CFStringRef strTag;
	CFNumberRef numValue;
	char tagcstr[5];
	const void *keys[2], *values[2];

	tagcstr[0] = a;
	tagcstr[1] = b;
	tagcstr[2] = c;
	tagcstr[3] = d;
	tagcstr[4] = '\0';
	keys[0] = *FUTURE_kCTFontOpenTypeFeatureTag;
	keys[1] = *FUTURE_kCTFontOpenTypeFeatureValue;
	strTag = CFStringCreateWithCString(NULL, tagcstr, kCFStringEncodingUTF8);
	if (strTag == NULL) {
		// TODO
	}
	numValue = CFNumberCreate(NULL, kCFNumberSInt32Type,
		(const SInt32 *) (&value));
	values[0] = strTag;
	values[1] = numValue;
	innerDict = CFDictionaryCreate(NULL,
		keys, values, 2,
		// TODO are these correct?
		&kCFCopyStringDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	if (innerDict == NULL) {
		// TODO
	}
	CFArrayAppendValue(outerArray, innerDict);
	CFRelease(innerDict);
	CFRelease(numValue);
	CFRelease(strTag);
	return uiForEachContinue;
}

CFArrayRef otfToFeaturesArray(const uiOpenTypeFeatures *otf)
{
	CFMutableArrayRef outerArray;
	uiOpenTypeFeaturesForEachFunc f;

	outerArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	if (outerArray == NULL) {
		// TODO
	}
	f = otfArrayForEachAAT;
	if (FUTURE_kCTFontOpenTypeFeatureTag != NULL && FUTURE_kCTFontOpenTypeFeatureValue != NULL)
		f = otfArrayForEachOT;
	uiOpenTypeFeaturesForEach(otf, f, outerArray);
	return outerArray;
}
