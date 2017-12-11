// uiAttribute stores information about an attribute in a
// uiAttributedString.
//
// You do not create uiAttributes directly; instead, you create a
// uiAttribute of a given type using the specialized constructor
// functions. For every Unicode codepoint in the uiAttributedString,
// at most one value of each attribute type can be applied.
//
// uiAttributes are immutable and the uiAttributedString takes
// ownership of the uiAttribute object once assigned, copying its
// contents as necessary.
typedef struct uiAttribute uiAttribute;

// uiFreeAttribute() frees a uiAttribute. You generally do not need to
// call this yourself, as uiAttributedString does this for you. In fact,
// it is an error to call this function on a uiAttribute that has been
// given to a uiAttributedString. You can call this, however, if you
// created a uiAttribute that you aren't going to use later.
_UI_EXTERN void uiFreeAttribute(uiAttribute *a);

// uiAttributeType holds the possible uiAttribute types that may be
// returned by uiAttributeGetType(). Refer to the documentation for
// each type's constructor function for details on each type.
_UI_ENUM(uiAttributeType) {
	uiAttributeTypeFamily,
	uiAttributeTypeSize,
	uiAttributeTypeWeight,
	uiAttributeTypeItalic,
	uiAttributeTypeStretch,
	uiAttributeTypeColor,
	uiAttributeTypeBackground,
	uiAttributeTypeUnderline,
	uiAttributeTypeUnderlineColor,
	uiAttributeTypeFeatures,
};

// uiAttributeGetType() returns the type of a.
// TODO I don't like this name
_UI_EXTERN uiAttributeType uiAttributeGetType(const uiAttribute *a);

// uiNewFamilyAttribute() creates a new uiAttribute that changes the
// font family of the text it is applied to. family is copied; you do not
// need to keep it alive after uiNewFamilyAttribute() returns. Font
// family names are case-insensitive.
_UI_EXTERN uiAttribute *uiNewFamilyAttribute(const char *family);

// uiAttributeFamily() returns the font family stored in a. The
// returned string is owned by a. It is an error to call this on a
// uiAttribute that does not hold a font family.
_UI_EXTERN const char *uiAttributeFamily(const uiAttribute *a);

// uiNewSizeAttribute() creates a new uiAttribute that changes the
// size of the text it is applied to, in typographical points.
_UI_EXTERN uiAttribute *uiNewFamilyAttribute(double size);

// uiAttributeSize() returns the font size stored in a. It is an error to
// call this on a uiAttribute that does not hold a font size.
_UI_EXTERN double uiAttributeSize(const uiAttribute *a);

// uiTextWeight represents possible text weights. These roughly
// map to the OSx2 text weight field of TrueType and OpenType
// fonts, or to CSS weight numbers. The named constants are
// nominal values; the actual values may vary by font and by OS,
// though this isn't particularly likely. Any value between
// uiTextWeightMinimum and uiDrawTextWeightMaximum,
// inclusive, is allowed.
//
// Note that due to restrictions in early versions of Windows, some
// fonts have "special" weights be exposed in many programs as
// separate font families. This is perhaps most notable with
// Arial Black. libui does not do this, even on Windows (because the
// DirectWrite API libui uses on Windows does not do this); to
// specify Arial Black, use family Arial and weight uiTextWeightBlack.
_UI_ENUM(uiTextWeight) {
	uiTextWeightMinimum = 0,
	uiTextWeightThin = 100,
	uiTextWeightUltraLight = 200,
	uiTextWeightLight = 300,
	uiTextWeightBook = 350,
	uiTextWeightNormal = 400,
	uiTextWeightMedium = 500,
	uiTextWeightSemiBold = 600,
	uiTextWeightBold = 700,
	uiTextWeightUltraBold = 800,
	uiTextWeightHeavy = 900,
	uiTextWeightUltraHeavy = 950,
	uiTextWeightMaximum = 1000,
};

// uiNewWeightAttribute() creates a new uiAttribute that changes the
// weight of the text it is applied to. It is an error to specify a weight
// outside the range [uiTextWeightMinimum,
// uiTextWeightMaximum].
_UI_EXTERN uiAttribute *uiNewWeightAttribute(uiTextWeight weight);

// uiAttributeWeight() returns the font weight stored in a. It is an error
// to call this on a uiAttribute that does not hold a font weight.
_UI_EXTERN uiTextWeight uiAttributeWeight(const uiAttribute *a);

// uiTextItalic represents possible italic modes for a font. Italic
// represents "true" italics where the slanted glyphs have custom
// shapes, whereas oblique represents italics that are merely slanted
// versions of the normal glyphs. Most fonts usually have one or the
// other.
_UI_ENUM(uiTextItalic) {
	uiTextItalicNormal,
	uiTextItalicOblique,
	uiTextItalicItalic,
};

// uiNewItalicAttribute() creates a new uiAttribute that changes the
// italic mode of the text it is applied to. It is an error to specify an
// italic mode not specified in uiTextItalic.
_UI_EXTERN uiAttribute *uiNewItalicAttribute(uiTextItalic italic);

// uiAttributeItalic() returns the font italic mode stored in a. It is an
// error to call this on a uiAttribute that does not hold a font italic
// mode.
_UI_EXTERN uiTextItalic uiAttributeItalic(const uiAttribute *a);

