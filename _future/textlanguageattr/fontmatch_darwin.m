// note: this doesn't work for languages; we have to parse the ltag table

// fortunately features that aren't supported are simply ignored, so we can copy them all in
// LONGTERM FUTURE when we switch to 10.9, the language parameter won't be needed anymore
// LONGTERM FUTURE and on 10.10 we can use OpenType tags directly!
CTFontDescriptorRef fontdescAppendFeatures(CTFontDescriptorRef desc, const uint16_t *types, const uint16_t *selectors, size_t n, const char *language)
{
	CTFontDescriptorRef new;
	CFMutableArrayRef outerArray;
	CFDictionaryRef innerDict;
	CFNumberRef numType, numSelector;
	const void *keys[2], *values[2];
	size_t i;
	CFArrayRef languages;
	CFIndex il, nl;
	CFStringRef curlang;
	char d[2];

	outerArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	if (outerArray == NULL) {
		// TODO
	}
	keys[0] = kCTFontFeatureTypeIdentifierKey;
	keys[1] = kCTFontFeatureSelectorIdentifierKey;
	for (i = 0; i < n; i++) {
		numType = CFNumberCreate(NULL, kCFNumberSInt16Type,
			(const SInt16 *) (types + i));
		numSelector = CFNumberCreate(NULL, kCFNumberSInt16Type,
			(const SInt16 *) (selectors + i));
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
	}

	// now we have to take care of the language
	if (language != NULL) {
		languages = CTFontDescriptorCopyAttribute(desc, kCTFontLanguagesAttribute);
		if (languages != NULL) {
			nl = CFArrayGetCount(languages);
			d[0] = language[0];
			if (d[0] >= 'A' && d[0] <= 'Z')
				d[0] += 'a' - 'A';
			d[1] = language[1];
			if (d[1] >= 'A' && d[1] <= 'Z')
				d[1] += 'a' - 'A';
			for (il = 0; il < nl; il++) {
				char c[2];
		
				curlang = (CFStringRef) CFArrayGetValueAtIndex(languages, il);
				// TODO check for failure
				CFStringGetBytes(curlang, CFRangeMake(0, 2),
					kCFStringEncodingUTF8, 0, false,
					(UInt8 *) c, 2, NULL);
				if (c[0] >= 'A' && c[0] <= 'Z')
					c[0] += 'a' - 'A';
				if (c[1] >= 'A' && c[1] <= 'Z')
					c[1] += 'a' - 'A';
				if (c[0] == d[0] && c[1] == d[1])
					break;
			}
			if (il != nl) {
				uint16_t typ;
		
				typ = kLanguageTagType;
				il++;
				numType = CFNumberCreate(NULL, kCFNumberSInt16Type,
					(const SInt16 *) (&typ));
				numSelector = CFNumberCreate(NULL, kCFNumberCFIndexType,
					&il);
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
			}
			CFRelease(languages);
		}
	}

	keys[0] = kCTFontFeatureSettingsAttribute;
	values[0] = outerArray;
	innerDict = CFDictionaryCreate(NULL,
		keys, values, 1,
		// TODO are these correct?
		&kCFCopyStringDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	CFRelease(outerArray);
	new = CTFontDescriptorCreateCopyWithAttributes(desc, innerDict);
	CFRelease(desc);
	CFRelease(innerDict);
	return new;
}
