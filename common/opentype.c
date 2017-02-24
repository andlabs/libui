// 14 february 2017
#include "../ui.h"
#include "uipriv.h"

// Notes:
// - Each tag should only appear in quotes once (including within comments); this allows automated tools to determine what we cover and don't cover

static void boolspec(uiAttributeSpec *spec, const char *featureTag, specToOpenTypeEnumFunc f, void *data)
{
	if (spec->Value != 0) {
		(*f)(featureTag, 1, data);
		return;
	}
	(*f)(featureTag, 0, data);
}

static void boolspecnot(uiAttributeSpec *spec, const char *featureTag, specToOpenTypeEnumFunc f, void *data)
{
	if (spec->Value == 0) {
		(*f)(featureTag, 1, data);
		return;
	}
	(*f)(featureTag, 0, data);
}

void specToOpenType(uiAttributeSpec *spec, specToOpenTypeEnumFunc f, void *data)
{
	switch (spec->Type) {
	case uiAttributeStandardLigatures:
		boolspec(spec, "liga", f, data);
		return;
	case uiAttributeRequiredLigatures:
		boolspec(spec, "rlig", f, data);
		return;
	case uiAttributeDiscretionaryLigatures:
		boolspec(spec, "dlig", f, data);
		return;
	case uiAttributeContextualLigatures:
		boolspec(spec, "clig", f, data);
		return;
	case uiAttributeHistoricalLigatures:
		boolspec(spec, "hlig", f, data);
		// This technically isn't what is meant by "historical ligatures", but Core Text's internal AAT-to-OpenType mapping says to include it, so we include it too
		boolspec(spec, "hist", f, data);
		return;
	case uiAttributeUnicase:
		boolspec(spec, "unic", f, data);
		return;
	// TODO is this correct or should we explicitly switch the rest off too?
	case uiAttributeNumberSpacings:
		// TODO does Core Text set both? do we?
		switch (spec->Value) {
		case uiAttributeNumberSpacingProportional:
			(*f)("pnum", 1, data);
			break;
		case uiAttributeNumberSpacingTabular:
			(*f)("tnum", 1, data);
			break;
		}
		return;
	case uiAttributeSuperscripts:
		switch (spec->Value) {
		case uiAttributeSuperscriptSuperscript:
			(*f)("sups", 1, data);
			break;
		case uiAttributeSuperscriptSubscript:
			(*f)("subs", 1, data);
			break;
		case uiAttributeSuperscriptOrdinal:
			(*f)("ordn", 1, data);
			break;
		case uiAttributeSuperscriptScientificInferior:
			(*f)("sinf", 1, data);
			break;
		}
		return;
	// TODO is this correct or should we explicitly switch the rest off too?
	case uiAttributeFractionForms:
		switch (spec->Value) {
		case uiAttributeFractionFormVertical:
			(*f)("afrc", 1, data);
			break;
		case uiAttributeFractionFormDiagonal:
			(*f)("frac", 1, data);
			break;
		}
		return;
	case uiAttributeSlashedZero:
		boolspec(spec, "zero", f, data);
		return;
	case uiAttributeMathematicalGreek:
		boolspec(spec, "mgrk", f, data);
		return;
	case uiAttributeOrnamentalForms:
		(*f)("ornm", (uint32_t) (spec->Value), data);
		return;
	case uiAttributeSpecificCharacterForm:
		(*f)("aalt", (uint32_t) (spec->Value), data);
		return;
	case uiAttributeTitlingCapitalForms:
		boolspec(spec, "titl", f, data);
		return;
	// TODO is this correct or should we explicitly switch the rest off too?
	case uiAttributeHanCharacterForms:
		switch (spec->Value) {
		case uiAttributeHanCharacterFormTraditional:
			(*f)("trad", 1, data);
			break;
		case uiAttributeHanCharacterFormSimplified:
			(*f)("smpl", 1, data);
			break;
		case uiAttributeHanCharacterFormJIS1978:
			(*f)("jp78", 1, data);
			break;
		case uiAttributeHanCharacterFormJIS1983:
			(*f)("jp83", 1, data);
			break;
		case uiAttributeHanCharacterFormJIS1990:
			(*f)("jp90", 1, data);
			break;
		case uiAttributeHanCharacterFormExpert:
			(*f)("expt", 1, data);
			break;
		case uiAttributeHanCharacterFormJIS2004:
			(*f)("jp04", 1, data);
			break;
		case uiAttributeHanCharacterFormHojo:
			(*f)("hojo", 1, data);
			break;
		case uiAttributeHanCharacterFormNLC:
			(*f)("nlck", 1, data);
			break;
		case uiAttributeHanCharacterFormTraditionalNames:
			(*f)("tnam", 1, data);
			break;
		}
		return;
	case uiAttributeLowercaseNumbers:
		boolspec(spec, "onum", f, data);
		// Core Text's internal AAT-to-OpenType mapping says to include this, so we include it too
		// TODO is it always set?
		boolspecnot(spec, "lnum", f, data);
		return;
	case uiAttributeHanjaToHangul:
		boolspec(spec, "hngl", f, data);
		return;
	case uiAttributeAnnotatedGlyphForms:
		(*f)("nalt", (uint32_t) (spec->Value), data);
		return;
	case uiAttributeRubyKanaForms:
		boolspec(spec, "ruby", f, data);
		return;
	case uiAttributeCJKRomansToItalics:
		boolspec(spec, "ital", f, data);
		return;
	case uiAttributeCaseSensitiveForms:
		boolspec(spec, "case", f, data);
		return;
	case uiAttributeCapitalSpacing:
		boolspec(spec, "cpsp", f, data);
		return;
	case uiAttributeAlternateHorizontalKana:
		boolspec(spec, "hkna", f, data);
		return;
	case uiAttributeAlternateVerticalKana:
		boolspec(spec, "vkna", f, data);
		return;
	case uiAttributeStylisticAlternative1:
		boolspec(spec, "ss01", f, data);
		return;
	case uiAttributeStylisticAlternative2:
		boolspec(spec, "ss02", f, data);
		return;
	case uiAttributeStylisticAlternative3:
		boolspec(spec, "ss03", f, data);
		return;
	case uiAttributeStylisticAlternative4:
		boolspec(spec, "ss04", f, data);
		return;
	case uiAttributeStylisticAlternative5:
		boolspec(spec, "ss05", f, data);
		return;
	case uiAttributeStylisticAlternative6:
		boolspec(spec, "ss06", f, data);
		return;
	case uiAttributeStylisticAlternative7:
		boolspec(spec, "ss07", f, data);
		return;
	case uiAttributeStylisticAlternative8:
		boolspec(spec, "ss08", f, data);
		return;
	case uiAttributeStylisticAlternative9:
		boolspec(spec, "ss09", f, data);
		return;
	case uiAttributeStylisticAlternative10:
		boolspec(spec, "ss10", f, data);
		return;
	case uiAttributeStylisticAlternative11:
		boolspec(spec, "ss11", f, data);
		return;
	case uiAttributeStylisticAlternative12:
		boolspec(spec, "ss12", f, data);
		return;
	case uiAttributeStylisticAlternative13:
		boolspec(spec, "ss13", f, data);
		return;
	case uiAttributeStylisticAlternative14:
		boolspec(spec, "ss14", f, data);
		return;
	case uiAttributeStylisticAlternative15:
		boolspec(spec, "ss15", f, data);
		return;
	case uiAttributeStylisticAlternative16:
		boolspec(spec, "ss16", f, data);
		return;
	case uiAttributeStylisticAlternative17:
		boolspec(spec, "ss17", f, data);
		return;
	case uiAttributeStylisticAlternative18:
		boolspec(spec, "ss18", f, data);
		return;
	case uiAttributeStylisticAlternative19:
		boolspec(spec, "ss19", f, data);
		return;
	case uiAttributeStylisticAlternative20:
		boolspec(spec, "ss20", f, data);
		return;
	case uiAttributeContextualAlternates:
		boolspec(spec, "calt", f, data);
		return;
	case uiAttributeSwashes:
		boolspec(spec, "swsh", f, data);
		return;
	case uiAttributeContextualSwashes:
		boolspec(spec, "cswh", f, data);
		return;
	// TODO is this correct or should we explicitly switch the rest off too?
	case uiAttributeLowercaseCapForms:
		switch (spec->Value) {
		case uiAttributeCapFormSmallCaps:
			(*f)("smcp", 1, data);
			break;
		case uiAttributeCapFormPetiteCaps:
			(*f)("pcap", 1, data);
			break;
		}
		return;
	// TODO is this correct or should we explicitly switch the rest off too?
	case uiAttributeUppercaseCapForms:
		switch (spec->Value) {
		case uiAttributeCapFormSmallCaps:
			(*f)("c2sc", 1, data);
			break;
		case uiAttributeCapFormPetiteCaps:
			(*f)("c2pc", 1, data);
			break;
		}
		return;
	}
}