// uiTextStretch represents possible stretches (also called "widths")
// of a font.
//
// Note that due to restrictions in early versions of Windows, some
// fonts have "special" stretches be exposed in many programs as
// separate font families. This is perhaps most notable with
// Arial Condensed. libui does not do this, even on Windows (because
// the DirectWrite API libui uses on Windows does not do this); to
// specify Arial Condensed, use family Arial and stretch
// uiTextStretchCondensed.
_UI_ENUM(uiTextStretch) {
	uiTextStretchUltraCondensed,
	uiTextStretchExtraCondensed,
	uiTextStretchCondensed,
	uiTextStretchSemiCondensed,
	uiTextStretchNormal,
	uiTextStretchSemiExpanded,
	uiTextStretchExpanded,
	uiTextStretchExtraExpanded,
	uiTextStretchUltraExpanded,
};

// uiNewStretchAttribute() creates a new uiAttribute that changes the
// stretch of the text it is applied to. It is an error to specify a strech
// not specified in uiTextStretch.
_UI_EXTERN uiAttribute *uiNewStretchAttribute(uiTextStretch stretch);

// uiAttributeStretch() returns the font stretch stored in a. It is an
// error to call this on a uiAttribute that does not hold a font stretch.
_UI_EXTERN uiTextStretch uiAttributeStretch(const uiAttribute *a);

// uiNewColorAttribute() creates a new uiAttribute that changes the
// color of the text it is applied to. It is an error to specify an invalid
// color.
_UI_EXTERN uiAttribute *uiNewColorAttribute(double r, double g, double b, double a);

// uiAttributeColor() returns the text color stored in a. It is an
// error to call this on a uiAttribute that does not hold a text color.
_UI_EXTERN void uiAttributeColor(const uiAttribute *a, double *r, double *g, double *b, double *alpha);

// uiNewBackgroundAttribute() creates a new uiAttribute that
// changes the background color of the text it is applied to. It is an
// error to specify an invalid color.
_UI_EXTERN uiAttribute *uiNewBackgroundAttribute(double r, double g, double b, double a);

// TODO reuse uiAttributeColor() for background colors, or make a new function...

// uiUnderline specifies a type of underline to use on text.
_UI_ENUM(uiUnderline) {
	uiUnderlineNone,
	uiUnderlineSingle,
	uiUnderlineDouble,
	uiUnderlineSuggestion,		// wavy or dotted underlines used for spelling/grammar checkers
};

// uiNewUnderlineAttribute() creates a new uiAttribute that changes
// the type of underline on the text it is applied to. It is an error to
// specify an underline type not specified in uiUnderline.
_UI_EXTERN uiAttribute *uiNewUnderlineAttribute(uiUnderline u);

// uiAttributeUnderline() returns the underline type stored in a. It is
// an error to call this on a uiAttribute that does not hold an underline
// style.
_UI_EXTERN uiUnderline uiAttributeUnderline(const uiAttribute *a);

// uiUnderlineColor specifies the color of any underline on the text it
// is applied to, regardless of the type of underline. In addition to
// being able to specify a custom color, you can explicitly specify
// platform-specific colors for suggestion underlines; to use them
// correctly, pair them with uiUnderlineSuggestion (though they can
// be used on other types of underline as well).
// 
// If an underline type is applied but no underline color is
// specified, the text color is used instead. If an underline color
// is specified without an underline type, the underline color
// attribute is ignored, but not removed from the uiAttributedString.
_UI_ENUM(uiUnderlineColor) {
	uiUnderlineColorCustom,
	uiUnderlineColorSpelling,
	uiUnderlineColorGrammar,
	uiUnderlineColorAuxiliary,		// for instance, the color used by smart replacements on macOS or in Microsoft Office
};

// uiNewUnderlineColorAttribute() creates a new uiAttribute that
// changes the color of the underline on the text it is applied to.
// It is an error to specify an underline color not specified in
// uiUnderlineColor.
//
// If the specified color type is uiUnderlineColorCustom, it is an
// error to specify an invalid color value. Otherwise, the color values
// are ignored and should be specified as zero.
_UI_EXTERN uiAttribute *uiNewUnderlineColorAttribute(uiUnderlineColor u, double r, double g, double b, double a);

// uiAttributeUnderlineColor() returns the underline color stored in
// a. It is an error to call this on a uiAttribute that does not hold an
// underline color.
_UI_EXTERN void uiAttributeUnderline(const uiAttribute *a, uiUnderlineColor *u, double *r, double *g, double *b, double *alpha);

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

// uiNewFeaturesAttribute() creates a new uiAttribute that changes
// the font family of the text it is applied to. otf is copied; you may
// free it after uiNewFeaturesAttribute() returns.
_UI_EXTERN uiAttribute *uiNewFeaturesAttribute(const uiOpenTypeFeatures *otf);

// uiAttributeFeatures() returns the OpenType features stored in a.
// The returned uiOpenTypeFeatures object is owned by a. It is an
// error to call this on a uiAttribute that does not hold OpenType
// features.
_UI_EXTERN const uiOpenTypeFeatures *uiAttributeFeatures(const uiAttribute *a);

// TODO CONTINUE HERE

// TODO add a function to uiAttributedString to get an attribute's value at a specific index or in a specific range, so we can edit

typedef struct uiDrawFontDescriptor uiDrawFontDescriptor;

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
