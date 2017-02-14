typedef struct uiAttributedString uiAttributedString;
typedef struct uiAttributeSpec uiAttributeSpec;

// Note: where you say "1 = on", any nonzero value means "on". (TODO)
_UI_ENUM(uiAttribute) {
	uiAttributeFamily,
	uiAttributeSize,				// use Double
	uiAttributeWeight,
	uiAttributeItalic,
	uiAttributeStretch,
	uiAttributeColor,			// use R, G, B, A
	uiAttributeBackground,		// use R, G, B, A
	// TODO rename to uiAttributeVertical?
	uiAttributeVerticalForms,		// 0 = off, 1 = on

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

	// TODO kCTLigatureAttributeName vs below

	// TODO kCTStrokeWidthAttributeName/kCTStrokeColorAttributeName? doesn't seem to be present anywhere else?

	// TODO underline styles
	// OS X: kCTUnderlineStyleAttributeName
	// 	none, single, thick, or double
	// 	styles: solid, dot, dash, dash dot, dash dot dot
	// Pango: pango_attr_underline_new()
	// 	none, single, double, low single, or wavy/error
	// DirectWrite: only ever defined in version 1
	// 	none or single only
	// 	we could do this with a custom renderer (see Petzold's articles; he does it there)

	// TODO kCTUnderlineColorAttributeName
	// OS X: RGBA
	// Pango: RGB(? TODO check for A in newer versions)
	// DirectWrite: none; unsure if this means "same as text color" or "black only" (I assume Direct2D decides)
	// 	we could do this with a custom renderer (see Petzold's articles; he does it there)

	// TODO kCTSuperscriptAttributeName vs below

	// TODO compare kCTVerticalFormsAttributeName to below?

	// TODO kCTGlyphInfoAttributeName

	// TODO kCTCharacterShapeAttributeName (seems to be provided by pango as well)

	// TODO kCTLanguageAttributeName?

	// TODO kCTRunDelegateAttributeName?

	// TODO kCTBaselineClassAttributeName, kCTBaselineInfoAttributeName, kCTBaselineReferenceInfoAttributeName

	// TODO kCTWritingDirectionAttributeName?

	// TODO kCTRubyAnnotationAttributeName vs below

	// TODO strikethroughs? (pango yes, directwrite yes, os x no)
	// TODO baseline offsets? (pango yes)
	// TODO size scales? (pango yes)
	// TODO fallbacks (pango: enable or disable)

#if 0

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

	uiAttributeCommonLigatures,		// 0 = off, 1 = on
	uiAttributeRequiredLigatures,		// 0 = off, 1 = on
	uiAttributeRareLigatures,			// 0 = off, 1 = on
	uiAttributeLogoLigatures,			// 0 = off, 1 = on
	uiAttributeRebusPictureLigatures,	// 0 = off, 1 = on
	uiAttributeDipthrongLigatures,		// 0 = off, 1 = on
	uiAttributeSquaredLigatures,		// 0 = off, 1 = on
		// TODO rename
	uiAttributeAbbreviatedSquared,	// 0 = off, 1 = on
	uiAttributeSymbolLigatures,		// 0 = off, 1 = on
	uiAttributeContextualLigatures,	// 0 = off, 1 = on
	uiAttributeHistoricalLigatures,		// 0 = off, 1 = on

	uiAttributeCursiveConnection,		// 0 = none, 1 = some, 2 = all

	uiAttributeLetterCasing,			// TODO
		// kLetterCaseType

	uiAttributeLinguisticRearrangement,	// 0 = off, 1 = on

	uiAttributeNumberSpacing,		// TODO
		// kNumberSpacingType

	// TODO kSmartSwashType

	// TODO kDiacriticsType

	// TODO kVerticalPositionType

	uiAttributeFractionForms,			// enum uiAttributeFractionForm

	// TODO kOverlappingCharactersType

	// TODO kTypographicExtrasType
	uiAttributeSlashedZero,			// 0 = off, 1 = on

	// TODO kMathematicalExtrasType
	uiAttributeMathematicalGreek,		// 0 = off, 1 = on

	// TODO kOrnamentSetsType

	// TODO kCharacterAlternativesType

	// TODO kDesignComplexityType

	// TODO kStyleOptionsType
	// TODO titling?

	// TODO kCharacterShapeType
	// we could probably make the enum now

	uiAttributeNumberCase,			// TODO

	// TODO kTextSpacingType

	// TODO kTransliterationType

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
	uiAttributeGlyphAnnotations,		// an integer from 0 to a font-specified upper bound

	// TODO kKanaSpacingType

	// TODO kIdeographicSpacingType

	// TODO kUnicodeDecompositionType

	// TODO kRubyKanaType

	// TODO kCJKSymbolAlternativesType

	// TODO kIdeographicAlternativesType

	// TODO kCJKVerticalRomanPlacementType

	// TODO kItalicCJKRomanType

	// TODO kCaseSensitiveLayoutType

	// TODO kAlternateKanaType

	uiAttributeStylisticAlternative1,		// 0 = off, 1 = on
	uiAttributeStylisticAlternative2,		// 0 = off, 1 = on
	uiAttributeStylisticAlternative3,		// 0 = off, 1 = on
	uiAttributeStylisticAlternative4,		// 0 = off, 1 = on
	uiAttributeStylisticAlternative5,		// 0 = off, 1 = on
	uiAttributeStylisticAlternative6,		// 0 = off, 1 = on
	uiAttributeStylisticAlternative7,		// 0 = off, 1 = on
	uiAttributeStylisticAlternative8,		// 0 = off, 1 = on
	uiAttributeStylisticAlternative9,		// 0 = off, 1 = on
	uiAttributeStylisticAlternative10,	// 0 = off, 1 = on
	uiAttributeStylisticAlternative11,	// 0 = off, 1 = on
	uiAttributeStylisticAlternative12,	// 0 = off, 1 = on
	uiAttributeStylisticAlternative13,	// 0 = off, 1 = on
	uiAttributeStylisticAlternative14,	// 0 = off, 1 = on
	uiAttributeStylisticAlternative15,	// 0 = off, 1 = on
	uiAttributeStylisticAlternative16,	// 0 = off, 1 = on
	uiAttributeStylisticAlternative17,	// 0 = off, 1 = on
	uiAttributeStylisticAlternative18,	// 0 = off, 1 = on
	uiAttributeStylisticAlternative19,	// 0 = off, 1 = on
	uiAttributeStylisticAlternative20,	// 0 = off, 1 = on

	// TODO kContextualAlternatesType

	// TODO kLowerCaseType

	// TODO kUpperCaseType

	// TODO kLanguageTagType?

	// TODO kCJKRomanSpacingType

#endif

	// TODO uiAttributeSystem,
	// TODO uiAttributeCustom,
};