// TODO missing that AAT uses directly:
// - pkna, pwid, fwid, hwid, twid, qwid, palt, valt, vpal, halt, vhal, kern, vkrn (CJK width control)
// missing that AAT knows about:
// - ccmp (compositions)
// - dnom, numr (fraction parts) — no AAT equivalent...
// - falt, jalt (Arabic support)
// 	- rclt (required contextual alternates)
// - lfbd, opbd, rtbd (optical bounds support)
// - locl (Cyrillic support)
// - ltra, ltrm, rtla, rtlm (bidi support)
// - mark, mkmk (mark positioning)
// - rand (random glyph selection candidates)
// - salt (stylistic alternatives)
// - size (sizing info)
//
// script-specific; core text and pango/harfbuzz use these automatically based on the language
// TODO if DirectWrite does too we can ignore them and just provide a language attribute (they all use BCP 47 syntax for language names)
// Tag	Core Text?	Harfbuzz?
// abvf	yes			yes
// abvm	yes			yes
// abvs	yes			TODO
// akhn	yes			yes
// blwf	yes			yes
// blwm	yes			yes
// blws	yes			TODO
// cjct	yes			yes
// curs	yes			yes
// dist	yes			yes
// falt	TODO		TODO
// fin2	yes			yes
// fin3	yes			yes
// fina	yes			yes
// half	yes			yes
// haln	yes			yes
// init	yes			yes
// isol	yes			yes
// jalt	TODO		TODO
// ljmo	yes			yes
// locl	TODO		all horz(!)
// med2	yes			yes
// medi	yes			yes
// mset	TODO		yes
// nukt	yes			yes
// pref	yes			yes
// pres	yes			yes
// pstf	yes			yes
// psts	yes			yes
// rclt	TODO		all horz(!)
// rkrf	yes			yes
// rphf	yes			yes
// tjmo	yes			yes
// vatu	yes			yes
// vjmo	yes			yes
