// 14 february 2017
#include "../ui.h"
#include "uipriv.h"

// note: each tag should only appear in quotes once; this allows automated tools to determine what we cover and don't cover

typedef void (*specToOpenTypeEnumFunc)(const char *featureTag, uint32_t param, void *data);

static void boolspec(uiAttributeSpec *spec, const char *featureTag, specToOpenTypeEnumFunc f, void *data)
{
	if (spec->Value != 0) {
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
		return;
	case uiAttributeUnicase:
		boolspec(spec, "unic", f, data);
		return;
	// TODO is this correct or should we explicitly switch the rest off too?
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
		boolspec(spec, "zero", data);
		return;
	case uiAttributeMathematicalGreek:
		boolspec(spec, "mgrk", data);
		return;
	case uiAttributeOrnamentalForms:
		(*f)("ornm", (uint32_t) (spec->Value), data);
		return;
	case uiAttributeTitlingCapitalForms:
		boolspec(spec, "titl", data);
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
		boolspec(spec, "onum", data);
		return;
	case uiAttributeGlyphAnnotations:
		(*f)("nalt", (uint32_t) (spec->Value), data);
		return;
	case uiAttributeCJKRomanToitalics:
		boolspec(spec, "ital", data);
		return;
	case uiAttributeCaseSensitiveForms:
		boolspec(spec, "case", data);
		return;
	case uiAttributeCapitalSpacing:
		boolspec(spec, "cpsp", data);
		return;
	case uiAttributeAlternateHorizontalKana:
		boolspec(spec, "hkna", data);
		return;
	case uiAttributeAlternateVerticalKana:
		boolspec(spec, "vkna", data);
		return;
	case uiAttributeStylisticAlternative1:
		boolspec(spec, "ss01", data);
		return;
	case uiAttributeStylisticAlternative2:
		boolspec(spec, "ss02", data);
		return;
	case uiAttributeStylisticAlternative3:
		boolspec(spec, "ss03", data);
		return;
	case uiAttributeStylisticAlternative4:
		boolspec(spec, "ss04", data);
		return;
	case uiAttributeStylisticAlternative5:
		boolspec(spec, "ss05", data);
		return;
	case uiAttributeStylisticAlternative6:
		boolspec(spec, "ss06", data);
		return;
	case uiAttributeStylisticAlternative7:
		boolspec(spec, "ss07", data);
		return;
	case uiAttributeStylisticAlternative8:
		boolspec(spec, "ss08", data);
		return;
	case uiAttributeStylisticAlternative9:
		boolspec(spec, "ss09", data);
		return;
	case uiAttributeStylisticAlternative10:
		boolspec(spec, "ss10", data);
		return;
	case uiAttributeStylisticAlternative11:
		boolspec(spec, "ss11", data);
		return;
	case uiAttributeStylisticAlternative12:
		boolspec(spec, "ss12", data);
		return;
	case uiAttributeStylisticAlternative13:
		boolspec(spec, "ss13", data);
		return;
	case uiAttributeStylisticAlternative14:
		boolspec(spec, "ss14", data);
		return;
	case uiAttributeStylisticAlternative15:
		boolspec(spec, "ss15", data);
		return;
	case uiAttributeStylisticAlternative16:
		boolspec(spec, "ss16", data);
		return;
	case uiAttributeStylisticAlternative17:
		boolspec(spec, "ss17", data);
		return;
	case uiAttributeStylisticAlternative18:
		boolspec(spec, "ss18", data);
		return;
	case uiAttributeStylisticAlternative19:
		boolspec(spec, "ss19", data);
		return;
	case uiAttributeStylisticAlternative20:
		boolspec(spec, "ss20", data);
		return;
	case uiAttributeContextualAlternates:
		boolspec(spec, "calt", data);
		return;
	case uiAttributeSwashes:
		boolspec(spec, "swsh", data);
		return;
	case uiAttributeContextualSwashes:
		boolspec(spec, "cswh", data);
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
