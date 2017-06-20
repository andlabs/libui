// uiAttributedString represents a string of UTF-8 text that can
// optionally be embellished with formatting attributes. libui
// provides the list of formatting attributes, which cover common
// formatting traits like boldface and color as well as advanced
// typographical features provided by OpenType like superscripts
// and small caps. These attributes can be combined in a variety of
// ways.
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

// uiAttribute specifies the types of possible attributes that can be
// applied to a uiAttributedString. For every byte in the
// uiAttributedString, at most one value of each attribute type can
// be applied.
// TODO just make a separate field in uiAttributeSpec for everything? or make attribute objects opaque instead?
_UI_ENUM(uiAttribute) {
	// uiAttributeFamily changes the font family of the text it is
	// applied to. Use the Family field of uiAttributeSpec.
	// TODO case-sensitive?
	uiAttributeFamily,
	// uiAttributeSize changes the size of the text it is applied to,
	// in typographical points. Use the Double field of
	// uiAttributeSpec.
	uiAttributeSize,
	// uiAttributeWeight changes the weight of the text it is applied
	// to. Use the Value field of uiAttributeSpec and the
	// uiDrawTextWeight constants.
	uiAttributeWeight,
	// uiAttributeItalic changes the italicness of the text it is applied
	// to. Use the Value field of uiAttributeSpec and the
	// uiDrawTextItalic constants.
	uiAttributeItalic,
	// uiAttributeStretch changes the stretch of the text it is applied
	// to. Use the Value field of uiAttributeSpec and the
	// uiDrawTextStretch constants.
	uiAttributeStretch,
	// uiAttributeColor changes the color of the text it is applied to.
	// Use the R, G, B, and A fields of uiAttributeSpec.
	uiAttributeColor,
	// uiAttributeBackground changes the color of the text it is
	// applied to. Use the R, G, B, and A fields of uiAttributeSpec.
	uiAttributeBackground,

	// uiAttributeUnderline changes the underline style of the text
	// it is applied to. Use the Value field of uiAttributeSpec and the
	// uiDrawUnderlineStyle constants.
	uiAttributeUnderline,
	// uiAttributeUnderlineColor changes the color of any underline
	// on the text it is applied to, regardless of the style. Use the
	// Value field of uiAttributeSpec and the uiDrawUnderlineColor
	// constants (refer to its documentation for more information).
	// 
	// If an underline style is applied but no underline color is
	// specified, the text color is used instead.
	uiAttributeUnderlineColor,

	// uiAttributeFeatures changes the OpenType features of the
	// text it is applied to. Use the Features field of uiAttributeSpec.
	uiAttributeFeatures,
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

// uiOpenTypeFeatures represents a set of OpenType feature
// tag-value pairs, for applying OpenType features to text.
// OpenType feature tags are four-character codes defined by
// OpenType that cover things from design features like small
// caps and swashes to language-specific glyph shapes and
// beyond. Each tag may only appear once in any given
// uiOpenTypeFeatures instance. Each value is a 32-bit integer,
// often used as a Boolean flag, but sometimes as an index to choose
// a glyph shape to use.
// 
// If a font does not support a certain feature, that feature will be
// ignored. (TODO verify this on all OSs)
// 
// See the OpenType specification at
// https://www.microsoft.com/typography/otspec/featuretags.htm
// for the complete list of available features, information on specific
// features, and how to use them.
// TODO invalid features
typedef struct uiOpenTypeFeatures uiOpenTypeFeatures;

// uiOpenTypeFeaturesForEachFunc is the type of the function
// invoked by uiOpenTypeFeaturesForEach() for every OpenType
// feature in otf. Refer to that function's documentation for more
// details.
typedef uiForEach (*uiOpenTypeFeaturesForEachFunc)(const uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value, void *data);

// @role uiOpenTypeFeatures constructor
// uiNewOpenTypeFeatures() returns a new uiOpenTypeFeatures
// instance, with no tags yet added.
_UI_EXTERN uiOpenTypeFeatures *uiNewOpenTypeFeatures(void);

// @role uiOpenTypeFeatures destructor
// uiFreeOpenTypeFeatures() frees otf.
_UI_EXTERN void uiFreeOpenTypeFeatures(uiOpenTypeFeatures *otf);

// uiOpenTypeFeaturesClone() makes a copy of otf and returns it.
// Changing one will not affect the other.
_UI_EXTERN uiOpenTypeFeatures *uiOpenTypeFeaturesClone(const uiOpenTypeFeatures *otf);

// uiOpenTypeFeaturesAdd() adds the given feature tag and value
// to otf. The feature tag is specified by a, b, c, and d. If there is
// already a value associated with the specified tag in otf, the old
// value is removed.
_UI_EXTERN void uiOpenTypeFeaturesAdd(uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value);

// uiOpenTypeFeaturesRemove() removes the given feature tag
// and value from otf. If the tag is not present in otf,
// uiOpenTypeFeaturesRemove() does nothing.
_UI_EXTERN void uiOpenTypeFeaturesRemove(uiOpenTypeFeatures *otf, char a, char b, char c, char d);

// uiOpenTypeFeaturesGet() determines whether the given feature
// tag is present in otf. If it is, *value is set to the tag's value and
// nonzero is returned. Otherwise, zero is returned.
// 
// Note that if uiOpenTypeFeaturesGet() returns zero, value isn't
// changed. This is important: if a feature is not present in a
// uiOpenTypeFeatures, the feature is NOT treated as if its
// value was zero anyway. Script-specific font shaping rules and
// font-specific feature settings may use a different default value
// for a feature. You should likewise not treat a missing feature as
// having a value of zero either. Instead, a missing feature should
// be treated as having some unspecified default value.
_UI_EXTERN int uiOpenTypeFeaturesGet(const uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t *value);

// uiOpenTypeFeaturesForEach() executes f for every tag-value
// pair in otf. The enumeration order is unspecified. You cannot
// modify otf while uiOpenTypeFeaturesForEach() is running.
_UI_EXTERN void uiOpenTypeFeaturesForEach(const uiOpenTypeFeatures *otf, uiOpenTypeFeaturesForEachFunc f, void *data);

// uiOpenTypeFeaturesEqual() returns nonzero if a is equal to b.
// a is defined as equal to b if and only if both have exactly the same
// tags with exactly the same values, or if both are NULL.
_UI_EXTERN int uiOpenTypeFeaturesEqual(const uiOpenTypeFeatures *a, const uiOpenTypeFeatures *b);

typedef struct uiAttributeSpec uiAttributeSpec;

// TODO note that pointers are copied
// TODO add a function to uiAttributedString to get an attribute's value at a specific index or in a specific range, so we can edit
// (TODO related to above: what about memory consumption during a read-modify-write cycle due to copying?)
// TODO normalize documentation between typedefs and structs
struct uiAttributeSpec {
	uiAttribute Type;
	const char *Family;
	uintptr_t Value;
	double Double;
	double R;
	double G;
	double B;
	double A;
	const uiOpenTypeFeatures *Features;
};

// uiAttributedStringForEachAttributeFunc is the type of the function
// invoked by uiAttributedStringForEachAttribute() for every
// attribute in s. Refer to that function's documentation for more
// details.
typedef uiForEach (*uiAttributedStringForEachAttributeFunc)(const uiAttributedString *s, const uiAttributeSpec *spec, size_t start, size_t end, void *data);

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
// TODO document this
// TODO possibly copy the spec each time to ensure it doesn't get clobbered
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

_UI_ENUM(uiDrawTextAlign) {
	uiDrawTextAlignLeft,
	uiDrawTextAlignCenter,
	uiDrawTextAlignRight,
};

struct uiDrawTextLayoutParams {
	uiAttributedString *String;
	uiDrawFontDescriptor *DefaultFont;
	double Width;
	uiDrawTextAlign Align;
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
// TODO allow secondary carets

typedef struct uiFontButton uiFontButton;
#define uiFontButton(this) ((uiFontButton *) (this))
// TODO have a function that sets an entire font descriptor to a range in a uiAttributedString at once, for SetFont?
_UI_EXTERN void uiFontButtonFont(uiFontButton *b, uiDrawFontDescriptor *desc);
// TOOD SetFont, mechanics
_UI_EXTERN void uiFontButtonOnChanged(uiFontButton *b, void (*f)(uiFontButton *, void *), void *data);
_UI_EXTERN uiFontButton *uiNewFontButton(void);
