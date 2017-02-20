// 11 february 2017
#include "drawtext.h"

static uiAttributedString *attrstr;

// some of these examples come from Microsoft's and Apple's lists of typographic features and also https://www.fontfont.com/staticcontent/downloads/FF_OT_User_Guide.pdf
static void setupAttributedString(void)
{
	uiAttributeSpec spec;
	size_t start, end;
	const char *next;
	int i;

	attrstr = uiNewAttributedString("uiAttributedString isn't just for plain text! It supports ");

	next = "multiple fonts";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeFamily;
	spec.Value = (uintptr_t) "Courier New";
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "multiple sizes";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeSize;
	spec.Double = 18;
	uiAttributedStringSetAttribute(attrstr,
		&spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "multiple weights";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeWeight;
	spec.Value = (uintptr_t) uiDrawTextWeightBold;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "multiple italics";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeItalic;
	spec.Value = (uintptr_t) uiDrawTextItalicItalic;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "multiple stretches";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeStretch;
	spec.Value = (uintptr_t) uiDrawTextStretchCondensed;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "multiple colors";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeColor;
	// Direct2D "Crimson" (#DC143C)
	spec.R = 0.8627450980392156;
	spec.G = 0.0784313725490196;
	spec.B = 0.2352941176470588;
	spec.A = 0.75;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "multiple backgrounds";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeBackground;
	// Direct2D "Peach Puff" (#FFDAB9)
	// TODO choose a darker color
	spec.R = 1.0;
	spec.G = 0.85490196078431372;
	spec.B = 0.7254901960784313;
	spec.A = 0.5;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "vertical glyph forms";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeVerticalForms;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " (which you can draw rotated for proper vertical text)");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "multiple";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeUnderline;
	spec.Value = uiDrawUnderlineStyleSingle;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " ");
	next = "underlines";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeUnderline;
	spec.Value = uiDrawUnderlineStyleDouble;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	spec.Type = uiAttributeUnderlineColor;
	spec.Value = uiDrawUnderlineColorCustom;
	spec.R = 0.5;
	spec.G = 0.0;
	spec.B = 1.0;
	spec.A = 1.0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " (");
	next = "including underlines for spelling correction and the like";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeUnderline;
	spec.Value = uiDrawUnderlineStyleSuggestion;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	spec.Type = uiAttributeUnderlineColor;
	spec.Value = uiDrawUnderlineColorSpelling;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	// thanks to https://twitter.com/codeman38/status/831924064012886017
	next = "\xD0\xB1\xD0\xB3\xD0\xB4\xD0\xBF\xD1\x82";
	uiAttributedStringAppendUnattributed(attrstr, "multiple languages (compare ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeItalic;
	spec.Value = uiDrawTextItalicItalic;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	spec.Type = uiAttributeLanguage;
	spec.Value = (uintptr_t) "ru";
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " to ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeItalic;
	spec.Value = uiDrawTextItalicItalic;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	spec.Type = uiAttributeLanguage;
	spec.Value = (uintptr_t) "sr";
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " \xE2\x80\x94 may require changing the font)");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	// TODO randomize these ranges better
	// TODO also change colors to light foreground dark background
	next = "or any combination of the above";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeWeight;
	spec.Value = (uintptr_t) uiDrawTextWeightBold;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end - 8);
	spec.Type = uiAttributeItalic;
	spec.Value = (uintptr_t) uiDrawTextItalicItalic;
	uiAttributedStringSetAttribute(attrstr, &spec, start + 3, end - 4);
	spec.Type = uiAttributeColor;
	spec.R = 0.8627450980392156;
	spec.G = 0.0784313725490196;
	spec.B = 0.2352941176470588;
	spec.A = 0.75;
	uiAttributedStringSetAttribute(attrstr, &spec, start + 12, end);
	spec.Type = uiAttributeFamily;
	spec.Value = (uintptr_t) "Helvetica";
	uiAttributedStringSetAttribute(attrstr, &spec, start + 8, end - 1);
	spec.Type = uiAttributeBackground;
	spec.R = 1.0;
	spec.G = 0.85490196078431372;
	spec.B = 0.7254901960784313;
	spec.A = 0.5;
	uiAttributedStringSetAttribute(attrstr, &spec, start + 5, end - 7);
	spec.Type = uiAttributeUnderline;
	spec.Value = uiDrawUnderlineStyleSingle;
	uiAttributedStringSetAttribute(attrstr, &spec, start + 9, end - 1);

	uiAttributedStringAppendUnattributed(attrstr, ". In addition, a variety of typographical features are available (depending on the chosen font) that can be switched on (or off, if the font enables them by default): ");

	next = "fi";
	uiAttributedStringAppendUnattributed(attrstr, "standard ligatures like f+i (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeStandardLigatures;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	// note the use of LTR marks and RTL embeds to make sure the bidi algorithm doesn't kick in for our demonstration (it will produce incorrect results)
	// see also: https://www.w3.org/International/articles/inline-bidi-markup/#nomarkup
	next = "\xD9\x84\xD8\xA7";
	uiAttributedStringAppendUnattributed(attrstr, "required ligatures like \xE2\x80\xAB\xD9\x84\xE2\x80\xAC+\xE2\x80\xAB\xD8\xA7\xE2\x80\xAC (\xE2\x80\x8E\xE2\x80\xAB");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeRequiredLigatures;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, "\xE2\x80\xAC)");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "ct";
	uiAttributedStringAppendUnattributed(attrstr, "discretionary/rare ligatures like c+t (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeDiscretionaryLigatures;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "the";
	uiAttributedStringAppendUnattributed(attrstr, "contextual ligatures like h+e in the (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeContextualLigatures;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "\xC3\x9F";
	uiAttributedStringAppendUnattributed(attrstr, "historical ligatures like the decomposition of \xC3\x9F (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeHistoricalLigatures;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	// TODO a different word than "writing"?
	next = "UnICasE wRITInG";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeUnicase;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "316";
	uiAttributedStringAppendUnattributed(attrstr, "proportional (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeNumberSpacings;
	spec.Value = uiAttributeNumberSpacingProportional;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ") and tabular/monospaced (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeNumberSpacings;
	spec.Value = uiAttributeNumberSpacingTabular;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ") numbers");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "123";
	uiAttributedStringAppendUnattributed(attrstr, "superscipts (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeSuperscripts;
	spec.Value = uiAttributeSuperscriptSuperscript;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "123";
	uiAttributedStringAppendUnattributed(attrstr, "subscripts (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeSuperscripts;
	spec.Value = uiAttributeSuperscriptSubscript;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "1st";
	uiAttributedStringAppendUnattributed(attrstr, "ordinals (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeSuperscripts;
	spec.Value = uiAttributeSuperscriptOrdinal;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "H2O";
	uiAttributedStringAppendUnattributed(attrstr, "scientific inferiors (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeSuperscripts;
	spec.Value = uiAttributeSuperscriptScientificInferior;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "2/3";
	uiAttributedStringAppendUnattributed(attrstr, "fraction forms (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeFractionForms;
	spec.Value = uiAttributeFractionFormNone;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ", ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeFractionForms;
	spec.Value = uiAttributeFractionFormVertical;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ", ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeFractionForms;
	spec.Value = uiAttributeFractionFormDiagonal;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "0";
	uiAttributedStringAppendUnattributed(attrstr, "slashed zeroes (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeSlashedZero;
	spec.Value = 0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeSlashedZero;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "\xCE\xA0\xCE\xA3";
	uiAttributedStringAppendUnattributed(attrstr, "mathematical greek (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeMathematicalGreek;
	spec.Value = 0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeMathematicalGreek;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "qwertyuiop\xE2\x80\xA2";
	uiAttributedStringAppendUnattributed(attrstr, "ornamental forms (");
	for (i = 1; i < 11; i++) {
		start = uiAttributedStringLen(attrstr);
		end = start + strlen(next);
		uiAttributedStringAppendUnattributed(attrstr, next);
		spec.Type = uiAttributeOrnamentalForms;
		spec.Value = (uintptr_t) i;
		uiAttributedStringSetAttribute(attrstr, &spec, start, end);
		next = "\xE2\x80\xA2";
	}
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "g";
	uiAttributedStringAppendUnattributed(attrstr, "specific forms/alternates (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeSpecificCharacterForm;
	spec.Value = 0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeSpecificCharacterForm;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "ABCDEFGQWERTY";
	uiAttributedStringAppendUnattributed(attrstr, "titling capital forms (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeTitlingCapitalForms;
	spec.Value = 0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeTitlingCapitalForms;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "\xE7\x80\x86";
	uiAttributedStringAppendUnattributed(attrstr, "alternate Han character forms (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeHanCharacterForms;
	spec.Value = uiAttributeHanCharacterFormJIS1978;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeHanCharacterForms;
	spec.Value = uiAttributeHanCharacterFormJIS1983;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "0123456789";
	uiAttributedStringAppendUnattributed(attrstr, "lowercase numbers (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeLowercaseNumbers;
	spec.Value = 0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeLowercaseNumbers;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "\xE4\xBC\xBD";
	uiAttributedStringAppendUnattributed(attrstr, "hanja to hangul translation (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeHanjaToHangul;
	spec.Value = 0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeHanjaToHangul;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "\xE3\x81\x82";
	uiAttributedStringAppendUnattributed(attrstr, "annotated glyph forms (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeGlyphAnnotations;
	spec.Value = 0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeGlyphAnnotations;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeGlyphAnnotations;
	spec.Value = 4;			// AAT inverted circle
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "\xE3\x81\x82";
	uiAttributedStringAppendUnattributed(attrstr, "ruby forms of kana (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeRubyKanaForms;
	spec.Value = 0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeRubyKanaForms;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "now is the time";
	uiAttributedStringAppendUnattributed(attrstr, "italic forms of Latin letters in CJK fonts (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeCJKRomansToItalics;
	spec.Value = 0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeCJKRomansToItalics;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "{I} > {J}";
	uiAttributedStringAppendUnattributed(attrstr, "case-sensitive character forms, such as punctuation (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeCaseSensitiveForms;
	spec.Value = 0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeCaseSensitiveForms;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "ABC";
	uiAttributedStringAppendUnattributed(attrstr, "specialized spacing between uppercase letters (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeCapitalSpacing;
	spec.Value = 0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeCapitalSpacing;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "\xE3\x82\xB9\xE3\x83\x98\xE3\x83\x88";
	uiAttributedStringAppendUnattributed(attrstr, "alternate horizontal (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeAlternateHorizontalKana;
	spec.Value = 0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeAlternateHorizontalKana;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ") and vertical (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeAlternateVerticalKana;
	spec.Value = 0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeAlternateVerticalKana;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ") kana forms");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "g";
	uiAttributedStringAppendUnattributed(attrstr, "stylistic alternates (");
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeStylisticAlternative1;
	spec.Value = 1;
	for (i = 0; i < 20; i++) {
		start = uiAttributedStringLen(attrstr);
		end = start + strlen(next);
		uiAttributedStringAppendUnattributed(attrstr, next);
		spec.Type++;
		uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	}
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "first";
	uiAttributedStringAppendUnattributed(attrstr, "contextual alternates (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeContextualAlternates;
	spec.Value = 0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeContextualAlternates;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "FONT";
	uiAttributedStringAppendUnattributed(attrstr, "swashes (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeSwashes;
	spec.Value = 0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeSwashes;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "Font";
	uiAttributedStringAppendUnattributed(attrstr, "contextual swashes (");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeContextualSwashes;
	spec.Value = 0;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, " vs. ");
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeContextualSwashes;
	spec.Value = 1;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ")");

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "Small Caps";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeLowercaseCapForms;
	spec.Value = uiAttributeCapFormSmallCaps;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ", ");
	next = "Petite Caps";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeLowercaseCapForms;
	spec.Value = uiAttributeCapFormPetiteCaps;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ", ");

	next = "SMALL UPPERCASES";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeUppercaseCapForms;
	spec.Value = uiAttributeCapFormSmallCaps;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);
	uiAttributedStringAppendUnattributed(attrstr, ", and ");
	next = "PETITE UPPERCASES";
	start = uiAttributedStringLen(attrstr);
	end = start + strlen(next);
	uiAttributedStringAppendUnattributed(attrstr, next);
	spec.Type = uiAttributeUppercaseCapForms;
	spec.Value = uiAttributeCapFormPetiteCaps;
	uiAttributedStringSetAttribute(attrstr, &spec, start, end);

	uiAttributedStringAppendUnattributed(attrstr, ".");

	// TODO write a dedicated example for experimenting with typographic features like the one in gtk3-demo
}

static char fontFamily[] = "Times New Roman";
// TODO should be const; look at constructor function?
static uiDrawFontDescriptor defaultFont = {
	.Family = fontFamily,
	.Size = 12,
	.Weight = uiDrawTextWeightNormal,
	.Italic = uiDrawTextItalicNormal,
	.Stretch = uiDrawTextStretchNormal,
};
static uiDrawTextLayoutParams params;

#define margins 10

static uiBox *panel;
static uiCheckbox *showLineBounds;
static uiFontButton *fontButton;

// TODO should be const?
static uiDrawBrush fillBrushes[4] = {
	{
		.Type = uiDrawBrushTypeSolid,
		.R = 1.0,
		.G = 0.0,
		.B = 0.0,
		.A = 0.5,
	},
	{
		.Type = uiDrawBrushTypeSolid,
		.R = 0.0,
		.G = 1.0,
		.B = 0.0,
		.A = 0.5,
	},
	{
		.Type = uiDrawBrushTypeSolid,
		.R = 0.0,
		.G = 0.0,
		.B = 1.0,
		.A = 0.5,
	},
	{
		.Type = uiDrawBrushTypeSolid,
		.R = 0.0,
		.G = 1.0,
		.B = 1.0,
		.A = 0.5,
	},
};

static void draw(uiAreaDrawParams *p)
{
	uiDrawPath *path;
	uiDrawTextLayout *layout;
	uiDrawBrush b;

	b.Type = uiDrawBrushTypeSolid;

	// only clip the text, not the guides
	uiDrawSave(p->Context);

	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path, margins, margins,
		p->AreaWidth - 2 * margins,
		p->AreaHeight - 2 * margins);
	uiDrawPathEnd(path);
	uiDrawClip(p->Context, path);
	uiDrawFreePath(path);

	params.Width = p->AreaWidth - 2 * margins;
	layout = uiDrawNewTextLayout(&params);
	uiDrawText(p->Context, layout, margins, margins);

	uiDrawRestore(p->Context);

	if (uiCheckboxChecked(showLineBounds)) {
		uiDrawTextLayoutLineMetrics m;
		int i, n;
		int fill = 0;

		n = uiDrawTextLayoutNumLines(layout);
		for (i = 0; i < n; i++) {
			uiDrawTextLayoutLineGetMetrics(layout, i, &m);

			path = uiDrawNewPath(uiDrawFillModeWinding);
			uiDrawPathAddRectangle(path, margins + m.X, margins + m.Y,
				m.Width, m.Height);
			uiDrawPathEnd(path);
			uiDrawFill(p->Context, path, fillBrushes + fill);
			uiDrawFreePath(path);
			fill = (fill + 1) % 4;
		}
	}

	uiDrawFreeTextLayout(layout);
}

static struct example attributesExample;

static void changeFont(uiFontButton *b, void *data)
{
	if (defaultFont.Family != fontFamily)
		uiFreeText(defaultFont.Family);
	// TODO rename defaultFont
	uiFontButtonFont(fontButton, &defaultFont);
	redraw();
}

// TODO share?
static void checkboxChecked(uiCheckbox *c, void *data)
{
	redraw();
}

static uiCheckbox *newCheckbox(const char *text)
{
	uiCheckbox *c;

	c = uiNewCheckbox(text);
	uiCheckboxOnToggled(c, checkboxChecked, NULL);
	uiBoxAppend(panel, uiControl(c), 0);
	return c;
}

struct example *mkAttributesExample(void)
{
	panel = uiNewVerticalBox();
	showLineBounds = newCheckbox("Show Line Bounds");
	fontButton = uiNewFontButton();
	uiFontButtonOnChanged(fontButton, changeFont, NULL);
	// TODO set the font button to the current defaultFont
	uiBoxAppend(panel, uiControl(fontButton), 0);

	attributesExample.name = "Attributed Text";
	attributesExample.panel = uiControl(panel);
	attributesExample.draw = draw;
	attributesExample.mouse = NULL;
	attributesExample.key = NULL;

	setupAttributedString();
	params.String = attrstr;
	params.DefaultFont = &defaultFont;
	params.Align = uiDrawTextLayoutAlignLeft;

	return &attributesExample;
}
