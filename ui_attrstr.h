// uiAttributedString represents a string of UTF-8 text that can
// optionally be embellished with formatting attributes. libui
// provides the list of formatting attributes, which cover common
// formatting traits like boldface and color as well as advanced
// typographical features like superscripts and small caps.
// These attributes can be combined in a variety of ways.
//
// In addition, uiAttributedString provides facilities for moving
// between grapheme clusters, which represent a character
// from the point of view of the end user. The cursor of a text editor
// is always placed on a grapheme boundary, so you can use these
// features to move the cursor left or right by one "character".
//
// uiAttributedString does not provide enough information to be able
// to draw itself onto a uiDrawContext or respond to user actions.
// In order to do that, you'll need to use a uiDrawTextLayout, which
// is built from the combination of a uiAttributedString and a set of
// layout-specific properties.
typedef struct uiAttributedString uiAttributedString;

// Note: where you say "1 = on", any nonzero value means "on". (TODO)
// TODO just make a separate field for everything?
_UI_ENUM(uiAttribute) {
	uiAttributeFamily,			// use Family
	uiAttributeSize,				// use Double
	uiAttributeWeight,
	uiAttributeItalic,
	uiAttributeStretch,
	uiAttributeColor,			// use R, G, B, A
	uiAttributeBackground,		// use R, G, B, A

	uiAttributeUnderline,		// enum uiDrawUnderlineStyle
	// TODO document that the color in the case we don't specify it is the text color
	uiAttributeUnderlineColor,	// enum uiDrawUnderlineColor

	// TODO note that for the purpose of uiAttributedString two sets of features are only the same (and thus their attributes are merged) only if the pointers are the same; whether the tag sets are the same only become relevant to uiDrawTextLayout
	uiAttributeFeatures,			// use Features
};

_UI_ENUM(uiDrawUnderlineStyle) {
	uiDrawUnderlineStyleNone,
	uiDrawUnderlineStyleSingle,
	uiDrawUnderlineStyleDouble,
	uiDrawUnderlineStyleSuggestion,		// wavy or dotted underlines used for spelling/grammar checkers
};

_UI_ENUM(uiDrawUnderlineColor) {
	uiDrawUnderlineColorCustom,		// also use R/G/B/A fields
	uiDrawUnderlineColorSpelling,
	uiDrawUnderlineColorGrammar,
	uiDrawUnderlineColorAuxiliary,	// for instance, the color used by smart replacements on OS X
};

// TODO rename?
typedef struct uiOpenTypeFeatures uiOpenTypeFeatures;
// TODO pass the feature set?
typedef int (*uiOpenTypeFeaturesForEachFunc)(char a, char b, char c, char d, uint32_t value, void *data);
// TODO detailed constructor?
_UI_EXTERN uiOpenTypeFeatures *uiNewOpenTypeFeatures(void);
_UI_EXTERN void uiFreeOpenTypeFeatures(uiOpenTypeFeatures *otf);
// TODO put above Free?
// TODO Copy instead of Clone?
_UI_EXTERN uiOpenTypeFeatures *uiOpenTypeFeaturesClone(const uiOpenTypeFeatures *otf);
_UI_EXTERN void uiOpenTypeFeaturesAdd(uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value);
_UI_EXTERN void uiOpenTypeFeaturesRemove(uiOpenTypeFeatures *otf, char a, char b, char c, char d);
_UI_EXTERN int uiOpenTypeFeaturesGet(uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t *value);
_UI_EXTERN void uiOpenTypeFeaturesForEach(uiOpenTypeFeatures *otf, uiOpenTypeFeaturesForEachFunc f, void *data);
_UI_EXTERN int uiOpenTypeFeaturesEqual(uiOpenTypeFeatures *a, uiOpenTypeFeatures *b);

typedef struct uiAttributeSpec uiAttributeSpec;

struct uiAttributeSpec {
	uiAttribute Type;
	const char *Family;
	uintptr_t Value;
	double Double;
	double R;
	double G;
	double B;
	double A;
	const uiOpenTypeFeatures *Features;	// TODO rename to OpenTypeFeatures?
};

// TODO name the foreach return values
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

// Height will equal ParagraphSpacingBefore + LineHeightSpace + Ascent + Descent + Leading + LineSpacing + ParagraphSpacing.
// The above values are listed in vertical order, from top to bottom.
// Ascent + Descent + Leading will give you the typographic bounds
// of the text. BaselineY is the boundary between Ascent and Descent.
// X, Y, and BaselineY are all in the layout's coordinate system, so the
// start point of the baseline will be at (X, BaselineY). All values are
// nonnegative.
struct uiDrawTextLayoutLineMetrics {
	// This describes the overall bounding box of the line.
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
_UI_EXTERN void uiFontButtonFont(uiFontButton *b, uiDrawFontDescriptor *desc);
// TOOD SetFont, mechanics
_UI_EXTERN void uiFontButtonOnChanged(uiFontButton *b, void (*f)(uiFontButton *, void *), void *data);
_UI_EXTERN uiFontButton *uiNewFontButton(void);
