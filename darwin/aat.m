// 14 february 2017
#import "uipriv_darwin.h"

static void boolspec(uiAttributeSpec *spec, uint16_t type, uint16_t ifTrue, uint16_t ifFalse, specToAATEnumFunc f, void *data)
{
	if (spec->Value != 0) {
		(*f)(type, ifTrue, data);
		return;
	}
	(*f)(type, ifFalse, data);
}

int specToAAT(uiAttributeSpec *spec, specToAATEnumFunc f, void *data)
{
	switch (spec->Type) {
	case uiAttributeStandardLigatures:
		boolspec(spec, kLigaturesType,
			kCommonLigaturesOnSelector,
			kCommonLigaturesOffSelector,
			f, data);
		return 1;
	case uiAttributeRequiredLigatures:
		boolspec(spec, kLigaturesType,
			kRequiredLigaturesOnSelector,
			kRequiredLigaturesOffSelector,
			f, data);
		return 1;
	case uiAttributeDiscretionaryLigatures:
		boolspec(spec, kLigaturesType,
			kRareLigaturesOnSelector,
			kRareLigaturesOffSelector,
			f, data);
		return 1;
	case uiAttributeContextualLigatures:
		boolspec(spec, kLigaturesType,
			kContextualLigaturesOnSelector,
			kContextualLigaturesOffSelector,
			f, data);
		return 1;
	case uiAttributeHistoricalLigatures:
		boolspec(spec, kLigaturesType,
			kHistoricalLigaturesOnSelector,
			kHistoricalLigaturesOffSelector,
			f, data);
		return 1;
	case uiAttributeUnicase:
		// TODO is this correct, or should we provide an else case?
		if (spec->Value != 0)
			// this is undocumented; it comes from Core Text's internal AAT-to-OpenType conversion table
			(*f)(kLetterCaseType, 14, data);
		return 1;
	// TODO make an array?
	case uiAttributeNumberSpacings:
		switch (spec->Value) {
		case uiAttributeNumberSpacingProportional:
			(*f)(kNumberSpacingType, kProportionalNumbersSelector, data);
			break;
		case uiAttributeNumberSpacingTabular:
			(*f)(kNumberSpacingType, kMonospacedNumbersSelector, data);
			break;
		}
		return 1;
	// TODO make an array?
	case uiAttributeSuperscripts:
		switch (spec->Value) {
		case uiAttributeSuperscriptNone:
			(*f)(kVerticalPositionType, kNormalPositionSelector, data);
			break;
		case uiAttributeSuperscriptSuperscript:
			(*f)(kVerticalPositionType, kSuperiorsSelector, data);
			break;
		case uiAttributeSuperscriptSubscript:
			(*f)(kVerticalPositionType, kInferiorsSelector, data);
			break;
		case uiAttributeSuperscriptOrdinal:
			(*f)(kVerticalPositionType, kOrdinalsSelector, data);
			break;
		case uiAttributeSuperscriptScientificInferior:
			(*f)(kVerticalPositionType, kScientificInferiorsSelector, data);
			break;
		}
		return 1;
	// TODO make an array?
	case uiAttributeFractionForms:
		switch (spec->Value) {
		case uiAttributeFractionFormNone:
			(*f)(kFractionsType, kNoFractionsSelector, data);
			break;
		case uiAttributeFractionFormVertical:
			(*f)(kFractionsType, kVerticalFractionsSelector, data);
			break;
		case uiAttributeFractionFormDiagonal:
			(*f)(kFractionsType, kDiagonalFractionsSelector, data);
			break;
		}
		return 1;
	case uiAttributeSlashedZero:
		boolspec(spec, kTypographicExtrasType,
			kSlashedZeroOnSelector,
			kSlashedZeroOffSelector,
			f, data);
		return 1;
	case uiAttributeMathematicalGreek:
		boolspec(spec, kMathematicalExtrasType,
			kMathematicalGreekOnSelector,
			kMathematicalGreekOffSelector,
			f, data);
		return 1;
	case uiAttributeOrnamentalForms:
		(*f)(kOrnamentSetsType, (uint16_t) (spec->Value), data);
		return 1;
	case uiAttributeSpecificCharacterForm:
		(*f)(kCharacterAlternativesType, (uint16_t) (spec->Value), data);
		return 1;
	case uiAttributeTitlingCapitalForms:
		// TODO is this correct, or should we provide an else case?
		if (spec->Value != 0)
			(*f)(kStyleOptionsType, kTitlingCapsSelector, data);
		return 1;
	// TODO make an array?
	case uiAttributeHanCharacterForms:
		switch (spec->Value) {
		case uiAttributeHanCharacterFormTraditional:
			(*f)(kCharacterShapeType, kTraditionalCharactersSelector, data);
			break;
		case uiAttributeHanCharacterFormSimplified:
			(*f)(kCharacterShapeType, kSimplifiedCharactersSelector, data);
			break;
		case uiAttributeHanCharacterFormJIS1978:
			(*f)(kCharacterShapeType, kJIS1978CharactersSelector, data);
			break;
		case uiAttributeHanCharacterFormJIS1983:
			(*f)(kCharacterShapeType, kJIS1983CharactersSelector, data);
			break;
		case uiAttributeHanCharacterFormJIS1990:
			(*f)(kCharacterShapeType, kJIS1990CharactersSelector, data);
			break;
		case uiAttributeHanCharacterFormExpert:
			(*f)(kCharacterShapeType, kExpertCharactersSelector, data);
			break;
		case uiAttributeHanCharacterFormJIS2004:
			(*f)(kCharacterShapeType, kJIS2004CharactersSelector, data);
			break;
		case uiAttributeHanCharacterFormHojo:
			(*f)(kCharacterShapeType, kHojoCharactersSelector, data);
			break;
		case uiAttributeHanCharacterFormNLC:
			(*f)(kCharacterShapeType, kNLCCharactersSelector, data);
			break;
		case uiAttributeHanCharacterFormTraditionalNames:
			(*f)(kCharacterShapeType, kTraditionalNamesCharactersSelector, data);
			break;
		}
		return 1;
	case uiAttributeLowercaseNumbers:
		// TODO is this correct, or should we provide an else case?
		if (spec->Value != 0)
			(*f)(kNumberCaseType, kLowerCaseNumbersSelector, data);
		return 1;
	case uiAttributeHanjaToHangul:
		// TODO is this correct, or should we provide an else case?
		if (spec->Value != 0)
			(*f)(kTransliterationType, kHanjaToHangulSelector, data);
		return 1;
	case uiAttributeAnnotatedGlyphForms:
		(*f)(kAnnotationType, (uint16_t) (spec->Value), data);
		return 1;
	case uiAttributeRubyKanaForms:
		// include this for completeness
		boolspec(spec, kRubyKanaType,
			kRubyKanaSelector,
			kNoRubyKanaSelector,
			f, data);
		// this is the current one
		boolspec(spec, kRubyKanaType,
			kRubyKanaOnSelector,
			kRubyKanaOffSelector,
			f, data);
		return 1;
	case uiAttributeCJKRomansToItalics:
		// include this for completeness
		boolspec(spec, kItalicCJKRomanType,
			kCJKItalicRomanSelector,
			kNoCJKItalicRomanSelector,
			f, data);
		// this is the current one
		boolspec(spec, kItalicCJKRomanType,
			kCJKItalicRomanOnSelector,
			kCJKItalicRomanOffSelector,
			f, data);
		return 1;
	case uiAttributeCaseSensitiveForms:
		boolspec(spec, kCaseSensitiveLayoutType,
			kCaseSensitiveLayoutOnSelector,
			kCaseSensitiveLayoutOffSelector,
			f, data);
		return 1;
	case uiAttributeCapitalSpacing:
		boolspec(spec, kCaseSensitiveLayoutType,
			kCaseSensitiveSpacingOnSelector,
			kCaseSensitiveSpacingOffSelector,
			f, data);
		return 1;
	case uiAttributeAlternateHorizontalKana:
		boolspec(spec, kAlternateKanaType,
			kAlternateHorizKanaOnSelector,
			kAlternateHorizKanaOffSelector,
			f, data);
		return 1;
	case uiAttributeAlternateVerticalKana:
		boolspec(spec, kAlternateKanaType,
			kAlternateVertKanaOnSelector,
			kAlternateVertKanaOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate1:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltOneOnSelector,
			kStylisticAltOneOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate2:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltTwoOnSelector,
			kStylisticAltTwoOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate3:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltThreeOnSelector,
			kStylisticAltThreeOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate4:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltFourOnSelector,
			kStylisticAltFourOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate5:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltFiveOnSelector,
			kStylisticAltFiveOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate6:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltSixOnSelector,
			kStylisticAltSixOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate7:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltSevenOnSelector,
			kStylisticAltSevenOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate8:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltEightOnSelector,
			kStylisticAltEightOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate9:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltNineOnSelector,
			kStylisticAltNineOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate10:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltTenOnSelector,
			kStylisticAltTenOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate11:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltElevenOnSelector,
			kStylisticAltElevenOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate12:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltTwelveOnSelector,
			kStylisticAltTwelveOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate13:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltThirteenOnSelector,
			kStylisticAltThirteenOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate14:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltFourteenOnSelector,
			kStylisticAltFourteenOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate15:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltFifteenOnSelector,
			kStylisticAltFifteenOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate16:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltSixteenOnSelector,
			kStylisticAltSixteenOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate17:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltSeventeenOnSelector,
			kStylisticAltSeventeenOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate18:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltEighteenOnSelector,
			kStylisticAltEighteenOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate19:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltNineteenOnSelector,
			kStylisticAltNineteenOffSelector,
			f, data);
		return 1;
	case uiAttributeStylisticAlternate20:
		boolspec(spec, kStylisticAlternativesType,
			kStylisticAltTwentyOnSelector,
			kStylisticAltTwentyOffSelector,
			f, data);
		return 1;
	case uiAttributeContextualAlternates:
		boolspec(spec, kContextualAlternatesType,
			kContextualAlternatesOnSelector,
			kContextualAlternatesOffSelector,
			f, data);
		return 1;
	case uiAttributeSwashes:
		boolspec(spec, kContextualAlternatesType,
			kSwashAlternatesOnSelector,
			kSwashAlternatesOffSelector,
			f, data);
		return 1;
	case uiAttributeContextualSwashes:
		boolspec(spec, kContextualAlternatesType,
			kContextualSwashAlternatesOnSelector,
			kContextualSwashAlternatesOffSelector,
			f, data);
		return 1;
	// TODO use arrays?
	case uiAttributeLowercaseCapForms:
		switch (spec->Value) {
		case uiAttributeCapFormNormal:
			(*f)(kLowerCaseType, kDefaultLowerCaseSelector, data);
			break;
		case uiAttributeCapFormSmallCaps:
			// include this for compatibility (some fonts that come with OS X still use this!)
			// TODO make it boolean?
			(*f)(kLetterCaseType, kSmallCapsSelector, data);
			// this is the current one
			(*f)(kLowerCaseType, kLowerCaseSmallCapsSelector, data);
			break;
		case uiAttributeCapFormPetiteCaps:
			(*f)(kLowerCaseType, kLowerCasePetiteCapsSelector, data);
			break;
		}
		return 1;
	// TODO use arrays?
	case uiAttributeUppercaseCapForms:
		switch (spec->Value) {
		case uiAttributeCapFormNormal:
			(*f)(kUpperCaseType, kDefaultUpperCaseSelector, data);
			break;
		case uiAttributeCapFormSmallCaps:
			(*f)(kUpperCaseType, kUpperCaseSmallCapsSelector, data);
			break;
		case uiAttributeCapFormPetiteCaps:
			(*f)(kUpperCaseType, kUpperCasePetiteCapsSelector, data);
			break;
		}
		return 1;
	}
	return 0;
}
