struct fontParams {
	uiDrawFontDescriptor desc;
	uint16_t featureTypes[maxFeatures];
	uint16_t featureSelectors[maxFeatures];
	size_t nFeatures;
	const char *language;
};


	// locale identifiers are specified as BCP 47: https://developer.apple.com/reference/corefoundation/cflocale?language=objc
	case uiAttributeLanguage:
		// LONGTERM FUTURE when we move to 10.9, switch to using kCTLanguageAttributeName
		ensureFontInRange(p, start, end);
		adjustFontInRange(p, start, end, ^(struct fontParams *fp) {
			fp->language = (const char *) (spec->Value);
		});
		break;

	desc = fontdescAppendFeatures(desc, fp->featureTypes, fp->featureSelectors, fp->nFeatures, fp->language);
