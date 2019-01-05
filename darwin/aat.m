// 14 february 2017
#import "uipriv_darwin.h"
#import "attrstr.h"

// TODO explain the purpose of this file

static void boolspec(uint32_t value, uint16_t type, uint16_t ifTrue, uint16_t ifFalse, uiprivAATBlock f)
{
	// TODO are values other than 1 accepted for true by OpenType itself? (same for the rest of the file)
	if (value != 0) {
		f(type, ifTrue);
		return;
	}
	f(type, ifFalse);
}

// TODO remove the need for this
// TODO remove x8tox32()
#define x8tox32(x) ((uint32_t) (((uint8_t) (x)) & 0xFF))
#define mkTag(a, b, c, d)		\
	((x8tox32(a) << 24) |	\
	(x8tox32(b) << 16) |		\
	(x8tox32(c) << 8) |		\
	x8tox32(d))

// TODO double-check drawtext example to make sure all of these are used properly (I already screwed dlig up by putting clig twice instead)
void uiprivOpenTypeToAAT(char a, char b, char c, char d, uint32_t value, uiprivAATBlock f)
{
	switch (mkTag(a, b, c, d)) {
	case mkTag('l', 'i', 'g', 'a'):
		boolspec(value, kLigaturesType,
			kCommonLigaturesOnSelector,
			kCommonLigaturesOffSelector,
			f);
		break;
	case mkTag('r', 'l', 'i', 'g'):
		boolspec(value, kLigaturesType,
			kRequiredLigaturesOnSelector,
			kRequiredLigaturesOffSelector,
			f);
		break;
	case mkTag('d', 'l', 'i', 'g'):
		boolspec(value, kLigaturesType,
			kRareLigaturesOnSelector,
			kRareLigaturesOffSelector,
			f);
		break;
	case mkTag('c', 'l', 'i', 'g'):
		boolspec(value, kLigaturesType,
			kContextualLigaturesOnSelector,
			kContextualLigaturesOffSelector,
			f);
		break;
	case mkTag('h', 'l', 'i', 'g'):
	// This technically isn't what is meant by "historical ligatures", but Core Text's internal AAT-to-OpenType mapping says to include it, so we include it too
	case mkTag('h', 'i', 's', 't'):
		boolspec(value, kLigaturesType,
			kHistoricalLigaturesOnSelector,
			kHistoricalLigaturesOffSelector,
			f);
		break;
	case mkTag('u', 'n', 'i', 'c'):
		// TODO is this correct, or should we provide an else case?
		if (value != 0)
			// this is undocumented; it comes from Core Text's internal AAT-to-OpenType conversion table
			f(kLetterCaseType, 14);
		break;

	// TODO will the following handle all cases properly, or are elses going to be needed?
	case mkTag('p', 'n', 'u', 'm'):
		if (value != 0)
			f(kNumberSpacingType, kProportionalNumbersSelector);
		break;
	case mkTag('t', 'n', 'u', 'm'):
		if (value != 0)
			f(kNumberSpacingType, kMonospacedNumbersSelector);
		break;

	// TODO will the following handle all cases properly, or are elses going to be needed?
	case mkTag('s', 'u', 'p', 's'):
		if (value != 0)
			f(kVerticalPositionType, kSuperiorsSelector);
		break;
	case mkTag('s', 'u', 'b', 's'):
		if (value != 0)
			f(kVerticalPositionType, kInferiorsSelector);
		break;
	case mkTag('o', 'r', 'd', 'n'):
		if (value != 0)
			f(kVerticalPositionType, kOrdinalsSelector);
		break;
	case mkTag('s', 'i', 'n', 'f'):
		if (value != 0)
			f(kVerticalPositionType, kScientificInferiorsSelector);
		break;

	// TODO will the following handle all cases properly, or are elses going to be needed?
	case mkTag('a', 'f', 'r', 'c'):
		if (value != 0)
			f(kFractionsType, kVerticalFractionsSelector);
		break;
	case mkTag('f', 'r', 'a', 'c'):
		if (value != 0)
			f(kFractionsType, kDiagonalFractionsSelector);
		break;

	case mkTag('z', 'e', 'r', 'o'):
		boolspec(value, kTypographicExtrasType,
			kSlashedZeroOnSelector,
			kSlashedZeroOffSelector,
			f);
		break;
	case mkTag('m', 'g', 'r', 'k'):
		boolspec(value, kMathematicalExtrasType,
			kMathematicalGreekOnSelector,
			kMathematicalGreekOffSelector,
			f);
		break;
	case mkTag('o', 'r', 'n', 'm'):
		f(kOrnamentSetsType, (uint16_t) value);
		break;
	case mkTag('a', 'a', 'l', 't'):
		f(kCharacterAlternativesType, (uint16_t) value);
		break;
	case mkTag('t', 'i', 't', 'l'):
		// TODO is this correct, or should we provide an else case?
		if (value != 0)
			f(kStyleOptionsType, kTitlingCapsSelector);
		break;

	// TODO will the following handle all cases properly, or are elses going to be needed?
	case mkTag('t', 'r', 'a', 'd'):
		if (value != 0)
			f(kCharacterShapeType, kTraditionalCharactersSelector);
		break;
	case mkTag('s', 'm', 'p', 'l'):
		if (value != 0)
			f(kCharacterShapeType, kSimplifiedCharactersSelector);
		break;
	case mkTag('j', 'p', '7', '8'):
		if (value != 0)
			f(kCharacterShapeType, kJIS1978CharactersSelector);
		break;
	case mkTag('j', 'p', '8', '3'):
		if (value != 0)
			f(kCharacterShapeType, kJIS1983CharactersSelector);
		break;
	case mkTag('j', 'p', '9', '0'):
		if (value != 0)
			f(kCharacterShapeType, kJIS1990CharactersSelector);
		break;
	case mkTag('e', 'x', 'p', 't'):
		if (value != 0)
			f(kCharacterShapeType, kExpertCharactersSelector);
		break;
	case mkTag('j', 'p', '0', '4'):
		if (value != 0)
			f(kCharacterShapeType, kJIS2004CharactersSelector);
		break;
	case mkTag('h', 'o', 'j', 'o'):
		if (value != 0)
			f(kCharacterShapeType, kHojoCharactersSelector);
		break;
	case mkTag('n', 'l', 'c', 'k'):
		if (value != 0)
			f(kCharacterShapeType, kNLCCharactersSelector);
		break;
	case mkTag('t', 'n', 'a', 'm'):
		if (value != 0)
			f(kCharacterShapeType, kTraditionalNamesCharactersSelector);
		break;

	case mkTag('o', 'n', 'u', 'm'):
	// Core Text's internal AAT-to-OpenType mapping says to include this, so we include it too
	// TODO is it always set?
	case mkTag('l', 'n', 'u', 'm'):
		// TODO is this correct, or should we provide an else case?
		if (value != 0)
			f(kNumberCaseType, kLowerCaseNumbersSelector);
		break;
	case mkTag('h', 'n', 'g', 'l'):
		// TODO is this correct, or should we provide an else case?
		if (value != 0)
			f(kTransliterationType, kHanjaToHangulSelector);
		break;
	case mkTag('n', 'a', 'l', 't'):
		f(kAnnotationType, (uint16_t) value);
		break;
	case mkTag('r', 'u', 'b', 'y'):
		// include this for completeness
		boolspec(value, kRubyKanaType,
			kRubyKanaSelector,
			kNoRubyKanaSelector,
			f);
		// this is the current one
		boolspec(value, kRubyKanaType,
			kRubyKanaOnSelector,
			kRubyKanaOffSelector,
			f);
		break;
	case mkTag('i', 't', 'a', 'l'):
		// include this for completeness
		boolspec(value, kItalicCJKRomanType,
			kCJKItalicRomanSelector,
			kNoCJKItalicRomanSelector,
			f);
		// this is the current one
		boolspec(value, kItalicCJKRomanType,
			kCJKItalicRomanOnSelector,
			kCJKItalicRomanOffSelector,
			f);
		break;
	case mkTag('c', 'a', 's', 'e'):
		boolspec(value, kCaseSensitiveLayoutType,
			kCaseSensitiveLayoutOnSelector,
			kCaseSensitiveLayoutOffSelector,
			f);
		break;
	case mkTag('c', 'p', 's', 'p'):
		boolspec(value, kCaseSensitiveLayoutType,
			kCaseSensitiveSpacingOnSelector,
			kCaseSensitiveSpacingOffSelector,
			f);
		break;
	case mkTag('h', 'k', 'n', 'a'):
		boolspec(value, kAlternateKanaType,
			kAlternateHorizKanaOnSelector,
			kAlternateHorizKanaOffSelector,
			f);
		break;
	case mkTag('v', 'k', 'n', 'a'):
		boolspec(value, kAlternateKanaType,
			kAlternateVertKanaOnSelector,
			kAlternateVertKanaOffSelector,
			f);
		break;
	case mkTag('s', 's', '0', '1'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltOneOnSelector,
			kStylisticAltOneOffSelector,
			f);
		break;
	case mkTag('s', 's', '0', '2'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltTwoOnSelector,
			kStylisticAltTwoOffSelector,
			f);
		break;
	case mkTag('s', 's', '0', '3'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltThreeOnSelector,
			kStylisticAltThreeOffSelector,
			f);
		break;
	case mkTag('s', 's', '0', '4'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltFourOnSelector,
			kStylisticAltFourOffSelector,
			f);
		break;
	case mkTag('s', 's', '0', '5'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltFiveOnSelector,
			kStylisticAltFiveOffSelector,
			f);
		break;
	case mkTag('s', 's', '0', '6'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltSixOnSelector,
			kStylisticAltSixOffSelector,
			f);
		break;
	case mkTag('s', 's', '0', '7'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltSevenOnSelector,
			kStylisticAltSevenOffSelector,
			f);
		break;
	case mkTag('s', 's', '0', '8'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltEightOnSelector,
			kStylisticAltEightOffSelector,
			f);
		break;
	case mkTag('s', 's', '0', '9'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltNineOnSelector,
			kStylisticAltNineOffSelector,
			f);
		break;
	case mkTag('s', 's', '1', '0'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltTenOnSelector,
			kStylisticAltTenOffSelector,
			f);
		break;
	case mkTag('s', 's', '1', '1'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltElevenOnSelector,
			kStylisticAltElevenOffSelector,
			f);
		break;
	case mkTag('s', 's', '1', '2'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltTwelveOnSelector,
			kStylisticAltTwelveOffSelector,
			f);
		break;
	case mkTag('s', 's', '1', '3'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltThirteenOnSelector,
			kStylisticAltThirteenOffSelector,
			f);
		break;
	case mkTag('s', 's', '1', '4'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltFourteenOnSelector,
			kStylisticAltFourteenOffSelector,
			f);
		break;
	case mkTag('s', 's', '1', '5'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltFifteenOnSelector,
			kStylisticAltFifteenOffSelector,
			f);
		break;
	case mkTag('s', 's', '1', '6'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltSixteenOnSelector,
			kStylisticAltSixteenOffSelector,
			f);
		break;
	case mkTag('s', 's', '1', '7'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltSeventeenOnSelector,
			kStylisticAltSeventeenOffSelector,
			f);
		break;
	case mkTag('s', 's', '1', '8'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltEighteenOnSelector,
			kStylisticAltEighteenOffSelector,
			f);
		break;
	case mkTag('s', 's', '1', '9'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltNineteenOnSelector,
			kStylisticAltNineteenOffSelector,
			f);
		break;
	case mkTag('s', 's', '2', '0'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltTwentyOnSelector,
			kStylisticAltTwentyOffSelector,
			f);
		break;
	case mkTag('c', 'a', 'l', 't'):
		boolspec(value, kContextualAlternatesType,
			kContextualAlternatesOnSelector,
			kContextualAlternatesOffSelector,
			f);
		break;
	case mkTag('s', 'w', 's', 'h'):
		boolspec(value, kContextualAlternatesType,
			kSwashAlternatesOnSelector,
			kSwashAlternatesOffSelector,
			f);
		break;
	case mkTag('c', 's', 'w', 'h'):
		boolspec(value, kContextualAlternatesType,
			kContextualSwashAlternatesOnSelector,
			kContextualSwashAlternatesOffSelector,
			f);
		break;

	// TODO will the following handle all cases properly, or are elses going to be needed?
	case mkTag('s', 'm', 'c', 'p'):
		if (value != 0) {
			// include this for compatibility (some fonts that come with OS X still use this!)
			// TODO make it boolean?
			f(kLetterCaseType, kSmallCapsSelector);
			// this is the current one
			f(kLowerCaseType, kLowerCaseSmallCapsSelector);
		}
		break;
	case mkTag('p', 'c', 'a', 'p'):
		if (value != 0)
			f(kLowerCaseType, kLowerCasePetiteCapsSelector);
		break;

	// TODO will the following handle all cases properly, or are elses going to be needed?
	case mkTag('c', '2', 's', 'c'):
		if (value != 0)
			f(kUpperCaseType, kUpperCaseSmallCapsSelector);
		break;
	case mkTag('c', '2', 'p', 'c'):
		if (value != 0)
			f(kUpperCaseType, kUpperCasePetiteCapsSelector);
		break;
	}
	// TODO handle this properly
	// (it used to return 0 when this still returned the number of selectors produced but IDK what properly is anymore)
}
