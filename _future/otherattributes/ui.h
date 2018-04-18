_UI_ENUM(uiAttribute) {
	uiAttributeFamily,
	uiAttributeSize,				// use Double
	uiAttributeWeight,
	uiAttributeItalic,
	uiAttributeStretch,
	uiAttributeColor,			// use R, G, B, A
	uiAttributeBackground,		// use R, G, B, A

	// TODO kerning amount
	// OS X: kCTKernAttributeName
	// 	> 0: farther (TODO from advance or standard kerning?)
	// 	== 0: no kerning
	// 	< 0: closer (TODO same)
	// 	undefined: standard kerning
	// Pango: pango_attr_letter_spacing_new()
	// 	parameter meaning unspecified
	// Windows: requires Platform Update, SetLetterSpacing()
	// 	parameter meaning unspecified

	uiAttributeUnderline,		// enum uiDrawUnderlineStyle
	// TODO what is the color in the case we don't specify it, black or the text color?
	uiAttributeUnderlineColor,	// enum uiDrawUnderlineColor

	// TODO kCTSuperscriptAttributeName vs below
	// all it does is set the below attribute so

	// TODO kCTBaselineClassAttributeName, kCTBaselineInfoAttributeName, kCTBaselineReferenceInfoAttributeName

	// TODO strikethroughs? (pango yes, directwrite yes, os x no)
	// TODO baseline offsets? (pango yes)
	// TODO size scales? (pango yes)
	// TODO fallbacks (pango: enable or disable)

	// TODO document that this will also enable language-specific font features (TODO on DirectWrite too?)
	// TODO document that this should be strict BCP 47 form (A-Z, a-z, 0-9, and -) for maximum compatibility
	uiAttributeLanguage,		// BCP 47 string

	// These attributes represent typographic features. Each feature
	// is a separate attribute, to make composition easier. The
	// availability of for each attribute are defined by the font; the
	// default values are defined by the font and/or by the OS.
	// 
	// A note about features whose parameter is an enumeration:
	// OS X defines typographic features using the AAT specification
	// and converts to OpenType internally when needed, whereas
	// other platforms use OpenType directly. OpenType is less
	// precise about what each enumeration value means than AAT
	// is, so enumeration values do not necessarily represent what
	// OS X expects with all fonts. In cases where they do, libui
	// provides an enumeration type to use. Otherwise, the AAT
	// enumeration values are provided in comments for
	// documentation purposes.

	// TODO kAllTypographicFeaturesType

	// AAT calls these "common ligatures"
	uiAttributeStandardLigatures,		// 0 = off, 1 = on
	uiAttributeRequiredLigatures,		// 0 = off, 1 = on
	// AAT calls these "rare ligatures"
	uiAttributeDiscretionaryLigatures,	// 0 = off, 1 = on
	uiAttributeContextualLigatures,	// 0 = off, 1 = on
	uiAttributeHistoricalLigatures,		// 0 = off, 1 = on

	// TODO uiAttributeCursiveConnection,		// 0 = none, 1 = some, 2 = all

	uiAttributeUnicase,				// 0 = off, 1 = on

	// TODO uiAttributeLinguisticRearrangement,	// 0 = off, 1 = on

	// TODO rename this
	uiAttributeNumberSpacings,		// enum uiAttributeNumberSpacing

	// TODO kSmartSwashType, falt and jalt

	// TODO kDiacriticsType

	uiAttributeSuperscripts,			// enum uiAttributeSuperscript

	uiAttributeFractionForms,			// enum uiAttributeFractionForm

	uiAttributeSlashedZero,			// 0 = off, 1 = on

	uiAttributeMathematicalGreek,		// 0 = off, 1 = on

	// AAT defines the following values:
	// 0 = none
	// 1 = dingbats
	// 2 = pi characters
	// 3 = fleurons
	// 4 = decorative borders
	// 5 = international symbols
	// 6 = mathematical symbols
	// OpenType says alphanumeric characters must(? TODO) have one form each and the bullet character U+2022 (•) can have many
	uiAttributeOrnamentalForms,		// an integer from 0 to a font-specified upper bound
	// TODO provide a function to get the upper bound?

	// AAT calls this "character alternatives" and defines the
	// following values:
	// 0 = none
	// OpenType calls this "access all alternates".
	// TODO doesn't OpenType do the same about 0?
	uiAttributeSpecificCharacterForm,	// an integer from 0 to a font-specified upper bound
	// TODO provide a function to get the upper bound?

	uiAttributeTitlingCapitalForms,		// 0 = off, 1 = on

	// AAT calls these "character shapes"
	uiAttributeHanCharacterForms,	// enum uiAttributeHanCharacterForm

	// OpenType calls these "old-style"
	uiAttributeLowercaseNumbers,	// 0 = off, 1 = on

	// TODO kTextSpacingType
	// see kKanaSpacingType below

	uiAttributeHanjaToHangul,		// 0 = off, 1 = on

	// AAT defines the following values:
	// 0 = none
	// 1 = box
	// 2 = rounded box
	// 3 = circle
	// 4 = inverted circle
	// 5 = parentheses
	// 6 = period
	// 7 = roman numeral
	// 8 = diamond
	// 9 = inverted box
	// 10 = inverted rounded box
	// TODO rename to AnnotatedForms?
	uiAttributeAnnotatedGlyphForms,		// an integer from 0 to a font-specified upper bound
	// TODO provide a function to get the upper bound?

	// TODO kKanaSpacingType
	// TODO kIdeographicSpacingType
	// can they be provided independently of kTextSpacingType? Core Text doesn't seem to

	// TODO kUnicodeDecompositionType

	uiAttributeRubyKanaForms,		// 0 = off, 1 = on

	// TODO kCJKVerticalRomanPlacementType
	// this is 'valt' in OpenType but I don't know if I want to make it selectable or not

	uiAttributeCJKRomansToItalics,	// 0 = off, 1 = on

	// AAT calls this "case-sensitive layout"
	uiAttributeCaseSensitiveForms,	// 0 = off, 1 = on
	// AAT: this is called "case-sensitive spacing"
	uiAttributeCapitalSpacing,		// 0 = off, 1 = on

	uiAttributeAlternateHorizontalKana,		// 0 = off, 1 = on
	uiAttributeAlternateVerticalKana,	// 0 = off, 1 = on

	// TODO "Alternate"? unify all this
	// TODO document that these are guaranteed to be consecutive
	uiAttributeStylisticAlternate1,		// 0 = off, 1 = on
	uiAttributeStylisticAlternate2,		// 0 = off, 1 = on
	uiAttributeStylisticAlternate3,		// 0 = off, 1 = on
	uiAttributeStylisticAlternate4,		// 0 = off, 1 = on
	uiAttributeStylisticAlternate5,		// 0 = off, 1 = on
	uiAttributeStylisticAlternate6,		// 0 = off, 1 = on
	uiAttributeStylisticAlternate7,		// 0 = off, 1 = on
	uiAttributeStylisticAlternate8,		// 0 = off, 1 = on
	uiAttributeStylisticAlternate9,		// 0 = off, 1 = on
	uiAttributeStylisticAlternate10,	// 0 = off, 1 = on
	uiAttributeStylisticAlternate11,	// 0 = off, 1 = on
	uiAttributeStylisticAlternate12,	// 0 = off, 1 = on
	uiAttributeStylisticAlternate13,	// 0 = off, 1 = on
	uiAttributeStylisticAlternate14,	// 0 = off, 1 = on
	uiAttributeStylisticAlternate15,	// 0 = off, 1 = on
	uiAttributeStylisticAlternate16,	// 0 = off, 1 = on
	uiAttributeStylisticAlternate17,	// 0 = off, 1 = on
	uiAttributeStylisticAlternate18,	// 0 = off, 1 = on
	uiAttributeStylisticAlternate19,	// 0 = off, 1 = on
	uiAttributeStylisticAlternate20,	// 0 = off, 1 = on

	uiAttributeContextualAlternates,	// 0 = off, 1 = on
	uiAttributeSwashes,				// 0 = off, 1 = on
	uiAttributeContextualSwashes,		// 0 = off, 1 = on

	uiAttributeLowercaseCapForms,	// enum uiAttributeCapForm
	uiAttributeUppercaseCapForms,	// enum uiAttributeCapForm

	// TODO kCJKRomanSpacingType

	// TODO uiAttributeSystem, (this might not be doable with DirectWrite)
	// TODO uiAttributeCustom,
};