_UI_ENUM(uiAttributeFractionForm) {
	uiAttributeFractionFormNone,
	uiAttributeFractionFormVertical,
	uiAttributeFractionFormDiagonal,
};

// TODO number case

struct uiAttributeSpec {
	uiAttribute Type;
	uintptr_t Value;
	double Double;
	double R;
	double G;
	double B;
	double A;
};

typedef int (*uiAttributedStringForEachAttributeFunc)(uiAttributedString *s, uiAttributeSpec *spec, size_t start, size_t end, void *data);

// @role uiAttributedString constructor
// uiNewAttributedString() creates a new uiAttributedString from
// initialString. The string will be entirely unattributed.
_UI_EXTERN uiAttributedString *uiNewAttributedString(const char *initialString);

// @role uiAttributedString destructor
// uiFreeAttributedString() destroys the uiAttributedString s.
_UI_EXTERN void uiFreeAttributedString(uiAttributedString *s);

// uiAttributedStringString() returns the textual content of s as a
// '\0'-terminated UTF-8 string. The returned pointer is valid until
// the next change to the textual content of s.
_UI_EXTERN const char *uiAttributedStringString(uiAttributedString *s);

// uiAttributedStringLength() returns the number of UTF-8 bytes in
// the textual content of s, excluding the terminating '\0'.
_UI_EXTERN size_t uiAttributedStringLen(uiAttributedString *s);

