// 14 february 2017
#import "uipriv_darwin.h"

struct openTypeAATParams {
	void (*doAAT)(uint16_t type, uint16_t selector, void *data);
	void *data;
};

#define pcall(p, type, selector) ((*(p->doAAT))(type, selector, p->data))

static void boolspec(uint32_t value, uint16_t type, uint16_t ifTrue, uint16_t ifFalse, struct openTypeAATParams *p)
{
	// TODO are values other than 1 accepted for true by OpenType itself? (same for the rest of the file)
	if (value != 0) {
		pcall(p, type, ifTrue);
		return;
	}
	pcall(p, type, ifFalse);
}

static int foreach(char a, char b, char c, char d, uint32_t value, void *data)
{
	struct openTypeAATParams *p = (struct openTypeAATParams *) data;

	switch (mkTag(a, b, c, d)) {
	case mkTag('l', 'i', 'g', 'a'):
		boolspec(value, kLigaturesType,
			kCommonLigaturesOnSelector,
			kCommonLigaturesOffSelector,
			p);
		break;
	case mkTag('r', 'l', 'i', 'g'):
		boolspec(value, kLigaturesType,
			kRequiredLigaturesOnSelector,
			kRequiredLigaturesOffSelector,
			p);
		break;
	case mkTag('d', 'l', 'i', 'g'):
		boolspec(value, kLigaturesType,
			kRareLigaturesOnSelector,
			kRareLigaturesOffSelector,
			p);
		break;
	case mkTag('c', 'l', 'i', 'g'):
		boolspec(value, kLigaturesType,
			kContextualLigaturesOnSelector,
			kContextualLigaturesOffSelector,
			p);
		break;
	case mkTag('h', 'l', 'i', 'g'):
	// This technically isn't what is meant by "historical ligatures", but Core Text's internal AAT-to-OpenType mapping says to include it, so we include it too
	case mkTag('h', 'i', 's', 't'):
		boolspec(value, kLigaturesType,
			kHistoricalLigaturesOnSelector,
			kHistoricalLigaturesOffSelector,
			p);
		break;
	case mkTag('u', 'n', 'i', 'c'):
		// TODO is this correct, or should we provide an else case?
		if (value != 0)
			// this is undocumented; it comes from Core Text's internal AAT-to-OpenType conversion table
			pcall(p, kLetterCaseType, 14);
		break;

	// TODO will the following handle all cases properly, or are elses going to be needed?
	case mkTag('p', 'n', 'u', 'm'):
		if (value != 0)
			pcall(p, kNumberSpacingType, kProportionalNumbersSelector);
		break;
	case mkTag('t', 'n', 'u', 'm'):
		if (value != 0)
			pcall(p, kNumberSpacingType, kMonospacedNumbersSelector);
		break;

	// TODO will the following handle all cases properly, or are elses going to be needed?
	case mkTag('s', 'u', 'p', 's'):
		if (value != 0)
			pcall(p, kVerticalPositionType, kSuperiorsSelector);
		break;
	case mkTag('s', 'u', 'b', 's'):
		if (value != 0)
			pcall(p, kVerticalPositionType, kInferiorsSelector);
		break;
	case mkTag('o', 'r', 'd', 'n'):
		if (value != 0)
			pcall(p, kVerticalPositionType, kOrdinalsSelector);
		break;
	case mkTag('s', 'i', 'n', 'f'):
		if (value != 0)
			pcall(p, kVerticalPositionType, kScientificInferiorsSelector);
		break;

	// TODO will the following handle all cases properly, or are elses going to be needed?
	case mkTag('a', 'f', 'r', 'c'):
		if (value != 0)
			pcall(p, kFractionsType, kVerticalFractionsSelector);
		break;
	case mkTag('f', 'r', 'a', 'c'):
		if (value != 0)
			pcall(p, kFractionsType, kDiagonalFractionsSelector);
		break;

	case mkTag('z', 'e', 'r', 'o'):
		boolspec(value, kTypographicExtrasType,
			kSlashedZeroOnSelector,
			kSlashedZeroOffSelector,
			p);
		break;
	case mkTag('m', 'g', 'r', 'k'):
		boolspec(value, kMathematicalExtrasType,
			kMathematicalGreekOnSelector,
			kMathematicalGreekOffSelector,
			p);
		break;
	case mkTag('o', 'r', 'n', 'm'):
		pcall(p, kOrnamentSetsType, (uint16_t) value);
		break;
	case mkTag('a', 'a', 'l', 't'):
		pcall(p, kCharacterAlternativesType, (uint16_t) value);
		break;
	case mkTag('t', 'i', 't', 'l'):
		// TODO is this correct, or should we provide an else case?
		if (spec->Value != 0)
			pcall(p, kStyleOptionsType, kTitlingCapsSelector);
		break;

	// TODO will the following handle all cases properly, or are elses going to be needed?
	case mkTag('t', 'r', 'a', 'd'):
		if (value != 0)
			pcall(p, kCharacterShapeType, kTraditionalCharactersSelector);
		break;
	case mkTag('s', 'm', 'p', 'l'):
		if (value != 0)
			pcall(p, kCharacterShapeType, kSimplifiedCharactersSelector);
		break;
	case mkTag('j', 'p', '7', '8'):
		if (value != 0)
			pcall(p, kCharacterShapeType, kJIS1978CharactersSelector);
		break;
	case mkTag('j', 'p', '8', '3'):
		if (value != 0)
			pcall(p, kCharacterShapeType, kJIS1983CharactersSelector);
		break;
	case mkTag('j', 'p', '9', '0'):
		if (value != 0)
			pcall(p, kCharacterShapeType, kJIS1990CharactersSelector);
		break;
	case mkTag('e', 'x', 'p', 't'):
		if (value != 0)
			pcall(p, kCharacterShapeType, kExpertCharactersSelector);
		break;
	case mkTag('j', 'p', '0', '4'):
		if (value != 0)
			pcall(p, kCharacterShapeType, kJIS2004CharactersSelector);
		break;
	case mkTag('h', 'o', 'j', 'o'):
		if (value != 0)
			pcall(p, kCharacterShapeType, kHojoCharactersSelector);
		break;
	case mkTag('n', 'l', 'c', 'k'):
		if (value != 0)
			pcall(p, kCharacterShapeType, kNLCCharactersSelector);
		break;
	case mkTag('t', 'n', 'a', 'm'):
		if (value != 0)
			pcall(p, kCharacterShapeType, kTraditionalNamesCharactersSelector);
		break;

	case mkTag('o', 'n', 'u', 'm'):
	// Core Text's internal AAT-to-OpenType mapping says to include this, so we include it too
	// TODO is it always set?
	case mkTag('l', 'n', 'u', 'm'):
		// TODO is this correct, or should we provide an else case?
		if (value != 0)
			pcall(p, kNumberCaseType, kLowerCaseNumbersSelector);
		break;
	case mkTag('h', 'n', 'g', 'l'):
		// TODO is this correct, or should we provide an else case?
		if (value != 0)
			pcall(p, kTransliterationType, kHanjaToHangulSelector);
		break;
	case mkTag('n', 'a', 'l', 't'):
		pcall(p, kAnnotationType, (uint16_t) value);
		break;
	case mkTag('r', 'u', 'b', 'y'):
		// include this for completeness
		boolspec(value, kRubyKanaType,
			kRubyKanaSelector,
			kNoRubyKanaSelector,
			p);
		// this is the current one
		boolspec(value, kRubyKanaType,
			kRubyKanaOnSelector,
			kRubyKanaOffSelector,
			p);
		break;
	case mkTag('i', 't', 'a', 'l'):
		// include this for completeness
		boolspec(value, kItalicCJKRomanType,
			kCJKItalicRomanSelector,
			kNoCJKItalicRomanSelector,
			p);
		// this is the current one
		boolspec(value, kItalicCJKRomanType,
			kCJKItalicRomanOnSelector,
			kCJKItalicRomanOffSelector,
			p);
		break;
	case mkTag('c', 'a', 's', 'e'):
		boolspec(value, kCaseSensitiveLayoutType,
			kCaseSensitiveLayoutOnSelector,
			kCaseSensitiveLayoutOffSelector,
			p);
		break;
	case mkTag('c', 'p', 's', 'p'):
		boolspec(value, kCaseSensitiveLayoutType,
			kCaseSensitiveSpacingOnSelector,
			kCaseSensitiveSpacingOffSelector,
			p);
		break;
	case mkTag('h', 'k', 'n', 'a'):
		boolspec(value, kAlternateKanaType,
			kAlternateHorizKanaOnSelector,
			kAlternateHorizKanaOffSelector,
			p);
		break;
	case mkTag('v', 'k', 'n', 'a'):
		boolspec(value, kAlternateKanaType,
			kAlternateVertKanaOnSelector,
			kAlternateVertKanaOffSelector,
			p);
		break;
	case mkTag('s', 's', '0', '1'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltOneOnSelector,
			kStylisticAltOneOffSelector,
			p);
		break;
	case mkTag('s', 's', '0', '2'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltTwoOnSelector,
			kStylisticAltTwoOffSelector,
			p);
		break;
	case mkTag('s', 's', '0', '3'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltThreeOnSelector,
			kStylisticAltThreeOffSelector,
			p);
		break;
	case mkTag('s', 's', '0', '4'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltFourOnSelector,
			kStylisticAltFourOffSelector,
			p);
		break;
	case mkTag('s', 's', '0', '5'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltFiveOnSelector,
			kStylisticAltFiveOffSelector,
			p);
		break;
	case mkTag('s', 's', '0', '6'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltSixOnSelector,
			kStylisticAltSixOffSelector,
			p);
		break;
	case mkTag('s', 's', '0', '7'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltSevenOnSelector,
			kStylisticAltSevenOffSelector,
			p);
		break;
	case mkTag('s', 's', '0', '8'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltEightOnSelector,
			kStylisticAltEightOffSelector,
			p);
		break;
	case mkTag('s', 's', '0', '9'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltNineOnSelector,
			kStylisticAltNineOffSelector,
			p);
		break;
	case mkTag('s', 's', '1', '0'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltTenOnSelector,
			kStylisticAltTenOffSelector,
			p);
		break;
	case mkTag('s', 's', '1', '1'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltElevenOnSelector,
			kStylisticAltElevenOffSelector,
			p);
		break;
	case mkTag('s', 's', '1', '2'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltTwelveOnSelector,
			kStylisticAltTwelveOffSelector,
			p);
		break;
	case mkTag('s', 's', '1', '3'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltThirteenOnSelector,
			kStylisticAltThirteenOffSelector,
			p);
		break;
	case mkTag('s', 's', '1', '4'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltFourteenOnSelector,
			kStylisticAltFourteenOffSelector,
			p);
		break;
	case mkTag('s', 's', '1', '5'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltFifteenOnSelector,
			kStylisticAltFifteenOffSelector,
			p);
		break;
	case mkTag('s', 's', '1', '6'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltSixteenOnSelector,
			kStylisticAltSixteenOffSelector,
			p);
		break;
	case mkTag('s', 's', '1', '7'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltSeventeenOnSelector,
			kStylisticAltSeventeenOffSelector,
			p);
		break;
	case mkTag('s', 's', '1', '8'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltEighteenOnSelector,
			kStylisticAltEighteenOffSelector,
			p);
		break;
	case mkTag('s', 's', '1', '9'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltNineteenOnSelector,
			kStylisticAltNineteenOffSelector,
			p);
		break;
	case mkTag('s', 's', '2', '0'):
		boolspec(value, kStylisticAlternativesType,
			kStylisticAltTwentyOnSelector,
			kStylisticAltTwentyOffSelector,
			p);
		break;
	case mkTag('c', 'a', 'l', 't'):
		boolspec(value, kContextualAlternatesType,
			kContextualAlternatesOnSelector,
			kContextualAlternatesOffSelector,
			p);
		break;
	case mkTag('s', 'w', 's', 'h'):
		boolspec(value, kContextualAlternatesType,
			kSwashAlternatesOnSelector,
			kSwashAlternatesOffSelector,
			p);
		break;
	case mkTag('c', 's', 'w', 'h'):
		boolspec(value, kContextualAlternatesType,
			kContextualSwashAlternatesOnSelector,
			kContextualSwashAlternatesOffSelector,
			p);
		break;

	// TODO will the following handle all cases properly, or are elses going to be needed?
	case mkTag('s', 'm', 'c', 'p'):
		if (value != 0) {
			// include this for compatibility (some fonts that come with OS X still use this!)
			// TODO make it boolean?
			pcall(p, kLetterCaseType, kSmallCapsSelector);
			// this is the current one
			pcall(p, kLowerCaseType, kLowerCaseSmallCapsSelector);
		}
		break;
	case mkTag('p', 'c', 'a', 'p'):
		if (value != 0)
			pcall(p, kLowerCaseType, kLowerCasePetiteCapsSelector);
		break;

	// TODO will the following handle all cases properly, or are elses going to be needed?
	case mkTag('c', '2', 's', 'c'):
		if (value != 0)
			pcall(p, kUpperCaseType, kUpperCaseSmallCapsSelector);
		break;
	case mkTag('c', '2', 'p', 'c'):
		if (value != 0)
			pcall(p, kUpperCaseType, kUpperCasePetiteCapsSelector);
		break;
	}
	// TODO handle this properly
	return 0;
}

void openTypeToAAT(uiOpenTypeFeatures *otf, void (*doAAT)(uint16_t type, uint16_t selector, void *data), void *data)
{
	struct openTypeAATParams p;

	p.doAAT = doAAT;
	p.data = data;
	uiOpenTypeFeaturesForEach(otf, foreach, &p);
}
