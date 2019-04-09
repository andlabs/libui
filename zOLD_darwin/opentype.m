// 11 may 2017
#import "uipriv_darwin.h"
#import "attrstr.h"

struct addCTFeatureEntryParams {
	CFMutableArrayRef array;
	const void *tagKey;
	BOOL tagIsNumber;
	CFNumberType tagType;
	const void *tagValue;
	const void *valueKey;
	CFNumberType valueType;
	const void *valueValue;
};

static void addCTFeatureEntry(struct addCTFeatureEntryParams *p)
{
	CFDictionaryRef featureDict;
	CFNumberRef tagNum, valueNum;
	const void *keys[2], *values[2];

	keys[0] = p->tagKey;
	tagNum = NULL;
	values[0] = p->tagValue;
	if (p->tagIsNumber) {
		tagNum = CFNumberCreate(NULL, p->tagType, p->tagValue);
		values[0] = tagNum;
	}

	keys[1] = p->valueKey;
	valueNum = CFNumberCreate(NULL, p->valueType, p->valueValue);
	values[1] = valueNum;

	featureDict = CFDictionaryCreate(NULL,
		keys, values, 2,
		// TODO are these correct?
		&kCFCopyStringDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	if (featureDict == NULL) {
		// TODO
	}
	CFArrayAppendValue(p->array, featureDict);

	CFRelease(featureDict);
	CFRelease(valueNum);
	if (p->tagIsNumber)
		CFRelease(tagNum);
}

static uiForEach otfArrayForEachAAT(const uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value, void *data)
{
	__block struct addCTFeatureEntryParams p;

	p.array = (CFMutableArrayRef) data;
	p.tagIsNumber = YES;
	uiprivOpenTypeToAAT(a, b, c, d, value, ^(uint16_t type, uint16_t selector) {
		p.tagKey = kCTFontFeatureTypeIdentifierKey;
		p.tagType = kCFNumberSInt16Type;
		p.tagValue = (const SInt16 *) (&type);
		p.valueKey = kCTFontFeatureSelectorIdentifierKey;
		p.valueType = kCFNumberSInt16Type;
		p.valueValue = (const SInt16 *) (&selector);
		addCTFeatureEntry(&p);
	});
	return uiForEachContinue;
}

// TODO find out which fonts differ in AAT small caps and test them with this
static uiForEach otfArrayForEachOT(const uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value, void *data)
{
	struct addCTFeatureEntryParams p;
	char tagcstr[5];
	CFStringRef tagstr;

	p.array = (CFMutableArrayRef) data;

	p.tagKey = *uiprivFUTURE_kCTFontOpenTypeFeatureTag;
	p.tagIsNumber = NO;
	tagcstr[0] = a;
	tagcstr[1] = b;
	tagcstr[2] = c;
	tagcstr[3] = d;
	tagcstr[4] = '\0';
	tagstr = CFStringCreateWithCString(NULL, tagcstr, kCFStringEncodingUTF8);
	if (tagstr == NULL) {
		// TODO
	}
	p.tagValue = tagstr;

	p.valueKey = *uiprivFUTURE_kCTFontOpenTypeFeatureValue;
	p.valueType = kCFNumberSInt32Type;
	p.valueValue = (const SInt32 *) (&value);
	addCTFeatureEntry(&p);

	CFRelease(tagstr);
	return uiForEachContinue;
}

CFArrayRef uiprivOpenTypeFeaturesToCTFeatures(const uiOpenTypeFeatures *otf)
{
	CFMutableArrayRef array;
	uiOpenTypeFeaturesForEachFunc f;

	array = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	if (array == NULL) {
		// TODO
	}
	f = otfArrayForEachAAT;
	if (uiprivFUTURE_kCTFontOpenTypeFeatureTag != NULL && uiprivFUTURE_kCTFontOpenTypeFeatureValue != NULL)
		f = otfArrayForEachOT;
	uiOpenTypeFeaturesForEach(otf, f, array);
	return array;
}