_UI_EXTERN void uiAttributedStringAppendUnattributed(uiAttributedString *s, const char *str);
_UI_EXTERN void uiAttributedStringInsertAtUnattributed(uiAttributedString *s, const char *str, size_t at);
_UI_EXTERN void uiAttributedStringDelete(uiAttributedString *s, size_t start, size_t end);
_UI_EXTERN size_t uiAttributedStringNumGraphemes(uiAttributedString *s);
_UI_EXTERN size_t uiAttributedStringByteIndexToGrapheme(uiAttributedString *s, size_t pos);
_UI_EXTERN size_t uiAttributedStringGraphemeToByteIndex(uiAttributedString *s, size_t pos);
_UI_EXTERN void uiAttributedStringSetAttribute(uiAttributedString *s, uiAttributeSpec *spec, size_t start, size_t end);
_UI_EXTERN void uiAttributedStringForEachAttribute(uiAttributedString *s, uiAttributedStringForEachAttributeFunc f, void *data);

typedef struct uiDrawFontDescriptor uiDrawFontDescriptor;

// TODO Minimum == 1? IIRC there is at least one font on OS X that actually has a weight of 0
// TODO Maximum == 999? IIRC there is at least one font on OS X that actually has a weight of 1000
_UI_ENUM(uiDrawTextWeight) {
	uiDrawTextWeightMinimum = 0,
	uiDrawTextWeightThin = 100,
	uiDrawTextWeightUltraLight = 200,
	uiDrawTextWeightLight = 300,
	uiDrawTextWeightBook = 350,
	uiDrawTextWeightNormal = 400,
	uiDrawTextWeightMedium = 500,
	uiDrawTextWeightSemiBold = 600,
	uiDrawTextWeightBold = 700,
	uiDrawTextWeightUltraBold = 800,
	uiDrawTextWeightHeavy = 900,
	uiDrawTextWeightUltraHeavy = 950,
	uiDrawTextWeightMaximum = 1000,
};

_UI_ENUM(uiDrawTextItalic) {
	uiDrawTextItalicNormal,
	uiDrawTextItalicOblique,
	uiDrawTextItalicItalic,
};

// TODO realign this  so that Normal == 0?
_UI_ENUM(uiDrawTextStretch) {
	uiDrawTextStretchUltraCondensed,
	uiDrawTextStretchExtraCondensed,
	uiDrawTextStretchCondensed,
	uiDrawTextStretchSemiCondensed,
	uiDrawTextStretchNormal,
	uiDrawTextStretchSemiExpanded,
	uiDrawTextStretchExpanded,
	uiDrawTextStretchExtraExpanded,
	uiDrawTextStretchUltraExpanded,
};

struct uiDrawFontDescriptor {
	char *Family;
	// TODO rename to PointSize?
	double Size;
	uiDrawTextWeight Weight;
	uiDrawTextItalic Italic;
	uiDrawTextStretch Stretch;
};

typedef struct uiDrawTextLayout uiDrawTextLayout;
typedef struct uiDrawTextLayoutParams uiDrawTextLayoutParams;
typedef struct uiDrawTextLayoutLineMetrics uiDrawTextLayoutLineMetrics;

// TODO drop the Layout from this?
_UI_ENUM(uiDrawTextLayoutAlign) {
	uiDrawTextLayoutAlignLeft,
	uiDrawTextLayoutAlignCenter,
	uiDrawTextLayoutAlignRight,
};

struct uiDrawTextLayoutParams {
	uiAttributedString *String;
	uiDrawFontDescriptor *DefaultFont;
	double Width;
	uiDrawTextLayoutAlign Align;
};

struct uiDrawTextLayoutLineMetrics {
	// This describes the overall bounding box of the line.
	// TODO figure out if X is correct regardless of both alignment and writing direction
	double X;
	double Y;
	double Width;
	double Height;

	// This describes the typographic bounds of the line.
	double BaselineY;
	double Ascent;
	double Descent;
	double Leading;

	// This describes any additional whitespace.
	// TODO come up with better names for these.
	double ParagraphSpacingBefore;
	double LineHeightSpace;
	double LineSpacing;
	double ParagraphSpacing;

	// Height should equal ParagraphSpacingBefore + LineHeightSpace + Ascent + Descent + Leading + LineSpacing + ParagraphSpacing.
	// The above values are listed in vertical order, from top to bottom.
	// Ascent + Descent + Leading will give you the typographic bounds of the text.
	// BaselineY will be the boundary between Ascent and Descent.
	// X, Y, and BaselineY are all in the layout's coordinate system, so the start point of the baseline will be at (X, BaselineY).
	// All values will be nonnegative.

	// TODO trailing whitespace?
};

// TODO
// - allow creating a layout out of a substring
// - allow marking compositon strings
// - allow marking selections, even after creation
// - add the following functions:
// 	- uiDrawTextLayoutHeightForWidth() (returns the height that a layout would need to be to display the entire string at a given width)
// 	- uiDrawTextLayoutRangeForSize() (returns what substring would fit in a given size)
// 	- uiDrawTextLayoutNewWithHeight() (limits amount of string used by the height)
// - some function to fix up a range (for text editing)
_UI_EXTERN uiDrawTextLayout *uiDrawNewTextLayout(uiDrawTextLayoutParams *params);
_UI_EXTERN void uiDrawFreeTextLayout(uiDrawTextLayout *tl);
_UI_EXTERN void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y);
_UI_EXTERN void uiDrawTextLayoutExtents(uiDrawTextLayout *tl, double *width, double *height);
_UI_EXTERN int uiDrawTextLayoutNumLines(uiDrawTextLayout *tl);
_UI_EXTERN void uiDrawTextLayoutLineByteRange(uiDrawTextLayout *tl, int line, size_t *start, size_t *end);
_UI_EXTERN void uiDrawTextLayoutLineGetMetrics(uiDrawTextLayout *tl, int line, uiDrawTextLayoutLineMetrics *m);
// TODO number of lines visible for clipping rect, range visible for clipping rect?

// TODO rewrite all this documentation

// uiDrawTextLayoutHitTest() returns the byte offset and line closest
// to the given point. The point is relative to the top-left of the layout.
// If the point is outside the layout itself, the closest point is chosen;
// this allows the function to be used for cursor positioning with the
// mouse. Do keep the returned line in mind if used in this way; the
// user might click on the end of a line, at which point the cursor
// might be at the trailing edge of the last grapheme on the line
// (subject to the operating system's APIs).
_UI_EXTERN void uiDrawTextLayoutHitTest(uiDrawTextLayout *tl, double x, double y, size_t *pos, int *line);

// uiDrawTextLayoutByteLocationInLine() returns the point offset
// into the given line that the given byte position stands. This is
// relative to the line's X position (as returned by
// uiDrawTextLayoutLineGetMetrics()), which in turn is relative to
// the top-left of the layout. This function can be used for cursor
// positioning: if start and end are the start and end of the line
// (as returned by uiDrawTextLayoutLineByteRange()), you will get
// the correct offset, even if pos is at the end of the line. If pos is not
// in the range [start, end], a negative value will be returned,
// indicating you need to move the cursor to another line.
// TODO make sure this works right for right-aligned and center-aligned lines and justified lines and RTL text
_UI_EXTERN double uiDrawTextLayoutByteLocationInLine(uiDrawTextLayout *tl, size_t pos, int line);

_UI_EXTERN void uiDrawCaret(uiDrawContext *c, double x, double y, uiDrawTextLayout *layout, size_t pos, int *line);
// TODO allow blinking

typedef struct uiFontButton uiFontButton;
#define uiFontButton(this) ((uiFontButton *) (this))
// TODO document this returns a new font
_UI_EXTERN void uiFontButtonFont(uiFontButton *b, uiDrawFontDescriptor *desc);
// TOOD SetFont, mechanics
_UI_EXTERN void uiFontButtonOnChanged(uiFontButton *b, void (*f)(uiFontButton *, void *), void *data);
_UI_EXTERN uiFontButton *uiNewFontButton(void);
